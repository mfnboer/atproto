// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "video_master.h"
#include <QTimer>

namespace ATProto {

using namespace std::chrono_literals;
static constexpr int MAX_CONCURRENT_PARTS = 3;

const QString VideoMaster::STATUS_UPLOADING = "SENDING";

VideoMaster::VideoMaster(Client& client) :
    Presence(),
    mClient(client)
{
}

VideoMaster::~VideoMaster()
{
    if (isParallelUploadInProgress())
        abortParallelUpload({}, {});
}

void VideoMaster::serialUpload(std::shared_ptr<QIODevice> ioDevice,
                               const UploadSuccessCb& successCb, const ErrorCb& errorCb,
                               const ProgressCb& progressCb)
{
    qDebug() << "Serial upload";
    Q_ASSERT(!mStarted);

    if (mStarted)
    {
        qWarning() << "Upload already started:" << mFileName;
        return;
    }

    mStarted = true;
    mStartTime = std::chrono::high_resolution_clock::now();
    mUploadSuccessCb = successCb;
    mUploadErrorCb = errorCb;
    mUploadProgressCb = progressCb;
    startSerialUpload(ioDevice);
}

void VideoMaster::startSerialUpload(std::shared_ptr<QIODevice> ioDevice)
{
    mClient.uploadVideo(ioDevice,
        [this, presence=getPresence()](ATProto::AppBskyVideo::JobStatus::SharedPtr output){
            if (!presence)
                return;

            qDebug() << "Upload finished, jobId:" << output->mJobId << "jobState:" << output->mRawState;
            mJobId = output->mJobId;
            mProcessingStartTime = std::chrono::high_resolution_clock::now();
            checkVideoUploadStatus(output);
        },
        [this, presence=getPresence()](const QString& error, const QString& message)
        {
            if (!presence)
                return;

            qWarning() << mFileName << "upload failed:" << error << " - " << message;
            failUpload(error, message);
        });
}

void VideoMaster::parallelUpload(const QString& fileName, std::optional<int> durationMs,
                                 std::optional<int> width, std::optional<int> height,
                                 const UploadSuccessCb& successCb, const ErrorCb& errorCb,
                                 const ProgressCb& progressCb)
{
    qDebug() << "Parallel upload:" << fileName << "duration:" << durationMs << "width:" << width << "height:" << height;
    Q_ASSERT(!mStarted);

    if (mStarted)
    {
        qWarning() << "Upload already started:" << mFileName;
        return;
    }

    mStarted = true;
    mStartTime = std::chrono::high_resolution_clock::now();
    mFileName = fileName;
    mUploadSuccessCb = successCb;
    mUploadErrorCb = errorCb;
    mUploadProgressCb = progressCb;

    QFileInfo fileInfo(fileName);
    mFileSize = fileInfo.size();

    if (mFileSize <= 0)
    {
        qWarning() << "Cannot open:" << fileName;
        QTimer::singleShot(0, [this, presence=getPresence()]{
            if (!presence)
                return;

            failUpload(ATProtoErrorMsg::FILE_ERROR, "Cannot open file");
        });

        return;
    }

    qDebug() << mFileName << "size:" << mFileSize;
    mClient.videoStartUpload(mFileSize, "video/mp4", fileName, durationMs, width, height,
        [this, presence=getPresence()](AppBskyVideo::StartUploadOutput::SharedPtr output, QString serviceAuthToken)
        {
            if (!presence)
                return;

            qDebug() << mFileName << "upload started:" << output->mJobId << "partCount:" << output->mPartCount << "partSize:" << output->mPartSizeBytes << "expires:" << output->mExpiresAt.toLocalTime();
            mServiceAuthToken = serviceAuthToken;
            mJobId = output->mJobId;

            if (!sliceFile(output->mPartCount, output->mPartSizeBytes))
            {
                failUpload(ATProtoErrorMsg::FILE_ERROR, "Failed to slice file");
                return;
            }

            uploadParts();
        },
        [this, presence=getPresence()](const QString& error, const QString& message)
        {
            if (!presence)
                return;

            qWarning() << mFileName << "upload failed:" << error << " - " << message;

            if (error == ATProtoErrorMsg::NOT_FOUND)
            {
                fallbackToSerialUpload();
                return;
            }

            failUpload(error, message);
        });
}

void VideoMaster::fallbackToSerialUpload()
{
    qDebug() << mFileName << "fall back to serial upload";
    mSerialFallbackFile = std::make_shared<QFile>(mFileName);

    if (mSerialFallbackFile->open(QFile::ReadOnly))
    {
        startSerialUpload(mSerialFallbackFile);
    }
    else
    {
        qWarning() << mFileName << "failed to open";
        failUpload(ATProtoErrorMsg::FILE_ERROR, "Cannot open file");
    }
}

void VideoMaster::uploadParts()
{
    while (mNextPartIndex < (int)mFileSlices.size() && mPartsUploading < MAX_CONCURRENT_PARTS && !mDone)
    {
        int partNumber = mNextPartIndex + 1;
        qDebug() << mFileName << "upload part:" << partNumber;
        auto part = mFileSlices[mNextPartIndex];

        if (!part->open(QFile::ReadOnly))
        {
            qWarning() << mFileName << "failed to open file, error:" << part->errorString();
            abortParallelUpload({}, {});
            failUpload(ATProtoErrorMsg::UPLOAD_ERROR, part->errorString());
            return;
        }

        ++mPartsUploading;
        ++mNextPartIndex;

        mClient.videoUploadPart(part, mServiceAuthToken, mJobId, partNumber,
            [this, presence=getPresence()](AppBskyVideo::UploadPartOutput::SharedPtr output)
            {
                if (!presence)
                    return;

                qDebug() << mFileName << "part uploaded:" << output->mPartNumber << "size:" << output->mSizeBytes;
                mBytesUploaded += output->mSizeBytes;
                closePart(output->mPartNumber);

                if (mUploadProgressCb)
                    mUploadProgressCb(STATUS_UPLOADING, (mBytesUploaded / (double)mFileSize) * 100);

                if (--mPartsUploading <= 0 && mNextPartIndex >= (int)mFileSlices.size())
                {
                    qDebug() << mFileName << "all parts uploaded";
                    finishUpload();
                }
                else
                {
                    uploadParts();
                }
            },
        [this, presence=getPresence(), partNumber](const QString& error, const QString& message)
        {
            if (!presence)
                return;

            qWarning() << mFileName << "upload part:" << partNumber << "failed:" << error << " - " << message;
            --mPartsUploading;
            closePart(partNumber);
            abortParallelUpload({}, {});
            failUpload(error, message);
        });
    }
}

void VideoMaster::closePart(int partNumber)
{
    qDebug() << mFileName << "close part:" << partNumber;
    const int index = partNumber - 1;

    if (index >= 0 && index < (int)mFileSlices.size())
        mFileSlices[index]->close();
    else
        qWarning() << mFileName << "part number out of bounds:" << partNumber << "parts:" << mFileSlices.size();
}

bool VideoMaster::isParallelUploadInProgress() const
{
    return mStarted && !mFileSlices.empty() && !mJobId.isEmpty() && !mServiceAuthToken.isEmpty();
}

void VideoMaster::abortParallelUpload(const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << mFileName << "abort upload";

    mClient.videoAbortUpload(mServiceAuthToken, mJobId,
        [this, presence=getPresence(), successCb](AppBskyVideo::AbortUploadOutput::SharedPtr){
            if (!presence)
                return;

            mFileSlices.clear();
            finish();

            if (successCb)
                successCb();
        },
        errorCb);
}

void VideoMaster::finishUpload()
{
    qDebug() << mFileName << "finish upload, jobId:" << mJobId;

    mClient.videoFinishUpload(mServiceAuthToken, mJobId,
        [this, presence=getPresence()](AppBskyVideo::FinishUploadOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << mFileName << "upload finished, jobId:" << output->mCompletedJobId << "jobStatus:" << output->mJobStatus->mRawState;
            mProcessingStartTime = std::chrono::high_resolution_clock::now();
            mJobId = output->mCompletedJobId;
            checkVideoUploadStatus(output->mJobStatus);
        },
        [this, presence=getPresence()](const QString& error, const QString& message)
        {
            if (!presence)
                return;

            qWarning() << mFileName << "finish upload failed:" << error << " - " << message;
            mFileSlices.clear();
            failUpload(error, message);
        });
}

void VideoMaster::checkVideoUploadStatus(AppBskyVideo::JobStatus::SharedPtr jobStatus)
{
    qDebug() << mFileName << "check job status:" << jobStatus->mRawState << "jobId:" << jobStatus->mJobId;

    switch (jobStatus->mState)
    {
    case AppBskyVideo::JobStatusState::JOB_STATE_COMPLETED:
        mFileSlices.clear();

        if (jobStatus->mBlob)
        {
            finish();

            if (mUploadSuccessCb)
                mUploadSuccessCb(jobStatus->mBlob);
        }
        else
        {
            qWarning() << mFileName << "blob missing from job status";
            failUpload(ATProtoErrorMsg::UPLOAD_ERROR, "Video blob missing");
        }

        break;
    case AppBskyVideo::JobStatusState::JOB_STATE_FAILED:
        mFileSlices.clear();
        failUpload(jobStatus->mError.value_or(ATProtoErrorMsg::UPLOAD_ERROR), jobStatus->mMessage.value_or("Job failed"));
        break;
    case AppBskyVideo::JobStatusState::JOB_STATE_INPROG:
        qDebug() << mFileName << "upload in progress, job:" << jobStatus->mJobId << "progress:" << jobStatus->mProgress.value_or(-1);

        if (mUploadProgressCb)
        {
            const QString status = jobStatus->mRawState.startsWith("JOB_STATE_") ? jobStatus->mRawState.sliced(10) : jobStatus->mRawState;
            mUploadProgressCb(status, jobStatus->mProgress);
        }

        mJobId = jobStatus->mJobId;
        QTimer::singleShot(1500, &mPresence, [this]{ getVideoUploadStatus(); });
        break;
    }
}

void VideoMaster::getVideoUploadStatus()
{
    qDebug() << mFileName << "get upload status, jobId:" << mJobId;

    mClient.getVideoJobStatus(mJobId,
        [this, presence=getPresence()](AppBskyVideo::JobStatusOutput::SharedPtr output){
            if (!presence)
                return;

            qDebug() << mFileName << "got job status:" << output->mJobStatus->mRawState << "jobId:" << output->mJobStatus->mJobId;
            checkVideoUploadStatus(output->mJobStatus);
        },
        [this, presence=getPresence()](const QString& error, const QString& message)
        {
            if (!presence)
                return;

            qWarning() << mFileName << "get job status failed:" << error << " - " << message;
            failUpload(error, message);
        });
}

bool VideoMaster::sliceFile(int partCount, int partSize)
{
    mFileSlices.clear();

    if (partCount <= 0)
    {
        qWarning() << mFileName << "invalid part count:" << partCount;
        return false;
    }

    if (partSize <= 0)
    {
        qWarning() << mFileName << "invalid part size:" << partSize;
        return false;
    }

    int start = 0;

    for (int i = 0; i < partCount; ++ i)
    {
        auto slice = std::make_shared<FileSlice>(mFileName, start, partSize);
        mFileSlices.emplace_back(std::move(slice));
        start += partSize;
    }

    qDebug() << mFileName << "parts:" << mFileSlices.size() << "partSize:" << partSize;
    mNextPartIndex = 0;
    mPartsUploading = 0;
    return true;
}

void VideoMaster::failUpload(const QString& error, const QString& message)
{
    if (mDone)
    {
        qDebug() << mFileName << "already done";
        return;
    }

    finish();

    if (mUploadErrorCb)
        mUploadErrorCb(error, message);
}

void VideoMaster::finish()
{
    mDone = true;
    const auto endTime = std::chrono::high_resolution_clock::now();

    if (mProcessingStartTime.time_since_epoch() == 0s)
    {
        const auto durationSecs = (endTime - mStartTime) / 1s;
        qDebug() << mFileName << "upload:" << durationSecs << "s";
    }
    else
    {
        const auto uploadDurationSecs = (mProcessingStartTime - mStartTime) / 1s;
        const auto processingDurationSecs = (endTime - mProcessingStartTime) / 1s;
        const auto totalDurationSecs = (endTime - mStartTime) / 1s;
        qDebug() << mFileName << "upload:" << uploadDurationSecs << "s" << "processing:" << processingDurationSecs << "s" << "total:" << totalDurationSecs << "s";
    }
}

}

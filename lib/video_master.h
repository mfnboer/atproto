// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"
#include "file_slice.h"
#include "presence.h"

namespace ATProto {

/**
 * @brief Use an instance of this class to upload 1 video.
 */
class VideoMaster : public Presence
{
public:
    using UploadSuccessCb = std::function<void(Blob::SharedPtr blob)>;
    using SuccessCb = Client::SuccessCb;
    using ErrorCb = Client::ErrorCb;
    using ProgressCb = std::function<void(const QString& status, std::optional<int> progress)>;

    static const QString STATUS_UPLOADING;

    explicit VideoMaster(Client& client);
    ~VideoMaster();

    void serialUpload(std::shared_ptr<QIODevice> ioDevice,
                      const UploadSuccessCb& successCb, const ErrorCb& errorCb,
                      const ProgressCb& progressCb);

    void parallelUpload(const QString& fileName, std::optional<int> durationMs,
                        std::optional<int> width, std::optional<int> height,
                        const UploadSuccessCb& successCb, const ErrorCb& errorCb,
                        const ProgressCb& progressCb);

    void abortParallelUpload(const SuccessCb& successCb, const ErrorCb& errorCb);

    bool isParallelUploadInProgress() const;

private:
    void startSerialUpload(std::shared_ptr<QIODevice> ioDevice);
    void fallbackToSerialUpload();
    void uploadParts();
    void finishUpload();
    void checkVideoUploadStatus(AppBskyVideo::JobStatus::SharedPtr jobStatus);
    void getVideoUploadStatus();
    bool sliceFile(int partCount, int partSize);
    void failUpload(const QString& error, const QString& message);
    void closePart(int partNumber);
    void finish();

    Client& mClient;
    bool mStarted = false;
    bool mDone = false;
    QString mFileName;
    qint64 mFileSize = 0;
    std::vector<std::shared_ptr<FileSlice>> mFileSlices;
    int mNextPartIndex = 0;
    int mPartsUploading = 0;
    qint64 mBytesUploaded = 0;
    UploadSuccessCb mUploadSuccessCb;
    ErrorCb mUploadErrorCb;
    ProgressCb mUploadProgressCb;
    QString mJobId;
    QString mServiceAuthToken;
    std::chrono::time_point<std::chrono::high_resolution_clock> mStartTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> mProcessingStartTime;
    std::shared_ptr<QFile> mSerialFallbackFile;
    QObject mPresence;
};

}
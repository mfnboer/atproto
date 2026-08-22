// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon.h"
#include <QJsonDocument>

namespace ATProto::AppBskyVideo {

enum class JobStatusState
{
    JOB_STATE_COMPLETED,
    JOB_STATE_FAILED,
    JOB_STATE_INPROG
};

JobStatusState stringToJobStatusState(const QString& str);

enum class JobStatusFailure
{
    JOB_FAILURE_VALIDATION,
    JOB_FAILURE_ENCODING,
    JOB_FAILURE_PDS_UPLOAD,
    JOB_FAILURE_PDS_UPLOAD_UNSUPPORTED_BLOB_SIZE,
    JOB_FAILURE_GENERIC,
    JOB_FAILURE_UNKNOWN
};

JobStatusFailure stringToJobStatusFailure(const QString& str);

// app.bsky.video.defs#jobStatus
struct JobStatus
{
    QString mJobId;
    QString mDid;
    JobStatusState mState;
    QString mRawState;
    std::optional<int> mProgress; // [0, 100]
    Blob::SharedPtr mBlob; // optional
    std::optional<QString> mError;
    std::optional<JobStatusFailure> mFailureCode;
    std::optional<QString> mRawFailureCode;
    std::optional<QString> mMessage;

    using SharedPtr = std::shared_ptr<JobStatus>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.video.getJobStatus#output
// app.bsky.video.uploadVideo
struct JobStatusOutput
{
    JobStatus::SharedPtr mJobStatus;

    using SharedPtr = std::shared_ptr<JobStatusOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.video.getUploadLimits#output
struct GetUploadLimitsOutput
{
    bool mCanUpload;
    std::optional<int> mRemainingDailyVideos;
    std::optional<qint64> mRemainingDailyBytes;
    std::optional<QString> mError;
    std::optional<QString> mMessage;

    using SharedPtr = std::shared_ptr<GetUploadLimitsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.video.startUpload#output
struct StartUploadOutput
{
    QString mJobId;
    int mPartSizeBytes = 0;
    int mPartCount = 0;
    QDateTime mExpiresAt;

    using SharedPtr = std::shared_ptr<StartUploadOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.video.uploadPart#output
struct UploadPartOutput
{
    int mPartNumnber = 0;
    int mSizeInBytes = 0;

    using SharedPtr = std::shared_ptr<UploadPartOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.video.finishUpload#output
struct FinishUploadOutput
{
    QString mCompletedJobId;
    JobStatus::SharedPtr mJobStatus;

    using SharedPtr = std::shared_ptr<FinishUploadOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

enum class AbortState
{
    ABORTED,
    COMPLETED,
    FAILED,
    EXPIRED,
    UNKNOWN
};

AbortState stringToAbortState(const QString& str);

// app.bsky.video.abortUpload#output
struct AbortUploadOutput
{
    AbortState mState;
    QString mRawState;
    std::optional<QString> mCompletedJobId;
    std::optional<QString> mFailureReason;

    using SharedPtr = std::shared_ptr<AbortUploadOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}

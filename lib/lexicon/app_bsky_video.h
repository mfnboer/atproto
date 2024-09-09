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

// app.bsky.video.defs#jobStatus
struct JobStatus
{
    QString mJobId;
    QString mDid;
    JobStatusState mState;
    QString mRawState;
    int mProgress; // [0, 100]
    Blob::SharedPtr mBlob; // optional
    std::optional<QString> mError;
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
    std::optional<int> mRemainingDailyBytes;
    std::optional<QString> mError;
    std::optional<QString> mMessage;

    using SharedPtr = std::shared_ptr<GetUploadLimitsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}

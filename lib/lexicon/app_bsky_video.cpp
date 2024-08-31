// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "app_bsky_video.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto::AppBskyVideo {

JobStatusState stringToJobStatusState(const QString& str)
{
    static const std::unordered_map<QString, JobStatusState> mapping = {
        { "JOB_STATE_COMPLETED", JobStatusState::JOB_STATE_COMPLETED },
        { "JOB_STATE_FAILED", JobStatusState::JOB_STATE_FAILED }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    // All unspecified states should be interpreted as in progress.
    qDebug() << "Unknown job status state:" << str;
    return JobStatusState::JOB_STATE_INPROG;
}

JobStatus::SharedPtr JobStatus::fromJson(const QJsonObject& json)
{
    auto jobStatus = std::make_shared<JobStatus>();
    const XJsonObject xjson(json);
    jobStatus->mJobId = xjson.getRequiredString("jobId");
    jobStatus->mDid = xjson.getRequiredString("did");
    jobStatus->mRawState = xjson.getRequiredString("state");
    jobStatus->mState = stringToJobStatusState(jobStatus->mRawState);
    jobStatus->mProgress = xjson.getOptionalInt("progress", 0);
    jobStatus->mBlob = xjson.getOptionalObject<Blob>("blob");
    jobStatus->mError = xjson.getOptionalString("error");
    jobStatus->mMessage = xjson.getOptionalString("message");
    return jobStatus;
}

JobStatusOutput::SharedPtr JobStatusOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<JobStatusOutput>();
    const XJsonObject xjson(json);
    output->mJobStatus = xjson.getRequiredObject<JobStatus>("jobStatus");
    return output;
}

GetUploadLimitsOutput::SharedPtr GetUploadLimitsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetUploadLimitsOutput>();
    const XJsonObject xjson(json);
    output->mCanUptload = xjson.getRequiredBool("canUpload");
    output->mRemainingDailyVideos = xjson.getOptionalInt("remainingDailyVideos");
    output->mRemainingDailyBytes = xjson.getOptionalInt("remainingDailyBytes");
    output->mError = xjson.getOptionalString("error");
    output->mMessage = xjson.getOptionalString("message");
    return output;
}

}

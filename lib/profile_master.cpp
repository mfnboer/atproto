// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "profile_master.h"
#include "at_uri.h"

namespace ATProto {

namespace {
constexpr char const* PROFILE_KEY = "self";
constexpr char const* STATUS_KEY = "self";
constexpr char const* LOGGED_OUT_VISIBILITY_LABEL = "!no-unauthenticated";
}

bool ProfileMaster::hasLabel(const ATProto::AppBskyActor::ProfileViewDetailed& profileView, const QString& label)
{
    auto it = std::find_if(profileView.mLabels.begin(), profileView.mLabels.end(),
                           [label](const auto& l){ return l->mVal == label && !l->mNeg; });

    return it != profileView.mLabels.end();
}

bool ProfileMaster::getLoggedOutVisibility(const ATProto::AppBskyActor::ProfileViewDetailed& profileView)
{
    return !hasLabel(profileView, LOGGED_OUT_VISIBILITY_LABEL);
}

ProfileMaster::ProfileMaster(Client& client) :
    Presence(),
    mClient(client)
{
}

void ProfileMaster::getProfile(const QString& did, const ProfileCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get profile:" << did;
    mClient.getRecord(did, ATUri::COLLECTION_ACTOR_PROFILE, PROFILE_KEY, {},
        [successCb, errorCb](ComATProtoRepo::Record::SharedPtr record) {
            qDebug() << "Got profile:" << record->mValue;

            try {
                auto profile = AppBskyActor::Profile::fromJson(record->mValue);

                if (successCb)
                    successCb(std::move(profile));
            } catch (InvalidJsonException& e) {
                qWarning() << e.msg();

                if (errorCb)
                    errorCb("InvalidJsonException", e.msg());
            }
        },
        [errorCb](const QString& err, const QString& msg) {
            qDebug() << "Failed to get profile:" << err << "-" << msg;

            if (errorCb)
                errorCb(err, msg);
        });
}

void ProfileMaster::updateProfile(const QString& did, const AppBskyActor::Profile& profile,
                   const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mClient.putRecord(did, ATUri::COLLECTION_ACTOR_PROFILE, PROFILE_KEY, profile.toJson(), true,
        [successCb](auto){
            if (successCb)
                successCb();
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void ProfileMaster::updateProfile(const QString& did, const QString& name, const QString& description,
                                  Blob::SharedPtr avatar, bool updateAvatar, Blob::SharedPtr banner, bool updateBanner,
                                  const QString& pronouns, const QString& website,
                                  const SuccessCb& successCb, const ErrorCb& errorCb)
{
    if (updateAvatar)
        mDidAvatarBlobMap[did] = std::move(avatar);

    if (updateBanner)
        mDidBannerBlobMap[did] = std::move(banner);

    getProfile(did,
        [this, presence=getPresence(), did, name, description, updateAvatar, updateBanner, pronouns, website, successCb, errorCb](auto&& profile)
        {
            if (!presence)
                return;

            setOptionalString(profile->mDisplayName, name);
            setOptionalString(profile->mDescription, description);

            if (updateAvatar)
            {
                profile->mAvatar = std::move(mDidAvatarBlobMap[did]);
                mDidAvatarBlobMap.erase(did);
            }

            if (updateBanner)
            {
                profile->mBanner = std::move(mDidBannerBlobMap[did]);
                mDidBannerBlobMap.erase(did);
            }

            setOptionalString(profile->mPronouns, pronouns);
            setOptionalString(profile->mWebsite, website);

            updateProfile(did, *profile, successCb, errorCb);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

bool ProfileMaster::addLabel(AppBskyActor::Profile& profile, const QString& label) const
{
    if (!profile.mLabels)
        profile.mLabels = std::make_shared<ComATProtoLabel::SelfLabels>();

    auto& labels = profile.mLabels->mValues;
    auto it = std::find_if(labels.begin(), labels.end(),
                           [label](const auto& l){ return l->mVal == label; });

    if (it != labels.end())
    {
        qDebug() << "Label already present:" << label;
        return false;
    }

    auto l = std::make_shared<ComATProtoLabel::SelfLabel>();
    l->mVal = label;
    labels.push_back(std::move(l));
    return true;
}

void ProfileMaster::addSelfLabel(const QString& did, const QString& label,
                  const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Add self label:" << label << "did:" << did;
    getProfile(did,
        [this, presence=getPresence(), did, label, successCb, errorCb](auto&& profile){
            if (!presence)
                return;

            if (addLabel(*profile, label))
            {
                updateProfile(did, *profile, successCb, errorCb);
            }
            else
            {
                // Label is already present, no need to update
                if (successCb)
                    successCb();
            }
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

bool ProfileMaster::removeLabel(AppBskyActor::Profile& profile, const QString& label) const
{
    if (!profile.mLabels)
        profile.mLabels = std::make_shared<ComATProtoLabel::SelfLabels>();

    auto& labels = profile.mLabels->mValues;
    auto it = std::find_if(labels.begin(), labels.end(),
                           [label](const auto& l){ return l->mVal == label; });

    if (it == labels.end())
    {
        qDebug() << "Label is not present:" << label;
        return false;
    }

    labels.erase(it);
    return true;
}

void ProfileMaster::removeSelfLabel(const QString& did, const QString& label,
                     const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Remove self label:" << label << "did:" << did;
    getProfile(did,
        [this, presence=getPresence(), did, label, successCb, errorCb](auto&& profile){
            if (!presence)
                return;

            if (removeLabel(*profile, label))
            {
                updateProfile(did, *profile, successCb, errorCb);
            }
            else
            {
                // Label was not present, no need to update
                if (successCb)
                    successCb();
            }
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void ProfileMaster::setLoggedOutVisibility(const QString& did, bool enable,
                                           const SuccessCb& successCb, const ErrorCb& errorCb)
{
    if (enable)
        removeSelfLabel(did, LOGGED_OUT_VISIBILITY_LABEL, successCb, errorCb);
    else
        addSelfLabel(did, LOGGED_OUT_VISIBILITY_LABEL, successCb, errorCb);
}

void ProfileMaster::setPinnedPost(const QString& did, const QString& uri, const QString& cid,
                                  const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Set pinned post, did:" << did << "uri:" << uri << "cid:" << cid;
    getProfile(did,
        [this, presence=getPresence(), did, uri, cid, successCb, errorCb](auto&& profile){
            if (!presence)
                return;

            if (setPinnedPost(*profile, uri, cid))
            {
                updateProfile(did, *profile, successCb, errorCb);
            }
            else
            {
                // Pinned post was already set, no need to update
                if (successCb)
                    successCb();
            }
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void ProfileMaster::clearPinnedPost(const QString& did, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Clear pinned post, did:" << did;
    getProfile(did,
        [this, presence=getPresence(), did, successCb, errorCb](auto&& profile){
            if (!presence)
                return;

            if (clearPinnedPost(*profile))
            {
                updateProfile(did, *profile, successCb, errorCb);
            }
            else
            {
                // No pinned post was set, no need to update
                if (successCb)
                    successCb();
            }
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

bool ProfileMaster::setPinnedPost(AppBskyActor::Profile& profile, const QString& uri, const QString& cid) const
{
    if (profile.mPinndedPost && profile.mPinndedPost->mUri == uri && profile.mPinndedPost->mCid == cid)
    {
        qDebug() << "Post already pinned:" << uri << cid;
        return false;
    }

    profile.mPinndedPost = std::make_shared<ComATProtoRepo::StrongRef>();
    profile.mPinndedPost->mUri = uri;
    profile.mPinndedPost->mCid = cid;
    return true;
}

bool ProfileMaster::clearPinnedPost(AppBskyActor::Profile& profile) const
{
    if (!profile.mPinndedPost)
    {
        qDebug() << "No pinned post";
        return false;
    }

    profile.mPinndedPost = nullptr;
    return true;
}

void ProfileMaster::getStatus(const QString& did, const StatusCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get status:" << did;
    mClient.getRecord(did, ATUri::COLLECTION_ACTOR_STATUS, STATUS_KEY, {},
        [successCb, errorCb](ComATProtoRepo::Record::SharedPtr record) {
            qDebug() << "Got status:" << record->mValue;

            try {
                auto status = AppBskyActor::Status::fromJson(record->mValue);

                if (successCb)
                    successCb(std::move(status));
            } catch (InvalidJsonException& e) {
                qWarning() << e.msg();

                if (errorCb)
                    errorCb("InvalidJsonException", e.msg());
            }
        },
        [errorCb](const QString& err, const QString& msg) {
            qDebug() << "Failed to get status:" << err << "-" << msg;

            if (errorCb)
                errorCb(err, msg);
        });
}

void ProfileMaster::updateStatus(const QString& did, const AppBskyActor::Status& status,
                  const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mClient.putRecord(did, ATUri::COLLECTION_ACTOR_STATUS, STATUS_KEY, status.toJson(), true,
        [successCb](auto){
            if (successCb)
                successCb();
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void ProfileMaster::deleteStatus(const QString& did, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mClient.deleteRecord(did, ATUri::COLLECTION_ACTOR_STATUS, STATUS_KEY,
        [successCb]{
            if (successCb)
                successCb();
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

}

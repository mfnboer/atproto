// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "profile_master.h"
#include "at_uri.h"

namespace ATProto {

namespace {
constexpr char const* PROFILE_KEY = "self";
constexpr char const* LOGGED_OUT_VISIBILITY_LABEL = "!no-unauthenticated";
}

bool ProfileMaster::hasLabel(const ATProto::AppBskyActor::ProfileView& profileView, const QString& label)
{
    auto it = std::find_if(profileView.mLabels.begin(), profileView.mLabels.end(),
                           [label](const auto& l){ return l->mVal == label && !l->mNeg; });

    return it != profileView.mLabels.end();
}

bool ProfileMaster::getLoggedOutVisibility(const ATProto::AppBskyActor::ProfileView& profileView)
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
    mClient.getRecord(did, ATUri::PROFILE_COLLECTION, PROFILE_KEY, {},
        [successCb, errorCb](ComATProtoRepo::Record::Ptr record) {
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
    mClient.putRecord(did, ATUri::PROFILE_COLLECTION, PROFILE_KEY, profile.toJson(),
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
                                  Blob::Ptr avatar, bool updateAvatar, Blob::Ptr banner, bool updateBanner,
                                  const SuccessCb& successCb, const ErrorCb& errorCb)
{
    if (updateAvatar)
        mDidAvatarBlobMap[did] = std::move(avatar);

    if (updateBanner)
        mDidBannerBlobMap[did] = std::move(banner);

    getProfile(did,
        [this, presence=getPresence(), did, name, description, updateAvatar, updateBanner, successCb, errorCb](auto&& profile)
        {
            if (!presence)
                return;

            profile->mDisplayName = name;
            profile->mDescription = description;

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
        profile.mLabels = std::make_unique<ComATProtoLabel::SelfLabels>();

    auto& labels = profile.mLabels->mValues;
    auto it = std::find_if(labels.begin(), labels.end(),
                           [label](const auto& l){ return l->mVal == label; });

    if (it != labels.end())
    {
        qDebug() << "Label already present:" << label;
        return false;
    }

    auto l = std::make_unique<ComATProtoLabel::SelfLabel>();
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
        profile.mLabels = std::make_unique<ComATProtoLabel::SelfLabels>();

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

}

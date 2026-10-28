// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "profile_master.h"
#include "at_uri.h"

namespace ATProto {

namespace {
constexpr char const* PROFILE_KEY = "self";
constexpr char const* STATUS_KEY = "self";
constexpr char const* CONTENT_VISIBILITY_DECLARATION_KEY = "self";
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
    mClient(client),
    mRepoMaster(mClient)
{
}

void ProfileMaster::getProfile(const QString& did, const ProfileCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get profile:" << did;
    mRepoMaster.getRecord<AppBskyActor::Profile>(did, PROFILE_KEY, {},
        [successCb](auto record){
            if (successCb)
                successCb(record->mEntity);
        },
        errorCb);
}

void ProfileMaster::updateProfile(const QString& did, const AppBskyActor::Profile& profile,
                   const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mRepoMaster.updateRecord(did, PROFILE_KEY, profile,
        [successCb](auto){
            if (successCb)
                successCb();
        },
        errorCb);
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
        errorCb);
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

    profile.mPinndedPost = RepoMaster::makeStrongRef(uri, cid);
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

void ProfileMaster::updateLinks(const QString& did, const Link::List links,
                                const LinkSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Update links:" << did;
    getProfile(did,
        [this, presence=getPresence(), did, links, successCb, errorCb](auto&& profile){
            if (!presence)
                return;

            updateLinks(did, links, *profile, successCb, errorCb);
        },
        errorCb);
}

void ProfileMaster::updateLinks(const QString& did, const Link::List links, const AppBskyActor::Profile& profile,
                 const LinkSuccessCb& successCb, const ErrorCb& errorCb)
{
    std::vector<QString> updateLinkUris;
    std::unordered_map<QString, Link> updateLinks;

    for (const auto& link : links)
    {
        if (link.mUpdate)
        {
            updateLinkUris.push_back(link.mUri);
            updateLinks[link.mUri] = link;
        }
    }

    if (updateLinkUris.empty())
    {
        updateLinksCreateUpdateRecords(did, links, profile, false, successCb, errorCb);
        return;
    }

    mRepoMaster.getRecords<AppBskyActor::Link>(updateLinkUris,
        [this, presence=getPresence(), updateLinks, did, links, profile, successCb, errorCb](auto linkRecords){
            if (!presence)
                return;

            if (linkRecords.size() != updateLinks.size())
            {
                qWarning() << "Could not get all links to update:" << updateLinks.size() << "got:" << linkRecords.size();

                if (errorCb)
                    errorCb(ATProtoErrorMsg::RECORD_NOT_FOUND, "Could not get records to update");

                return;
            }

            for (const auto& linkRecord : linkRecords)
            {
                AppBskyActor::Link::SharedPtr currentLink = linkRecord->mEntity;

                if (!updateLinks.contains(linkRecord->mUri))
                {
                    qWarning() << "Uri missing:" << linkRecord->mUri;

                    if (errorCb)
                        errorCb(ATProtoErrorMsg::RECORD_NOT_FOUND, "Could not get records to update");

                    return;
                }

                // Do not update created-at timestamp.
                // Copy json to persist unsupported fields
                auto& updateLink = updateLinks.at(linkRecord->mUri);
                updateLink.mLink->mCreatedAt = currentLink->mCreatedAt;
                updateLink.mLink->mJson = currentLink->mJson;

                // We do not support setting of labels on a link. Keep whatever was set.
                updateLink.mLink->mLabels = currentLink->mLabels;

                if (!updateLink.mHasNewIcon)
                    updateLink.mLink->mIcon = currentLink->mIcon;
            }

            updateLinksCreateUpdateRecords(did, links, profile, false, successCb, errorCb);
        });
}

void ProfileMaster::updateLinksCreateUpdateRecords(
    const QString& did, const Link::List links, const AppBskyActor::Profile& profile, bool partial,
    const LinkSuccessCb& successCb, const ErrorCb& errorCb, int index)
{
    if (index >= (int)links.size())
    {
        std::unordered_map<QString, ComATProtoRepo::StrongRef::SharedPtr> deleteUris; // uri -> ref

        for (const auto& link : profile.mLinks)
            deleteUris[link->mUri] = link;

        for (const auto& link : links)
            deleteUris.erase(link.mUri);

        updateLinksDeleteRecords(did, links, profile, partial, successCb, errorCb, std::move(deleteUris));
        return;
    }

    const auto& link = links[index];

    if (link.mUri.isEmpty())
    {
        mRepoMaster.createRecord(did, *link.mLink,
            [this, presence=getPresence(), did, links, profile, partial, successCb, errorCb, index](ComATProtoRepo::StrongRef::SharedPtr ref){
                if (!presence)
                    return;

                Link::List newLinks = links;
                auto& newLink = newLinks[index];
                newLink.mUri = ref->mUri;
                newLink.mCid = ref->mCid;
                newLink.mNew = true;

                qDebug() << "Created link:" << newLink.mLink->mUrl << "did:" << did;
                updateLinksCreateUpdateRecords(did, newLinks, profile, partial, successCb, errorCb, index + 1);
            },
            [this, presence=getPresence(), did, links, profile, successCb, errorCb, index](const QString& error, const QString& msg){
                if (!presence)
                    return;

                // Fail on first error, assuming the rest has a high likelyhood to fail.
                // Later on we will continue with partial success as previous succesful changes
                // can (probably) not be rolled back.
                if (index == 0)
                {
                    if (errorCb)
                        errorCb(error, msg);

                    return;
                }

                Link::List newLinks = links;
                auto& newLink = newLinks[index];
                newLink.mFailed = true;

                qWarning() << "Failed to create link:" << error << "-" << msg << "link:" << newLink.mLink->mUrl << "did:" << did;
                updateLinksCreateUpdateRecords(did, newLinks, profile, true, successCb, errorCb, index + 1);
            });
    }
    else if (link.mUpdate)
    {
        const bool updating = mRepoMaster.updateRecordByUri(did, link.mUri, *link.mLink,
            [this, presence=getPresence(), did, links, profile, partial, successCb, errorCb, index](ComATProtoRepo::StrongRef::SharedPtr ref){
                if (!presence)
                    return;

                // Content is updated, so CID changes.
                Link::List newLinks = links;
                auto& newLink = newLinks[index];

                qDebug() << "Updated link:" << links[index].mLink->mUrl << "did:" << did;
                qDebug() << "link:" << newLink.mUri << newLink.mCid << "updated:" << ref->mUri << ref->mCid;

                Q_ASSERT(newLink.mUri == ref->mUri);
                newLink.mUri = ref->mUri;
                newLink.mCid = ref->mCid;

                updateLinksCreateUpdateRecords(did, newLinks, profile, partial, successCb, errorCb, index + 1);
            },
            [this, presence=getPresence(), did, links, profile, successCb, errorCb, index](const QString& error, const QString& msg){
                if (!presence)
                    return;

                if (index == 0)
                {
                    if (errorCb)
                        errorCb(error, msg);

                    return;
                }

                Link::List newLinks = links;
                auto& newLink = newLinks[index];
                newLink.mFailed = true;

                qWarning() << "Failed to update link:" << error << "-" << msg << "link:" << newLink.mLink->mUrl << "did:" << did;
                updateLinksCreateUpdateRecords(did, newLinks, profile, true, successCb, errorCb, index + 1);
            });

        if (!updating)
        {   
            Link::List newLinks = links;
            auto& newLink = newLinks[index];
            newLink.mFailed = true;

            qWarning() << "Failed to update link: invalid uri, link:" << newLink.mLink->mUrl << "did:" << did;
            updateLinksCreateUpdateRecords(did, newLinks, profile, true, successCb, errorCb, index + 1);
        }
    }
    else
    {
        qDebug() << "No change link:" << links[index].mLink->mUrl << "did:" << did;
        updateLinksCreateUpdateRecords(did, links, profile, partial, successCb, errorCb, index + 1);
    }
}

void ProfileMaster::updateLinksDeleteRecords(
    const QString& did, const Link::List links, const AppBskyActor::Profile& profile, bool partial,
    const LinkSuccessCb& successCb, const ErrorCb& errorCb,
    std::unordered_map<QString, ComATProtoRepo::StrongRef::SharedPtr> deleteUris)
{
    qDebug() << "Delete link records:" << did << "count:" << deleteUris.size();

    if (deleteUris.empty())
    {
        updateLinksInProfile(did, links, profile, partial, successCb, errorCb);
        return;
    }

    const auto ref = deleteUris.begin()->second;
    deleteUris.erase(deleteUris.begin());
    qDebug() << "Delete link record:" << did << "uri:" << ref->mUri << "cid:" << ref->mCid;

    const bool deleting = mRepoMaster.deleteRecordByUri(did, AppBskyActor::Link::TYPE, ref->mUri,
        [this, presence=getPresence(), did, links, profile, partial, successCb, errorCb, deleteUris, ref]{
            if (!presence)
                return;

            qDebug() << "Deleted link:" << ref->mUri << "cid:" << ref->mCid << "did:" << did;
            updateLinksDeleteRecords(did, links, profile, partial, successCb, errorCb, deleteUris);
        },
        [this, presence=getPresence(), did, links, profile, successCb, errorCb, deleteUris, ref](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Failed to delete link:" << error << "-" << msg << "uri:" << ref->mUri << "cid:" << ref->mCid << "did:" << did;
            Link::List newLinks = links;
            Link link;
            link.mUri = ref->mUri;
            link.mCid = ref->mCid;
            link.mFailed = true; // Link without a Link record is a failed delete
            newLinks.push_back(link);
            updateLinksDeleteRecords(did, newLinks, profile, true, successCb, errorCb, deleteUris);
        });

    if (!deleting)
    {
        qWarning() << "Failed to delete link: invalid uri, uri:" << ref->mUri << "cid:" << ref->mCid << "did:" << did;
        Link::List newLinks = links;
        Link link;
        link.mUri = ref->mUri;
        link.mCid = ref->mCid;
        link.mFailed = true; // Link without a Link record is a failed delete
        newLinks.push_back(link);
        updateLinksDeleteRecords(did, newLinks, profile, true, successCb, errorCb, deleteUris);
    }
}

void ProfileMaster::updateLinksInProfile(const QString& did, const Link::List links, const AppBskyActor::Profile& profile, bool partial,
                                         const LinkSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Update links in profile:" << did << "links:" << links.size() << "partial:" << partial;
    ComATProtoRepo::StrongRef::List linkRefs;
    linkRefs.reserve(links.size());

    for (const auto& link : links)
    {
        if (!link.mUri.isEmpty() && !link.mCid.isEmpty())
        {   
            qDebug() << "Add link:" << link.mUri << "cid:" << link.mCid;
            auto ref = RepoMaster::makeStrongRef(link.mUri, link.mCid);
            linkRefs.push_back(std::move(ref));
        }
    }

    AppBskyActor::Profile newProfile = profile;
    newProfile.mLinks = linkRefs;

    updateProfile(did, newProfile,
        [partial, linkRefs, successCb]{
            if (successCb)
                successCb(partial, linkRefs);
        },
        [this, presence=getPresence(), did, links, errorCb](const QString& error, const QString& message){
            if (!presence)
                return;

            qWarning() << "Failed to update profile links:" << error << '-' << message;
            ComATProtoRepo::ApplyWritesList writes;

            for (const auto& link : links)
            {
                if (link.mNew)
                {
                    qDebug() << "Undo link:" << link.mUri << "did:" << did;
                    RepoMaster::addDeleteByUri(writes, link.mUri);
                }
            }

            if (!writes.empty())
            {
                // Try to delete just created records. If that fails there is nothing we can do.
                mClient.applyWrites(did, writes, {}, {}, {});
            }

            if (errorCb)
                errorCb(error, message);
        });
}

void ProfileMaster::getStatus(const QString& did, const StatusCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get status:" << did;
    mRepoMaster.getRecord<AppBskyActor::Status>(did, STATUS_KEY, {},
        [successCb](auto record){
            if (successCb)
                successCb(record->mEntity);
        },
        errorCb);
}

void ProfileMaster::updateStatus(const QString& did, const AppBskyActor::Status& status,
                  const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mRepoMaster.updateRecord(did, STATUS_KEY, status,
        [successCb](auto){
            if (successCb)
                successCb();
        },
        errorCb);
}

void ProfileMaster::deleteStatus(const QString& did, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mRepoMaster.deleteRecord(did, ATUri::COLLECTION_ACTOR_STATUS, STATUS_KEY, successCb, errorCb);
}

void ProfileMaster::getContentVisibilityDeclaration(const QString& did, const ContentVisibilityDeclarationCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get content visibility declatration" << did;
    mRepoMaster.getRecord<AppBskyActor::ContentVisibilityDeclaration>(
        did, CONTENT_VISIBILITY_DECLARATION_KEY, {},
        [successCb](auto record){
            if (successCb)
                successCb(record->mEntity);
        },
        errorCb);
}

void ProfileMaster::updateContentVisibilityDeclaration(const QString& did, const AppBskyActor::ContentVisibilityDeclaration& declaration,
                                        const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mRepoMaster.updateRecord(did, CONTENT_VISIBILITY_DECLARATION_KEY, declaration,
        [successCb](auto){
            if (successCb)
                successCb();
        },
        errorCb);
}

}

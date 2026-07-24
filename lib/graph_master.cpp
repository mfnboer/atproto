// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "graph_master.h"
#include "at_uri.h"
#include "lexicon/app_bsky_graph.h"

namespace ATProto {

GraphMaster::GraphMaster(Client & client) :
    Presence(),
    mClient(client),
    mRichTextMaster(client),
    mRepoMaster(client)
{
}

AppBskyGraph::StarterPackViewBasic::SharedPtr GraphMaster::createStarterPackViewBasic(const AppBskyGraph::StarterPackView::SharedPtr& view)
{
    auto viewBasic = std::make_shared<AppBskyGraph::StarterPackViewBasic>();
    viewBasic->mUri = view->mUri;
    viewBasic->mCid = view->mCid;
    viewBasic->mRecord = view->mRecord;
    viewBasic->mCreator = view->mCreator;
    viewBasic->mJoinedWeekCount = view->mJoinedWeekCount;
    viewBasic->mJoinedAllTimeCount = view->mJoinedAllTimeCount;
    viewBasic->mLabels = view->mLabels;
    viewBasic->mIndexedAt = view->mIndexedAt;
    return viewBasic;
}

void GraphMaster::follow(const QString& did,
                         const RecordSuccessCb& successCb, const ErrorCb& errorCb)
{
    createRecord<AppBskyGraph::Follow>(did, successCb, errorCb);
}

void GraphMaster::block(const QString& did,
                        const RecordSuccessCb& successCb, const ErrorCb& errorCb)
{
    createRecord<AppBskyGraph::Block>(did, successCb, errorCb);
}

void GraphMaster::listBlock(const QString& listUri,
                            const RecordSuccessCb& successCb, const ErrorCb& errorCb)
{
    createRecord<AppBskyGraph::ListBlock>(listUri, successCb, errorCb);
}

void GraphMaster::undo(const QString& uri,
                       const Client::SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Undo:" << uri;
    const auto atUri = ATUri::createAtUri(uri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    mRepoMaster.deleteRecord(atUri.getAuthority(), atUri.getCollection(), atUri.getRkey(), successCb, errorCb);
}

void GraphMaster::createList(AppBskyGraph::ListPurpose purpose, const QString& name,
                const QString& description,
                const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks,
                Blob::SharedPtr avatar, const QString& rKey,
                const CreateListSuccessCb& successCb, const ErrorCb& errorCb)
{
    auto list = std::make_shared<AppBskyGraph::List>();
    list->mPurpose = purpose;
    list->mName = name;
    list->mAvatar = std::move(avatar);
    list->mCreatedAt = QDateTime::currentDateTimeUtc();
    auto facets = RichTextMaster::parseFacets(description);
    RichTextMaster::insertEmbeddedLinksToFacets(embeddedLinks, facets);

    mRichTextMaster.resolveFacets(description, facets, 0, true,
        [this, presence=getPresence(), list, rKey, successCb, errorCb](const QString& richText, AppBskyRichtext::Facet::List resolvedFacets){
            if (!presence)
                return;

            if (!richText.isEmpty())
            {
                list->mDescription = richText;
                list->mDescriptionFacets = std::move(resolvedFacets);
            }

            createList(*list, rKey, successCb, errorCb);
        });
}

void GraphMaster::createList(const AppBskyGraph::List& list, const QString& rKey, const CreateListSuccessCb& successCb, const ErrorCb& errorCb)
{
    const auto listJson = list.toJson();
    qDebug() << "Create list:" << listJson;
    const QString& repo = mClient.getSessionDid();
    const QString collection = listJson["$type"].toString();

    mClient.createRecord(repo, collection, rKey, listJson, true,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void GraphMaster::updateList(const QString& listUri, const QString& name, const std::optional<QString>& description,
                             const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks,
                             Blob::SharedPtr avatar, bool updateAvatar,
                             const UpdateListSuccessCb& successCb, const ErrorCb& errorCb)
{
    const auto atUri = ATUri::createAtUri(listUri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    if (updateAvatar)
        mRKeyBlobMap[atUri.getRkey()] = std::move(avatar);

    mClient.getRecord(atUri.getAuthority(), atUri.getCollection(), atUri.getRkey(), {},
        [this, presence=getPresence(), atUri, name, description, embeddedLinks, updateAvatar, successCb, errorCb](ComATProtoRepo::Record::SharedPtr record){
            if (!presence)
                return;

            try {
                auto list = AppBskyGraph::List::fromJson(record->mValue);
                list->mName = name;

                if (updateAvatar)
                {
                    list->mAvatar = std::move(mRKeyBlobMap[atUri.getRkey()]);
                    mRKeyBlobMap.erase(atUri.getRkey());
                }

                if (description && list->mDescription.value_or("") != *description)
                    updateList(std::move(list), atUri.getRkey(), *description, embeddedLinks, successCb, errorCb);
                else
                    updateList(*list, atUri.getRkey(), successCb, errorCb);
            } catch (InvalidJsonException& e) {
                qWarning() << e.msg();
                mRKeyBlobMap.erase(atUri.getRkey());

                if (errorCb)
                    errorCb("InvalidJsonException", e.msg());
            }
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void GraphMaster::updateList(AppBskyGraph::List::SharedPtr list, const QString& rkey, const QString& description,
                             const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks,
                             const UpdateListSuccessCb& successCb, const ErrorCb& errorCb)
{
    auto facets = RichTextMaster::parseFacets(description);
    RichTextMaster::insertEmbeddedLinksToFacets(embeddedLinks, facets);
    mRKeyListMap[rkey] = std::move(list);

    mRichTextMaster.resolveFacets(description, facets, 0, true,
        [this, presence=getPresence(), rkey, successCb, errorCb](const QString& richText, AppBskyRichtext::Facet::List resolvedFacets){
            if (!presence)
                return;

            auto l = std::move(mRKeyListMap[rkey]);
            mRKeyListMap.erase(rkey);

            Q_ASSERT(l);
            if (!l) {
                qWarning() << "List not stored:" << rkey;

                if (errorCb)
                    errorCb("InternalError", "Internal error: list not stored");

                return;
            }

            if (!richText.isEmpty())
            {
                l->mDescription = richText;
                l->mDescriptionFacets = std::move(resolvedFacets);
            }
            else
            {
                l->mDescription.reset();
                l->mDescriptionFacets.clear();
            }

            updateList(*l, rkey, successCb, errorCb);
        });
}

void GraphMaster::updateList(const AppBskyGraph::List& list, const QString& rkey,
                             const UpdateListSuccessCb& successCb, const ErrorCb& errorCb)
{
    const auto listJson = list.toJson();
    qDebug() << "Update list:" << listJson;
    const QString& repo = mClient.getSessionDid();
    const QString collection = listJson["$type"].toString();

    mClient.putRecord(repo, collection, rkey, listJson, true,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void GraphMaster::addUserToList(const QString& listUri, const QString& did,
                                const AddListUserSuccessCb& successCb, const ErrorCb& errorCb)
{
    AppBskyGraph::ListItem record;
    record.mSubject = did;
    record.mList = listUri;
    record.mCreatedAt = QDateTime::currentDateTimeUtc();

    const auto recordJson = record.toJson();
    const QString& repo = mClient.getSessionDid();
    const QString collection = AppBskyGraph::ListItem::TYPE;

    mClient.createRecord(repo, collection, {}, recordJson, true,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void GraphMaster::batchAddUsersToList(const QString& listUri, const QStringList& dids,
                         const SuccessCb& successCb, const ErrorCb& errorCb)
{
    ATProto::ComATProtoRepo::ApplyWritesList writes;
    writes.reserve(dids.size());

    for (const auto& did : dids)
    {
        AppBskyGraph::ListItem record;
        record.mSubject = did;
        record.mList = listUri;
        record.mCreatedAt = QDateTime::currentDateTimeUtc();
        auto create = std::make_shared<ATProto::ComATProtoRepo::ApplyWritesCreate>();
        create->mCollection = AppBskyGraph::ListItem::TYPE;
        create->mValue = record.toJson();
        writes.push_back(std::move(create));
    }

    const QString& repo = mClient.getSessionDid();

    mClient.applyWrites(repo, writes, false,
        [successCb, presence=getPresence()] {
            if (!presence)
                return;

            if (successCb)
                successCb();
        },
        [errorCb, presence=getPresence()](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed to create records:" << error << "-" << msg;

            if (errorCb)
                errorCb(error, msg);
        });
}

void GraphMaster::getListByName(const QString& did, const QString& name, AppBskyGraph::ListPurpose purpose,
                                const std::optional<QString>& cursor,
                                const GetListSuccessCb& successCb, const ErrorCb& errorCb,
                                int maxPages)
{
    qDebug() << "Get list:" << name << "purpose:" << (int)purpose << "did:" << did << "cursor:" << cursor << "maxPages:" << maxPages;

    mClient.getLists(did, { purpose }, 100, cursor,
        [this, presence=getPresence(), did, name, purpose, successCb, errorCb, maxPages](AppBskyGraph::GetListsOutput::SharedPtr output){
            if (!presence)
                return;

            for (const auto& list : output->mLists)
            {
                if (list->mName == name)
                {
                    qDebug() << "Found list:" << name << "uri:" << list->mUri << "cid:" << list->mCid;

                    if (successCb)
                        successCb(list->mUri, list->mCid);

                    return;
                }
            }

            if (output->mCursor)
            {
                if (maxPages <= 1)
                {
                    qWarning() << "Max pages reached:" << name;

                    if (errorCb)
                        errorCb(ATProtoErrorMsg::NOT_FOUND, "Max pages reached");

                    return;
                }

                getListByName(did, name, purpose, output->mCursor, successCb, errorCb, maxPages - 1);
                return;
            }

            qDebug() << "List not found:" << name;

            if (errorCb)
                errorCb(ATProtoErrorMsg::NOT_FOUND, "List not found");
        },
        errorCb);
}

void GraphMaster::renameListByName(const QString& did, const QString& oldName, const QString& newName,
                      AppBskyGraph::ListPurpose purpose, const std::optional<QString>& cursor,
                      const RenameListSuccessCb& successCb, const ErrorCb& errorCb,
                      int maxPages)
{
    qDebug() << "Rename list:" << oldName << "new:" << newName << "purpose:" << (int)purpose << "did:" << did << "cursor:" << cursor << "maxPages:" << maxPages;

    getListByName(did, oldName, purpose, cursor,
        [this, presence=getPresence(), newName, successCb, errorCb](const QString& uri, const QString&){
            if (!presence)
                return;

            qDebug() << "Found list:" << uri << "rename to:" << newName;
            updateList(uri, newName, {}, {}, nullptr, false, successCb, errorCb);
        },
        errorCb,
        maxPages);
}

void GraphMaster::getVerifications(const QString& issuerDid, bool addVerificationsAsValid,
                      std::optional<int> limit, const std::optional<QString>& cursor,
                      const GetVerificationsSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get verifications issuer:" << issuerDid << "limit:" << limit << "cursor:" << cursor;
    if (!limit)
        limit = MAX_GET_VERIFICATIONS;

    if (limit < 1 || limit > MAX_GET_VERIFICATIONS)
    {
        qWarning() << "Invalid limit:" << *limit;
        return;
    }

    mClient.listRecords(issuerDid, AppBskyGraph::Verification::TYPE, limit, cursor,
        [this, issuerDid, addVerificationsAsValid, successCb, errorCb](ComATProtoRepo::ListRecordsOutput::SharedPtr output){
            if (output->mRecords.empty())
            {
                qDebug() << "No verifications:" << issuerDid;

                if (successCb)
                {
                    auto feed = std::make_shared<VerificationsOuput>();
                    feed->mCursor = output->mCursor;
                    successCb(feed);
                }

                return;
            }

            getVerificationsContinue(issuerDid, addVerificationsAsValid,
                                     output->mRecords, output->mCursor, successCb, errorCb);
        },
        [errorCb](const QString& err, const QString& msg){
            if (errorCb)
                errorCb(err, msg);
        });
}

void GraphMaster::getVerificationsContinue(const QString& issuerDid, bool addVerificationsAsValid,
                              const ATProto::ComATProtoRepo::Record::List& verificationRecords,
                              const std::optional<QString>& cursor,
                              const GetVerificationsSuccessCb& successCb, const ErrorCb& errorCb)
{
    std::vector<QString> dids;
    dids.reserve(verificationRecords.size());
    std::unordered_map<QString, ATProto::ComATProtoRepo::Record::SharedPtr> recordMap;
    std::unordered_map<QString, AppBskyGraph::Verification::SharedPtr> verificationMap;

    for (const auto& record : verificationRecords)
    {
        try {
            auto verification = AppBskyGraph::Verification::fromJson(record->mValue);
            const QString& did = verification->mSubject;
            dids.push_back(did);
            recordMap[did] = record;
            verificationMap[did] = verification;
        } catch (InvalidJsonException& e) {
            qWarning() << "Invalid verification:" << e.msg();
            qDebug() << record->mValue;
            continue;
        }
    }

    if (dids.empty())
    {
        qWarning() << "No DIDs";

        if (successCb)
        {
            auto feed = std::make_shared<VerificationsOuput>();
            feed->mCursor = cursor;
            successCb(feed);
        }

        return;
    }

    mClient.getProfiles(dids,
        [recordMap, verificationMap, issuerDid, addVerificationsAsValid, cursor, successCb](AppBskyActor::ProfileViewDetailed::List profiles){
            if (!successCb)
                return;

            auto feed = std::make_shared<VerificationsOuput>();
            feed->mCursor = cursor;

            for (auto& profile : profiles)
            {
                const auto verificationIt = verificationMap.find(profile->mDid);

                if (verificationIt == verificationMap.end())
                {
                    qWarning() << "DID missing in verificationMap:" << profile->mDid;
                    continue;
                }

                const auto recordIt = recordMap.find(profile->mDid);

                if (recordIt == recordMap.end())
                {
                    qWarning() << "DID missing in recordMap:" << profile->mDid;
                    continue;
                }

                auto& verification = verificationIt->second;
                auto verificationView = std::make_shared<AppBskyActor::VerificationView>();
                verificationView->mIssuer = issuerDid;
                verificationView->mUri = recordIt->second->mUri;
                verificationView->mIsValid = addVerificationsAsValid;
                verificationView->mCreatedAt = verification->mCreatedAt;

                if (!profile->mVerification)
                    profile->mVerification = std::make_shared<AppBskyActor::VerificationState>();

                profile->mVerification->mVerifications.push_back(verificationView);
                feed->mVerifiedUsers.push_back(profile);
            }

            successCb(feed);
        },
        [errorCb](const QString& err, const QString& msg){
            if (errorCb)
                errorCb(err, msg);
        });
}

template<class RecordType>
void GraphMaster::createRecord(const QString& subject, const RecordSuccessCb& successCb, const ErrorCb& errorCb)
{
    RecordType record;
    record.mSubject = subject;
    record.mCreatedAt = QDateTime::currentDateTimeUtc();

    const auto recordJson = record.toJson();
    const QString& repo = mClient.getSessionDid();
    const QString collection = recordJson["$type"].toString();

    mClient.createRecord(repo, collection, {}, recordJson, true,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

}

// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"
#include "presence.h"
#include "rich_text_master.h"
#include "repo_master.h"

/**
 * Functions to change the social graph, e.g. follow, block, ...
 */
namespace ATProto {

class GraphMaster : public Presence
{
public:
    struct VerificationsOuput
    {
        AppBskyActor::ProfileViewDetailed::List mVerifiedUsers;
        std::optional<QString> mCursor;

        using SharedPtr = std::shared_ptr<VerificationsOuput>;
    };

    using RecordSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using CreateListSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using UpdateListSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using AddListUserSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using GetListSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using RenameListSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using GetVerificationsSuccessCb = std::function<void(VerificationsOuput::SharedPtr)>;
    using SuccessCb = Client::SuccessCb;
    using ErrorCb = Client::ErrorCb;

    static constexpr int MAX_GET_VERIFICATIONS = 25;

    explicit GraphMaster(Client& client);

    static AppBskyGraph::StarterPackViewBasic::SharedPtr createStarterPackViewBasic(const AppBskyGraph::StarterPackView::SharedPtr& view);

    void follow(const QString& did,
                const RecordSuccessCb& successCb, const ErrorCb& errorCb);
    void block(const QString& did,
               const RecordSuccessCb& successCb, const ErrorCb& errorCb);
    void listBlock(const QString& listUri,
                   const RecordSuccessCb& successCb, const ErrorCb& errorCb);
    void undo(const QString& uri,
              const SuccessCb& successCb, const ErrorCb& errorCb);

    void createList(AppBskyGraph::ListPurpose purpose, const QString& name,
                    const QString& description,
                    const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks,
                    Blob::SharedPtr avatar, const QString& rKey,
                    const CreateListSuccessCb& successCb, const ErrorCb& errorCb);

    void updateList(const QString& listUri, const QString& name, const std::optional<QString>& description,
                    const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks,
                    Blob::SharedPtr avatar, bool updateAvatar,
                    const UpdateListSuccessCb& successCb, const ErrorCb& errorCb);

    void addUserToList(const QString& listUri, const QString& did,
                       const AddListUserSuccessCb& successCb, const ErrorCb& errorCb);

    void batchAddUsersToList(const QString& listUri, const QStringList& dids,
                             const SuccessCb& successCb, const ErrorCb& errorCb);

    void getListByName(const QString& did, const QString& name, AppBskyGraph::ListPurpose purpose,
                       const std::optional<QString>& cursor,
                       const GetListSuccessCb& successCb, const ErrorCb& errorCb,
                       int maxPages = 10);

    // Find a list by its old name and change the name to new name.
    void renameListByName(const QString& did, const QString& oldName, const QString& newName,
                          AppBskyGraph::ListPurpose purpose, const std::optional<QString>& cursor,
                          const RenameListSuccessCb& successCb, const ErrorCb& errorCb,
                          int maxPages = 10);

    /**
     * @brief getVerifications
     * @param issuerDid
     * @param addVerificationsAsValid value of the isValid flag in the VerificationView added
     * @param limit min=1 max=25 default=25
     * @param cursor
     * @param successCb
     * @param errorCb
     * The verification state of the returned profile will have the verification as the last
     * entry in the VerificationView list.
     */
    void getVerifications(const QString& issuerDid, bool addVerificationsAsValid,
                          std::optional<int> limit, const std::optional<QString>& cursor,
                          const GetVerificationsSuccessCb& successCb, const ErrorCb& errorCb);

private:
    void createList(const AppBskyGraph::List& list, const QString& rKey, const CreateListSuccessCb& successCb, const ErrorCb& errorCb);
    void updateList(AppBskyGraph::List::SharedPtr list, const QString& rkey, const QString& description,
                    const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks,
                    const UpdateListSuccessCb& successCb, const ErrorCb& errorCb);
    void updateList(const AppBskyGraph::List& list, const QString& rkey,
                    const UpdateListSuccessCb& successCb, const ErrorCb& errorCb);
    void getVerificationsContinue(const QString& issuerDid, bool addVerificationsAsValid,
                                  const ATProto::ComATProtoRepo::Record::List& verificationRecords,
                                  const std::optional<QString>& cursor,
                                  const GetVerificationsSuccessCb& successCb, const ErrorCb& errorCb);

    template<class RecordType>
    void createRecord(const QString& subject, const RecordSuccessCb& successCb, const ErrorCb& errorCb);

    Client& mClient;
    RichTextMaster mRichTextMaster;
    RepoMaster mRepoMaster;
    std::unordered_map<QString, Blob::SharedPtr> mRKeyBlobMap;
    std::unordered_map<QString, AppBskyGraph::List::SharedPtr> mRKeyListMap;
    QObject mPresence;
};

}

// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"
#include "presence.h"
#include "rich_text_master.h"

/**
 * Functions to change the social graph, e.g. follow, block, ...
 */
namespace ATProto {

class GraphMaster : public Presence
{
public:
    using RecordSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using CreateListSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using UpdateListSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using AddListUserSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using SuccessCb = Client::SuccessCb;
    using ErrorCb = Client::ErrorCb;

    explicit GraphMaster(Client& client);

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

    void updateList(const QString& listUri, const QString& name, const QString& description,
                    const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks,
                    Blob::SharedPtr avatar, bool updateAvatar,
                    const UpdateListSuccessCb& successCb, const ErrorCb& errorCb);

    void addUserToList(const QString& listUri, const QString& did,
                       const AddListUserSuccessCb& successCb, const ErrorCb& errorCb);

    void batchAddUsersToList(const QString& listUri, const QStringList& dids,
                             const SuccessCb& successCb, const ErrorCb& errorCb);

private:
    void createList(const AppBskyGraph::List& list, const QString& rKey, const CreateListSuccessCb& successCb, const ErrorCb& errorCb);
    void updateList(AppBskyGraph::List::SharedPtr list, const QString& rkey, const QString& description,
                    const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks,
                    const UpdateListSuccessCb& successCb, const ErrorCb& errorCb);
    void updateList(const AppBskyGraph::List& list, const QString& rkey,
                    const UpdateListSuccessCb& successCb, const ErrorCb& errorCb);

    template<class RecordType>
    void createRecord(const QString& subject, const RecordSuccessCb& successCb, const ErrorCb& errorCb);

    Client& mClient;
    RichTextMaster mRichTextMaster;
    std::unordered_map<QString, Blob::SharedPtr> mRKeyBlobMap;
    std::unordered_map<QString, AppBskyGraph::List::SharedPtr> mRKeyListMap;
    QObject mPresence;
};

}

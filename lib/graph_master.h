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
              const Client::SuccessCb& successCb, const ErrorCb& errorCb);

    void createList(AppBskyGraph::ListPurpose purpose, const QString& name,
                    const QString& description, Blob::Ptr avatar,
                    const CreateListSuccessCb& successCb, const ErrorCb& errorCb);
    void updateList(const AppBskyGraph::List& list, const QString& rkey, const SuccessCb& successCb, const ErrorCb& errorCb);

private:
    void createList(const AppBskyGraph::List& list, const CreateListSuccessCb& successCb, const ErrorCb& errorCb);

    template<class RecordType>
    void createRecord(const QString& subject, const RecordSuccessCb& successCb, const ErrorCb& errorCb);

    Client& mClient;
    RichTextMaster mRichTextMaster;
    QObject mPresence;
};

}

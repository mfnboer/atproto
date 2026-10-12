// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"
#include "at_uri.h"
#include "presence.h"

namespace ATProto {

class RepoMaster : public Presence
{
public:
    using ErrorCb = Client::ErrorCb;
    using SuccessCb = Client::SuccessCb;

    explicit RepoMaster(Client& client);

    template <typename Entity>
    void getRecord(const QString& repo, const QString& collection, const QString& rkey,
                   const std::optional<QString>& cid,
                   const std::function<void(typename Entity::SharedPtr)>& successCb,
                   const ErrorCb& errorCb);

    template <typename Entity>
    void getRecords(const std::vector<QString>& uris,
                    const std::function<void(typename Entity::List)>& successCb);

    template <typename Entity>
    void updateRecord(const QString& repo, const QString& collection, const QString& rkey,
                      const Entity& entity,
                      const SuccessCb& successCb, const ErrorCb& errorCb);

    void deleteRecord(const QString& repo, const QString& collection, const QString& rkey,
                      const SuccessCb& successCb, const ErrorCb& errorCb);

private:
    Client& mClient;
};

template <typename Entity>
inline void RepoMaster::getRecord(const QString& repo, const QString& collection, const QString& rkey,
                                  const std::optional<QString>& cid,
                                  const std::function<void(typename Entity::SharedPtr)>& successCb,
                                  const ErrorCb& errorCb)
{
    qDebug() << "Get record:" << repo << "collection:" << collection << "rkey:" << rkey << "cid:" << cid.value_or("");
    mClient.getRecord(repo, collection, rkey, cid,
        [successCb, errorCb](ComATProtoRepo::Record::SharedPtr record) {
            qDebug() << "Got record:" << record->mValue;

            try {
                auto entity = Entity::fromJson(record->mValue);

                if (successCb)
                    successCb(std::move(entity));
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

template <typename Entity>
inline void RepoMaster::getRecords(const std::vector<QString>& uris,
                                   const std::function<void(typename Entity::List)>& successCb)
{
    if (uris.empty())
    {
        qDebug() << "No records to get";

        if (successCb)
            QTimer::singleShot(0, [successCb]{ successCb({}); });

        return;
    }

    auto todo = std::make_shared<int>(uris.size());
    auto records = std::make_shared<typename Entity::List>();
    records->reserve(uris.size());

    for (const auto& uri : uris)
    {
        const ATUri atUri(uri);

        getRecord<Entity>(atUri.getAuthority(), atUri.getCollection(), atUri.getRkey(), {},
            [todo, records, successCb](auto record){
                records->push_back(record);

                if (--*todo > 0)
                    return;

                if (successCb)
                    successCb(*records);
            },
            [uri, todo, records, successCb](const QString& err, const QString& msg) {
                qDebug() << "Failed to get record:" << uri << err << "-" << msg;

                if (--*todo > 0)
                    return;

                if (successCb)
                    successCb(*records);
            });
    }
}

template <typename Entity>
inline void RepoMaster::updateRecord(const QString& repo, const QString& collection, const QString& rkey,
                                     const Entity& entity,
                                     const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Update record:" << repo << "collection:" << collection << "rkey:" << rkey;
    mClient.putRecord(repo, collection, rkey, entity.toJson(), {},
        [successCb](auto){
            if (successCb)
                successCb();
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

}

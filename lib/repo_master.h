// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"
#include "presence.h"

namespace ATProto {

class RepoMaster : public Presence
{
public:
    using ErrorCb = Client::ErrorCb;
    using SuccessCb = Client::SuccessCb;

    explicit RepoMaster(Client& client);

    template <typename Entity, typename EntitySuccessCb>
    void getRecord(const QString& repo, const QString& collection, const QString& rkey,
                   const std::optional<QString>& cid,
                   const EntitySuccessCb& successCb, const ErrorCb& errorCb);

    template <typename Entity>
    void updateRecord(const QString& repo, const QString& collection, const QString& rkey,
                      const Entity& entity,
                      const SuccessCb& successCb, const ErrorCb& errorCb);

    void deleteRecord(const QString& repo, const QString& collection, const QString& rkey,
                      const SuccessCb& successCb, const ErrorCb& errorCb);

private:
    Client& mClient;
};

template <typename Entity, typename EntitySuccessCb>
inline void RepoMaster::getRecord(const QString& repo, const QString& collection, const QString& rkey,
                                  const std::optional<QString>& cid,
                                  const EntitySuccessCb& successCb, const ErrorCb& errorCb)
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
inline void RepoMaster::updateRecord(const QString& repo, const QString& collection, const QString& rkey,
                                     const Entity& entity,
                                     const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Update record:" << repo << "collection:" << collection << "rkey:" << rkey;
    mClient.putRecord(repo, collection, rkey, entity.toJson(), true,
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

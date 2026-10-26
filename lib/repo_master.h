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
    template <typename Entity>
    struct Record
    {
        QString mUri;
        std::optional<QString> mCid;
        Entity::SharedPtr mEntity;

        using SharedPtr = std::shared_ptr<Record<Entity>>;
        using List = std::vector<SharedPtr>;
    };

    using ErrorCb = Client::ErrorCb;
    using SuccessCb = Client::SuccessCb;
    using CreateSuccessCb = std::function<void(ComATProtoRepo::StrongRef::SharedPtr)>;

    explicit RepoMaster(Client& client);

    template <typename Entity>
    void getRecord(const QString& repo, const QString& collection, const QString& rkey,
                   const std::optional<QString>& cid,
                   const std::function<void(typename Record<Entity>::SharedPtr)>& successCb,
                   const ErrorCb& errorCb);

    template <typename Entity>
    void getRecords(const std::vector<QString>& uris,
                    const std::function<void(typename Record<Entity>::List)>& successCb);

    // TODO: collection not needed -> collection = Entity::TYPE
    template <typename Entity>
    void createRecord(const QString& repo, const QString& collection, const Entity& entity,
                      const CreateSuccessCb& successCb, const ErrorCb& errorCb);

    template <typename Entity>
    void updateRecord(const QString& repo, const QString& collection, const QString& rkey,
                      const Entity& entity,
                      const SuccessCb& successCb, const ErrorCb& errorCb);

    template <typename Entity>
    bool updateRecordByUri(const QString& repo, const QString& collection, const QString& uri,
                           const Entity& entity,
                           const SuccessCb& successCb, const ErrorCb& errorCb);

    void deleteRecord(const QString& repo, const QString& collection, const QString& rkey,
                      const SuccessCb& successCb, const ErrorCb& errorCb);

    bool deleteRecordByUri(const QString& repo, const QString& collection, const QString& uri,
                           const SuccessCb& successCb, const ErrorCb& errorCb);

    template <typename Entity>
    static void addCreate(ComATProtoRepo::ApplyWritesList& writes, const Entity& entity);

    template <typename Entity>
    static void addUpdate(ComATProtoRepo::ApplyWritesList& writes, const Entity& entity, const QString& rkey);

    template <typename Entity>
    static bool addUpdateByUri(ComATProtoRepo::ApplyWritesList& writes, const Entity& entity, const QString& uri);

    template <typename Entity>
    static void addDelete(ComATProtoRepo::ApplyWritesList& writes, const QString& rkey);

    static bool addDeleteByUri(ComATProtoRepo::ApplyWritesList& writes, const QString& uri);

    static ComATProtoRepo::StrongRef::SharedPtr makeStrongRef(const QString& uri, const QString& cid);

private:
    Client& mClient;
};

template <typename Entity>
inline void RepoMaster::getRecord(const QString& repo, const QString& collection, const QString& rkey,
                                  const std::optional<QString>& cid,
                                  const std::function<void(typename Record<Entity>::SharedPtr)>& successCb,
                                  const ErrorCb& errorCb)
{
    qDebug() << "Get record:" << repo << "collection:" << collection << "rkey:" << rkey << "cid:" << cid.value_or("");
    mClient.getRecord(repo, collection, rkey, cid,
        [successCb, errorCb](ComATProtoRepo::Record::SharedPtr record) {
            qDebug() << "Got record:" << record->mValue;

            try {
                auto entityRecord = std::make_shared<Record<Entity>>();
                entityRecord->mUri = record->mUri;
                entityRecord->mCid = record->mCid;
                entityRecord->mEntity = Entity::fromJson(record->mValue);

                if (successCb)
                    successCb(std::move(entityRecord));
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
                                   const std::function<void(typename Record<Entity>::List)>& successCb)
{
    if (uris.empty())
    {
        qDebug() << "No records to get";

        if (successCb)
            QTimer::singleShot(0, [successCb]{ successCb({}); });

        return;
    }

    auto todo = std::make_shared<int>(uris.size());
    auto records = std::make_shared<typename Record<Entity>::List>();
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
inline void RepoMaster::createRecord(const QString& repo, const QString& collection, const Entity& entity,
                                     const CreateSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Create record:" << repo << "collection:" << collection;
    mClient.createRecord(repo, collection, {}, entity.toJson(), {}, successCb, errorCb);
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
        errorCb);
}

template <typename Entity>
bool RepoMaster::updateRecordByUri(const QString& repo, const QString& collection, const QString& uri,
                                   const Entity& entity,
                                   const SuccessCb& successCb, const ErrorCb& errorCb)
{
    const ATUri atUri(uri);

    if (!atUri.isValid())
    {
        qWarning() << "Invalid uri:" << uri;
        return false;
    }

    updateRecord(repo, collection, atUri.getRkey(), entity, successCb, errorCb);
    return true;
}

template <typename Entity>
inline void RepoMaster::addCreate(ComATProtoRepo::ApplyWritesList& writes, const Entity& entity)
{
    auto create = std::make_shared<ComATProtoRepo::ApplyWritesCreate>();
    create->mCollection = Entity::TYPE;
    create->mValue = entity.toJson();
    writes.push_back(std::move(create));
}

template <typename Entity>
inline void RepoMaster::addUpdate(ComATProtoRepo::ApplyWritesList& writes, const Entity& entity, const QString& rkey)
{
    auto update = std::make_shared<ComATProtoRepo::ApplyWritesUpdate>();
    update->mCollection = Entity::TYPE;
    update->mValue = entity.toJson();
    update->mRKey = rkey;
    writes.push_back(std::move(update));
}

template <typename Entity>
inline bool RepoMaster::addUpdateByUri(ComATProtoRepo::ApplyWritesList& writes, const Entity& entity, const QString& uri)
{
    const ATUri atUri(uri);

    if (!atUri.isValid())
    {
        qWarning() << "Invalid uri:" << uri;
        return false;
    }

    addUpdate(writes, entity, atUri.getRkey());
    return true;
}

template <typename Entity>
inline void RepoMaster::addDelete(ComATProtoRepo::ApplyWritesList& writes, const QString& rkey)
{
    auto remove = std::make_shared<ComATProtoRepo::ApplyWritesDelete>();
    remove->mCollection = Entity::TYPE;
    remove->mRKey = rkey;
    writes.push_back(std::move(remove));
}

inline bool RepoMaster::addDeleteByUri(ComATProtoRepo::ApplyWritesList& writes, const QString& uri)
{
    const ATUri atUri(uri);

    if (!atUri.isValid())
    {
        qWarning() << "Invalid uri:" << uri;
        return false;
    }

    auto remove = std::make_shared<ComATProtoRepo::ApplyWritesDelete>();
    remove->mCollection = atUri.getCollection();
    remove->mRKey = atUri.getRkey();
    writes.push_back(std::move(remove));
    return true;
}

}

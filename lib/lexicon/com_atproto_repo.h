// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon.h"
#include <QJsonDocument>
#include <QJsonObject>

namespace ATProto::ComATProtoRepo {

// com.atproto.repo.strongRef
struct StrongRef
{
    QString mUri;
    QString mCid;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<StrongRef>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// com.atproto.repo.uploadBlob#output
struct UploadBlobOutput
{
    Blob::SharedPtr mBlob;

    using SharedPtr = std::shared_ptr<UploadBlobOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// com.atproto.repo.getRecord#output
struct Record
{
    QString mUri;
    std::optional<QString> mCid;
    QJsonObject mValue;

    using SharedPtr = std::shared_ptr<Record>;
    static SharedPtr fromJson(const QJsonObject& json);
};

using RecordList = std::vector<Record::SharedPtr>;

// com.atproto.repo.listRecords#output
struct ListRecordsOutput
{
    std::optional<QString> mCursor;
    RecordList mRecords;

    using SharedPtr = std::shared_ptr<ListRecordsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// com.atproto.repo.applyWrites#create
struct ApplyWritesCreate
{
    QString mCollection;
    std::optional<QString> mRKey;
    QJsonObject mValue;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ApplyWritesCreate>;
};

// com.atproto.repo.applyWrites.defs#update
struct ApplyWritesUpdate
{
    QString mCollection;
    QString mRKey;
    QJsonObject mValue;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ApplyWritesUpdate>;
};

// com.atproto.repo.applyWrites.defs#delete
struct ApplyWritesDelete
{
    QString mCollection;
    QString mRKey;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ApplyWritesDelete>;
};

using ApplyWritesType = std::variant<ApplyWritesCreate::SharedPtr, ApplyWritesUpdate::SharedPtr, ApplyWritesDelete::SharedPtr>;
using ApplyWritesList = std::vector<ApplyWritesType>;

}

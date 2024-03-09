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

    using Ptr = std::unique_ptr<StrongRef>;
    static Ptr fromJson(const QJsonObject& json);
};

// com.atproto.repo.uploadBlob#output
struct UploadBlobOutput
{
    Blob::Ptr mBlob;

    using Ptr = std::unique_ptr<UploadBlobOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// com.atproto.repo.getRecord#output
struct Record
{
    QString mUri;
    std::optional<QString> mCid;
    QJsonObject mValue;

    using Ptr = std::unique_ptr<Record>;
    static Ptr fromJson(const QJsonObject& json);
};

using RecordList = std::vector<Record::Ptr>;

// com.atproto.repo.listRecords#output
struct ListRecordsOutput
{
    std::optional<QString> mCursor;
    RecordList mRecords;

    using Ptr = std::unique_ptr<ListRecordsOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// com.atproto.repo.applyWrites#create
struct ApplyWritesCreate
{
    QString mCollection;
    std::optional<QString> mRKey;
    QJsonObject mValue;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<ApplyWritesCreate>;
};

// com.atproto.repo.applyWrites.defs#update
struct ApplyWritesUpdate
{
    QString mCollection;
    QString mRKey;
    QJsonObject mValue;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<ApplyWritesUpdate>;
};

// com.atproto.repo.applyWrites.defs#delete
struct ApplyWritesDelete
{
    QString mCollection;
    QString mRKey;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<ApplyWritesDelete>;
};

using ApplyWritesType = std::variant<ApplyWritesCreate::Ptr, ApplyWritesUpdate::Ptr, ApplyWritesDelete::Ptr>;
using ApplyWritesList = std::vector<ApplyWritesType>;

}

// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon.h"
#include <QJsonDocument>
#include <QJsonObject>

namespace ATProto::ComATProtoRepo {


// com.atproto.repo.defs#commitMeta
struct CommitMeta
{
    QString mCid;
    QString mRev;

    using SharedPtr = std::shared_ptr<CommitMeta>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// com.atproto.repo.strongRef
struct StrongRef
{
    QString mUri;
    QString mCid;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<StrongRef>;
    using List = std::vector<SharedPtr>;
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
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// com.atproto.repo.listRecords#output
struct ListRecordsOutput
{
    std::optional<QString> mCursor;
    Record::List mRecords;

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
    static constexpr char const* TYPE = "com.atproto.repo.applyWrites#create";
};

// com.atproto.repo.applyWrites#update
struct ApplyWritesUpdate
{
    QString mCollection;
    QString mRKey;
    QJsonObject mValue;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ApplyWritesUpdate>;
    static constexpr char const* TYPE = "com.atproto.repo.applyWrites#update";
};

// com.atproto.repo.applyWrites#delete
struct ApplyWritesDelete
{
    QString mCollection;
    QString mRKey;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ApplyWritesDelete>;
    static constexpr char const* TYPE = "com.atproto.repo.applyWrites#delete";
};

using ApplyWritesType = std::variant<ApplyWritesCreate::SharedPtr, ApplyWritesUpdate::SharedPtr, ApplyWritesDelete::SharedPtr>;
using ApplyWritesList = std::vector<ApplyWritesType>;

// com.atproto.repo.applyWrites#createResult
struct ApplyWritesCreateResult
{
    QString mUri;
    QString mCid;
    std::optional<QString> mValidationStatus;

    using SharedPtr = std::shared_ptr<ApplyWritesCreateResult>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "com.atproto.repo.applyWrites#createResult";
};

// com.atproto.repo.applyWrites#updateResult
struct ApplyWritesUpdateResult
{
    QString mUri;
    QString mCid;
    std::optional<QString> mValidationStatus;

    using SharedPtr = std::shared_ptr<ApplyWritesUpdateResult>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "com.atproto.repo.applyWrites#updateResult";
};

// com.atproto.repo.applyWrites#deleteResult
struct ApplyWritesDeleteResult
{
    using SharedPtr = std::shared_ptr<ApplyWritesDeleteResult>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "com.atproto.repo.applyWrites#deleteResult";
};

using ApplyWritesResultType = std::variant<ApplyWritesCreateResult::SharedPtr, ApplyWritesUpdateResult::SharedPtr, ApplyWritesDeleteResult::SharedPtr>;
using ApplyWritesResultList = std::vector<ApplyWritesResultType>;

// com.atproto.repo.applyWrites#output
struct ApplyWritesOutput
{
    CommitMeta::SharedPtr mCommit; // optional
    ApplyWritesResultList mResults; // optional

    using SharedPtr = std::shared_ptr<ApplyWritesOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}

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

// com.atproto.repo.uploadBlob/output
struct UploadBlobOutput
{
    Blob::Ptr mBlob;

    using Ptr = std::unique_ptr<UploadBlobOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// com.atproto.repo.getRecord/output
struct Record
{
    QString mUri;
    std::optional<QString> mCid;
    QJsonObject mValue;

    using Ptr = std::unique_ptr<Record>;
    static Ptr fromJson(const QJsonObject& json);
};

}

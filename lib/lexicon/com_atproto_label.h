// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonObject>

namespace ATProto::ComATProtoLabel {

// com.atproto.label.defs#label
struct Label
{
    QString mSrc; // DID of creator
    QString mUri; // at-uri
    std::optional<QString> mCid;
    QString mVal;
    bool mNeg = false; // Negate the label
    QDateTime mCreatedAt;

    using Ptr = std::unique_ptr<Label>;
    static Ptr fromJson(const QJsonObject& json);
};

void getLabels(std::vector<Label::Ptr>& labels, const QJsonObject& json);

}

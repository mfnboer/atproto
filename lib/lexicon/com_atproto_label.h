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

using LabelList = std::vector<Label::Ptr>;
void getLabels(LabelList& labels, const QJsonObject& json);

// com.atproto.label.defs#selfLabel
struct SelfLabel
{
    QString mVal; // maxLength: 128
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<SelfLabel>;
    static Ptr fromJson(const QJsonObject& json);
};

using SelfLabelList = std::vector<SelfLabel::Ptr>;

// com.atproto.label.defs#selfLabels
struct SelfLabels
{
    SelfLabelList mValues; // max 10
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<SelfLabels>;
    static Ptr fromJson(const QJsonObject& json);
};

}

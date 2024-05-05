// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonObject>
#include <QtQmlIntegration>

namespace ATProto::ComATProtoLabel {

// com.atproto.label.defs#label
struct Label
{
    std::optional<int> mVersion;
    QString mSrc; // DID of creator
    QString mUri; // at-uri
    std::optional<QString> mCid;
    QString mVal;
    bool mNeg = false; // Negate the label
    QDateTime mCreatedAt;
    std::optional<QDateTime> mExpires;

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

// com.atproto.label.defs#labelValueDefinitionStrings
struct LabelValueDefinitionStrings
{
    QString mLang;
    QString mName;
    QString mDescription;

    using Ptr = std::unique_ptr<LabelValueDefinitionStrings>;
    static Ptr fromJson(const QJsonObject& json);
};

// com.atproto.label.defs#labelValueDefinition
struct LabelValueDefinition
{
    enum class Severity
    {
        INFORM,
        ALERT,
        NONE,
        UNKNOWN
    };
    static Severity stringToSeverity(const QString& str);

    enum class Blurs
    {
        CONTENT,
        MEDIA,
        NONE,
        UNKNOWN
    };
    static Blurs stringToBlurs(const QString& str);

    enum class Setting
    {
        IGNORE,
        WARN,
        HIDE,
        UNKNOWN
    };
    static Setting stringToSetting(const QString &str);

    QString mIdentifier;
    Severity mSeverity;
    QString mRawSeverity;
    Blurs mBlurs;
    QString mRawBlurs;
    Setting mDefaultSetting = Setting::WARN;
    QString mRawDefaultSetting = QStringLiteral("warn");
    bool mAdultOnly = false;
    std::vector<LabelValueDefinitionStrings::Ptr> mLocales;

    using Ptr = std::unique_ptr<LabelValueDefinition>;
    static Ptr fromJson(const QJsonObject& json);
};

}

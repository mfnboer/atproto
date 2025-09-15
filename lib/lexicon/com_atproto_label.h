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

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Label>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

void getLabels(Label::List& labels, const QJsonObject& json);

// com.atproto.label.defs#selfLabel
struct SelfLabel
{
    QString mVal; // maxLength: 128
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<SelfLabel>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// com.atproto.label.defs#selfLabels
struct SelfLabels
{
    SelfLabel::List mValues; // max 10
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<SelfLabels>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// com.atproto.label.defs#labelValueDefinitionStrings
struct LabelValueDefinitionStrings
{
    QString mLang;
    QString mName;
    QString mDescription;

    using SharedPtr = std::shared_ptr<LabelValueDefinitionStrings>;
    static SharedPtr fromJson(const QJsonObject& json);
};
using LabelValueDefinitionStringsList = std::vector<LabelValueDefinitionStrings::SharedPtr>;

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
    LabelValueDefinitionStringsList mLocales;

    using SharedPtr = std::shared_ptr<LabelValueDefinition>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}

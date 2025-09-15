// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_label.h"
#include "../xjson.h"

namespace ATProto::ComATProtoLabel {

QJsonObject Label::toJson() const
{
    QJsonObject json;
    XJsonObject::insertOptionalJsonValue<int>(json, "ver", mVersion);
    json.insert("src", mSrc);
    json.insert("uri", mUri);
    XJsonObject::insertOptionalJsonValue(json, "cid", mCid);
    json.insert("val", mVal);
    XJsonObject::insertOptionalJsonValue(json, "neg", mNeg, false);
    json.insert("cts", mCreatedAt.toString(Qt::ISODateWithMs));
    XJsonObject::insertOptionalDateTime(json, "exp", mExpires);

    return json;
}

Label::SharedPtr Label::fromJson(const QJsonObject& json)
{
    auto label = std::make_shared<Label>();
    XJsonObject xjson(json);
    label->mVersion = xjson.getOptionalInt("ver");
    label->mSrc = xjson.getRequiredString("src");
    label->mUri = xjson.getRequiredString("uri");
    label->mCid = xjson.getOptionalString("cid");
    label->mVal = xjson.getRequiredString("val");
    label->mNeg = xjson.getOptionalBool("neg", false);
    label->mCreatedAt = xjson.getRequiredDateTime("cts");
    label->mExpires = xjson.getOptionalDateTime("exp");
    return label;
}

void getLabels(Label::List& labels, const QJsonObject& json)
{
    XJsonObject xjson(json);
    labels = xjson.getOptionalVector<Label>("labels");
}

SelfLabel::SharedPtr SelfLabel::fromJson(const QJsonObject& json)
{
    auto label = std::make_shared<SelfLabel>();
    XJsonObject xjson(json);
    label->mJson = json;
    label->mVal = xjson.getRequiredString("val");
    return label;
}

QJsonObject SelfLabel::toJson() const
{
    QJsonObject json(mJson);
    json.insert("val", mVal);
    return json;
}

SelfLabels::SharedPtr SelfLabels::fromJson(const QJsonObject& json)
{
    auto labels = std::make_shared<SelfLabels>();
    XJsonObject xjson(json);
    labels->mJson = json;
    labels->mValues = xjson.getRequiredVector<SelfLabel>("values");
    return labels;
}

QJsonObject SelfLabels::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "com.atproto.label.defs#selfLabels");
    json.insert("values", XJsonObject::toJsonArray<SelfLabel>(mValues));
    return json;
}

LabelValueDefinitionStrings::SharedPtr LabelValueDefinitionStrings::fromJson(const QJsonObject& json)
{
    auto defStrings = std::make_shared<LabelValueDefinitionStrings>();
    XJsonObject xjson(json);
    defStrings->mLang = xjson.getRequiredString("lang");
    defStrings->mName = xjson.getRequiredString("name");
    defStrings->mDescription = xjson.getRequiredString("description");
    return defStrings;
}

LabelValueDefinition::Severity LabelValueDefinition::stringToSeverity(const QString& str)
{
    static const std::unordered_map<QString, Severity> mapping = {
        { "inform", Severity::INFORM },
        { "alert", Severity::ALERT },
        { "none", Severity::NONE }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    qWarning() << "Unknown severity:" << str;
    return Severity::UNKNOWN;
}

LabelValueDefinition::Blurs LabelValueDefinition::stringToBlurs(const QString& str)
{
    static const std::unordered_map<QString, Blurs> mapping = {
        { "content", Blurs::CONTENT },
        { "media", Blurs::MEDIA },
        { "none", Blurs::NONE }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    qWarning() << "Unknown blurs:" << str;
    return Blurs::UNKNOWN;
}

LabelValueDefinition::Setting LabelValueDefinition::stringToSetting(const QString& str)
{
    static const std::unordered_map<QString, Setting> mapping = {
        { "ignore", Setting::IGNORE },
        { "show", Setting::IGNORE },
        { "inform", Setting::IGNORE },
        { "warn", Setting::WARN },
        { "hide", Setting::HIDE }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    qWarning() << "Unknown settings:" << str;
    return Setting::UNKNOWN;
}

LabelValueDefinition::SharedPtr LabelValueDefinition::fromJson(const QJsonObject& json)
{
    auto def = std::make_shared<LabelValueDefinition>();
    XJsonObject xjson(json);
    def->mIdentifier = xjson.getRequiredString("identifier");
    def->mRawSeverity = xjson.getRequiredString("severity");
    def->mSeverity = stringToSeverity(def->mRawSeverity);
    def->mRawBlurs = xjson.getRequiredString("blurs");
    def->mBlurs = stringToBlurs(def->mRawBlurs);
    def->mRawDefaultSetting = xjson.getOptionalString("defaultSetting", "warn");
    def->mDefaultSetting = stringToSetting(def->mRawDefaultSetting);
    def->mAdultOnly = xjson.getOptionalBool("adultOnly", false);
    def->mLocales = xjson.getOptionalVector<LabelValueDefinitionStrings>("locales");
    return def;
}

}

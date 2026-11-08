// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_moderation.h"
#include "lexicon.h"
#include <QObject>
#include <map>

namespace ATProto::ComATProtoModeration {

QString categoryTypeToTitle(CategoryType category)
{
    switch (category)
    {
    case CategoryType::OZONE_VIOLENCE:
        return "Violence";
    case CategoryType::OZONE_SEXUAL:
        return "Adult content";
    case CategoryType::OZONE_CHILD:
        return "Child safety";
    case CategoryType::OZONE_HARASSMENT:
        return "Harassment or hate";
    case CategoryType::OZONE_MISLEADING:
        return "Misleading";
    case CategoryType::OZONE_RULE:
        return "Breaking site rules";
    case CategoryType::OZONE_SELF_HARM:
        return "Self-harm or dangerous behaviors";
    case CategoryType::OZONE_OTHER:
        return "Other";
    }

    Q_ASSERT(false);
    return "UNKNOWN";
}

QString categoryTypeToDescription(CategoryType category)
{
    switch (category)
    {
    case CategoryType::OZONE_VIOLENCE:
        return "Violent or threatening content";
    case CategoryType::OZONE_SEXUAL:
        return "Unlabeled, abusive, or non-consensual adult content";
    case CategoryType::OZONE_CHILD:
        return "Harming or endangering minors";
    case CategoryType::OZONE_HARASSMENT:
        return "Abusive or discriminatory behavior";
    case CategoryType::OZONE_MISLEADING:
        return "Spam or other inauthentic behavior or deception";
    case CategoryType::OZONE_RULE:
        return "Banned activities or security violations";
    case CategoryType::OZONE_SELF_HARM:
        return "Harmful or high-risk activities";
    case CategoryType::OZONE_OTHER:
        return "An issue not included in these options";
    }

    Q_ASSERT(false);
    return "UNKNOWN";
}

static const std::unordered_map<QString, ReasonType> stringReasonMap = {
    { "com.atproto.moderation.defs#reasonSpam", ReasonType::SPAM },
    { "com.atproto.moderation.defs#reasonViolation", ReasonType::VIOLATION },
    { "com.atproto.moderation.defs#reasonMisleading", ReasonType::MISLEADING },
    { "com.atproto.moderation.defs#reasonSexual", ReasonType::SEXUAL },
    { "com.atproto.moderation.defs#reasonRude", ReasonType::RUDE },
    { "com.atproto.moderation.defs#reasonOther", ReasonType::OTHER },
    { "com.atproto.moderation.defs#reasonAppeal", ReasonType::APPEAL },

    { "tools.ozone.report.defs#reasonAppeal", ReasonType::OZONE_APPEAL },
    { "tools.ozone.report.defs#reasonOther", ReasonType::OZONE_OTHER },

    { "tools.ozone.report.defs#reasonViolenceAnimal", ReasonType::OZONE_VIOLENCE_ANIMAL },
    { "tools.ozone.report.defs#reasonViolenceThreats", ReasonType::OZONE_VIOLENCE_THREATS },
    { "tools.ozone.report.defs#reasonViolenceGraphicContent", ReasonType::OZONE_VIOLENCE_GRAPHIC_CONTENT },
    { "tools.ozone.report.defs#reasonViolenceGlorification", ReasonType::OZONE_VIOLENCE_GLORIFICATION },
    { "tools.ozone.report.defs#reasonViolenceExtremistContent", ReasonType::OZONE_VIOLENCE_EXTREMIST_CONTENT },
    { "tools.ozone.report.defs#reasonViolenceTrafficking", ReasonType::OZONE_VIOLENCE_TRAFFICKING },
    { "tools.ozone.report.defs#reasonViolenceOther", ReasonType::OZONE_VIOLENCE_OTHER },

    { "tools.ozone.report.defs#reasonSexualAbuseContent", ReasonType::OZONE_SEXUAL_ABUSE_CONTENT },
    { "tools.ozone.report.defs#reasonSexualNCII", ReasonType::OZONE_SEXUAL_NCII },
    { "tools.ozone.report.defs#reasonSexualDeepFake", ReasonType::OZONE_SEXUAL_DEEP_FAKE },
    { "tools.ozone.report.defs#reasonSexualAnimal", ReasonType::OZONE_SEXUAL_ANIMAL },
    { "tools.ozone.report.defs#reasonSexualUnlabeled", ReasonType::OZONE_SEXUAL_UNLABELED },
    { "tools.ozone.report.defs#reasonSexualOther", ReasonType::OZONE_SEXUAL_OTHER },

    { "tools.ozone.report.defs#reasonChildSafetyCSAM", ReasonType::OZONE_CHILD_SAFETY_CSAM },
    { "tools.ozone.report.defs#reasonChildSafetyGroom", ReasonType::OZONE_CHILD_SAFETY_GROOM },
    { "tools.ozone.report.defs#reasonChildSafetyPrivacy", ReasonType::OZONE_CHILD_SAFETY_PRIVACY },
    { "tools.ozone.report.defs#reasonChildSafetyHarassment", ReasonType::OZONE_CHILD_SAFETY_HARASSMENT },
    { "tools.ozone.report.defs#reasonChildSafetyOther", ReasonType::OZONE_CHILD_SAFETY_OTHER },

    { "tools.ozone.report.defs#reasonHarassmentTroll", ReasonType::OZONE_HARASSMENT_TROLL },
    { "tools.ozone.report.defs#reasonHarassmentTargeted", ReasonType::OZONE_HARASSMENT_TARGETED },
    { "tools.ozone.report.defs#reasonHarassmentHateSpeech", ReasonType::OZONE_HARASSMENT_HATE_SPEECH },
    { "tools.ozone.report.defs#reasonHarassmentDoxxing", ReasonType::OZONE_HARASSMENT_DOXXING },
    { "tools.ozone.report.defs#reasonHarassmentOther", ReasonType::OZONE_HARASSMENT_OTHER },

    { "tools.ozone.report.defs#reasonMisleadingBot", ReasonType::OZONE_MISLEADING_BOT },
    { "tools.ozone.report.defs#reasonMisleadingImpersonation", ReasonType::OZONE_MISLEADING_IMPERSONATION },
    { "tools.ozone.report.defs#reasonMisleadingSpam", ReasonType::OZONE_MISLEADING_SPAM },
    { "tools.ozone.report.defs#reasonMisleadingScam", ReasonType::OZONE_MISLEADING_SCAM },
    { "tools.ozone.report.defs#reasonMisleadingElections", ReasonType::OZONE_MISLEADING_ELECTIONS },
    { "tools.ozone.report.defs#reasonMisleadingOther", ReasonType::OZONE_MISLEADING_OTHER },

    { "tools.ozone.report.defs#reasonRuleSecurity", ReasonType::OZONE_RULE_SITE_SECURITY },
    { "tools.ozone.report.defs#reasonProhibitedSales", ReasonType::OZONE_RULE_PROHIBITED_SALES },
    { "tools.ozone.report.defs#reasonRuleBanEvasion", ReasonType::OZONE_RULE_BAN_EVASION },
    { "tools.ozone.report.defs#reasonRuleOther", ReasonType::OZONE_RULE_OTHER },

    { "tools.ozone.report.defs#reasonSelfHarmContent", ReasonType::OZONE_SELF_HARM_CONTENT },
    { "tools.ozone.report.defs#reasonSelfHarmED", ReasonType::OZONE_SELF_HARM_ED },
    { "tools.ozone.report.defs#reasonSelfHarmStunts", ReasonType::OZONE_SELF_HARM_STUNTS },
    { "tools.ozone.report.defs#reasonSelfHarmSubstances", ReasonType::OZONE_SELF_HARM_SUBSTANCES },
    { "tools.ozone.report.defs#reasonSelfHarmOther", ReasonType::OZONE_SELF_HARM_OTHER }
};

QString reasonTypeToString(ReasonType reason)
{
    return enumToString(reason, stringReasonMap, "UKNOWN");
}

ReasonType stringToReasonType(const QString& str)
{
    return stringToEnum(str, stringReasonMap, ReasonType::OZONE_OTHER);
}

std::vector<ReasonType> getCategoryReasons(CategoryType category)
{
    static const std::map<CategoryType, std::vector<ReasonType>> CATEGORY_REASON_MAP =
    {
        {
            CategoryType::OZONE_VIOLENCE,
            {
                ReasonType::OZONE_VIOLENCE_ANIMAL,
                ReasonType::OZONE_VIOLENCE_THREATS,
                ReasonType::OZONE_VIOLENCE_GRAPHIC_CONTENT,
                ReasonType::OZONE_VIOLENCE_GLORIFICATION,
                ReasonType::OZONE_VIOLENCE_EXTREMIST_CONTENT,
                ReasonType::OZONE_VIOLENCE_TRAFFICKING,
                ReasonType::OZONE_VIOLENCE_OTHER
            }
        },
        {
            CategoryType::OZONE_SEXUAL,
            {
                ReasonType::OZONE_SEXUAL_ABUSE_CONTENT,
                ReasonType::OZONE_SEXUAL_NCII,
                ReasonType::OZONE_SEXUAL_DEEP_FAKE,
                ReasonType::OZONE_SEXUAL_ANIMAL,
                ReasonType::OZONE_SEXUAL_UNLABELED,
                ReasonType::OZONE_SEXUAL_OTHER
            }
        },
        {
            CategoryType::OZONE_CHILD,
            {
                ReasonType::OZONE_CHILD_SAFETY_CSAM,
                ReasonType::OZONE_CHILD_SAFETY_GROOM,
                ReasonType::OZONE_CHILD_SAFETY_PRIVACY,
                ReasonType::OZONE_CHILD_SAFETY_HARASSMENT,
                ReasonType::OZONE_CHILD_SAFETY_OTHER
            }
        },
        {
            CategoryType::OZONE_HARASSMENT,
            {
                ReasonType::OZONE_HARASSMENT_TROLL,
                ReasonType::OZONE_HARASSMENT_TARGETED,
                ReasonType::OZONE_HARASSMENT_HATE_SPEECH,
                ReasonType::OZONE_HARASSMENT_DOXXING,
                ReasonType::OZONE_HARASSMENT_OTHER
            }
        },
        {
            CategoryType::OZONE_MISLEADING,
            {
                ReasonType::OZONE_MISLEADING_BOT,
                ReasonType::OZONE_MISLEADING_IMPERSONATION,
                ReasonType::OZONE_MISLEADING_SPAM,
                ReasonType::OZONE_MISLEADING_SCAM,
                ReasonType::OZONE_MISLEADING_ELECTIONS,
                ReasonType::OZONE_MISLEADING_OTHER
            }
        },
        {
            CategoryType::OZONE_RULE,
            {
                ReasonType::OZONE_RULE_SITE_SECURITY,
                ReasonType::OZONE_RULE_PROHIBITED_SALES,
                ReasonType::OZONE_RULE_BAN_EVASION,
                ReasonType::OZONE_RULE_OTHER
            }
        },
        {
            CategoryType::OZONE_SELF_HARM,
            {
                ReasonType::OZONE_SELF_HARM_CONTENT,
                ReasonType::OZONE_SELF_HARM_ED,
                ReasonType::OZONE_SELF_HARM_STUNTS,
                ReasonType::OZONE_SELF_HARM_SUBSTANCES,
                ReasonType::OZONE_SELF_HARM_OTHER
            }
        },
        {
            CategoryType::OZONE_OTHER,
            {
                ReasonType::OZONE_OTHER
            }
        }
    };

    const auto it = CATEGORY_REASON_MAP.find(category);
    return it != CATEGORY_REASON_MAP.end() ? it->second : std::vector<ReasonType>{};
}

QString reasonTypeToTitle(ReasonType reason)
{
    switch (reason)
    {
    case ReasonType::SPAM:
        return "Spam";
    case ReasonType::VIOLATION:
        return "Violation";
    case ReasonType::MISLEADING:
        return "Misleading";
    case ReasonType::SEXUAL:
        return "Sexual";
    case ReasonType::RUDE:
        return "Rude";
    case ReasonType::OTHER:
        return "Other";
    case ReasonType::APPEAL:
        return "Appeal";

    case ReasonType::OZONE_APPEAL:
        return "Appeal";
    case ReasonType::OZONE_OTHER:
        return "Other";

    case ReasonType::OZONE_VIOLENCE_ANIMAL:
        return "Animal welfare";
    case ReasonType::OZONE_VIOLENCE_THREATS:
        return "Threats or incitement";
    case ReasonType::OZONE_VIOLENCE_GRAPHIC_CONTENT:
        return "Graphic violent content";
    case ReasonType::OZONE_VIOLENCE_GLORIFICATION:
        return "Glorification of violence";
    case ReasonType::OZONE_VIOLENCE_EXTREMIST_CONTENT:
        return "Extremist content";
    case ReasonType::OZONE_VIOLENCE_TRAFFICKING:
        return "Human trafficking";
    case ReasonType::OZONE_VIOLENCE_OTHER:
        return "Other violent content";

    case ReasonType::OZONE_SEXUAL_ABUSE_CONTENT:
        return "Adult sexual abuse content";
    case ReasonType::OZONE_SEXUAL_NCII:
        return "Non-consensual intimate imagery";
    case ReasonType::OZONE_SEXUAL_DEEP_FAKE:
        return "Deepfake adult content";
    case ReasonType::OZONE_SEXUAL_ANIMAL:
        return "Animal sexual abuse";
    case ReasonType::OZONE_SEXUAL_UNLABELED:
        return "Unlabeled adult content";
    case ReasonType::OZONE_SEXUAL_OTHER:
        return "Other sexual violence content";

    case ReasonType::OZONE_CHILD_SAFETY_CSAM:
        return "Child Sexual Abuse Material (CSAM)";
    case ReasonType::OZONE_CHILD_SAFETY_GROOM:
        return "Grooming or predatory behavior";
    case ReasonType::OZONE_CHILD_SAFETY_PRIVACY:
        return "Privacy violation of a minor";
    case ReasonType::OZONE_CHILD_SAFETY_HARASSMENT:
        return "Minor harassment or bullying";
    case ReasonType::OZONE_CHILD_SAFETY_OTHER:
        return "Other child safety issue";

    case ReasonType::OZONE_HARASSMENT_TROLL:
        return "Trolling";
    case ReasonType::OZONE_HARASSMENT_TARGETED:
        return "Targeted harassment";
    case ReasonType::OZONE_HARASSMENT_HATE_SPEECH:
        return "Hate speech";
    case ReasonType::OZONE_HARASSMENT_DOXXING:
        return "Doxxing";
    case ReasonType::OZONE_HARASSMENT_OTHER:
        return "Other harassing or hateful content";

    case ReasonType::OZONE_MISLEADING_BOT:
        return "Fake account or bot";
    case ReasonType::OZONE_MISLEADING_IMPERSONATION:
        return "Impersonation";
    case ReasonType::OZONE_MISLEADING_SPAM:
        return "Spam";
    case ReasonType::OZONE_MISLEADING_SCAM:
        return "Scam";
    case ReasonType::OZONE_MISLEADING_ELECTIONS:
        return "False information about elections";
    case ReasonType::OZONE_MISLEADING_OTHER:
        return "Other misleading content";

    case ReasonType::OZONE_RULE_SITE_SECURITY:
        return "Hacking or system attacks";
    case ReasonType::OZONE_RULE_PROHIBITED_SALES:
        return "Promoting or selling prohibited items or services";
    case ReasonType::OZONE_RULE_BAN_EVASION:
        return "Banned user returning";
    case ReasonType::OZONE_RULE_OTHER:
        return "Other network rule-breaking";

    case ReasonType::OZONE_SELF_HARM_CONTENT:
        return "Content promoting or depicting self-harm";
    case ReasonType::OZONE_SELF_HARM_ED:
        return "Eating disorders";
    case ReasonType::OZONE_SELF_HARM_STUNTS:
        return "Dangerous challenges or activities";
    case ReasonType::OZONE_SELF_HARM_SUBSTANCES:
        return "Dangerous substances or drug abuse";
    case ReasonType::OZONE_SELF_HARM_OTHER:
        return "Other dangerous content";
    }

    Q_ASSERT(false);
    return "UNKNOWN";
}

QString reasonStringToTitle(const QString& str)
{
    return reasonTypeToTitle(stringToReasonType(str));
}

QJsonObject ModTool::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("name", mName);

    if (mMeta)
        json.insert("meta", *mMeta);

    return json;
}

}

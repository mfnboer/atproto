// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "network_utils.h"
#include "xjson.h"
#include <QJsonDocument>

namespace ATProto {

bool NetworkUtils::isSafeUrl(const QUrl url)
{
    if (url.scheme() != "https" ||
        url.host().isEmpty() ||
        !url.userName().isEmpty() ||
        !url.password().isEmpty() ||
        url.port() > -1)
    {
        qWarning() << "Unsafe URL:" << url;
        return false;
    }

    const auto segments = url.host().split('.');

    if (segments.length() < 2)
    {
        qWarning() << "Not enough segments in host:" << url;
        return false;
    }

    static const std::unordered_set<QString> FORBIDDEN_DOMAINS = {"local", "arpa", "internal", "localhost"};

    if (FORBIDDEN_DOMAINS.contains(segments.last()))
    {
        qWarning() << "Forbidden domain in URL:" << url;
        return false;
    }

    const QChar lastChar = url.host().back();

    if (lastChar.isDigit())
    {
        qWarning() << "IP address as host:" << url;
        return false;
    }

    return true;
}

static QStringList parseHttpList(const QString& value) {
    QStringList result;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < value.size(); ++i)
    {
        QChar c = value[i];

        if (c == '"')
        {
            inQuotes = !inQuotes;
            current += c;
        }
        else if (c == ',' && !inQuotes)
        {
            result.append(current.trimmed());
            current.clear();
        }
        else
        {
            current += c;
        }
    }

    if (!current.trimmed().isEmpty())
        result << current.trimmed();

    return result;
}

static QString unquote(const QString& value)
{
    if (value.length() > 1 && value.startsWith('"') && value.endsWith('"'))
        return value.mid(1, value.length() - 2);

    return value;
}

static std::pair<QString, std::unordered_map<QString, QString>> parseWwwAuthenticate(const QString& headerValue)
{
    int index = headerValue.indexOf(' ');
    const QString scheme = headerValue.mid(0, index);
    const QString paramString = headerValue.sliced(index + 1);
    const QStringList paramList = parseHttpList(paramString);
    std::unordered_map<QString, QString> params;

    for (const auto& param : paramList)
    {
        const auto kvList = param.split('=');
        const QString key = unquote(kvList[0].trimmed());
        const QString value = kvList.size() > 1 ? unquote(kvList[1].trimmed()) : "";
        params[key] = value;
    }

    return { scheme, params };
}

static bool isDpopError(const QString& error)
{
    return error == "use_dpop_nonce" || error == "invalid_dpop_proof";
}

bool NetworkUtils::isDpopNonceError(QNetworkReply* reply, const QByteArray& data)
{
    const QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (!status.isValid())
        return false;

    const int statusCode = status.toInt();

    if (statusCode != 400 && statusCode != 401)
        return false;

    if (reply->hasRawHeader("WWW-Authenticate"))
    {
        QByteArray headerValue = reply->rawHeader("WWW-Authenticate");
        auto [scheme, params] = parseWwwAuthenticate(headerValue);
        qDebug() << QString(headerValue);

        if (scheme.toLower() == "dpop" && isDpopError(params["error"]))
            return true;
    }

    const QJsonDocument json(QJsonDocument::fromJson(data));
    qDebug() << "body:" << json;

    if (json.isObject())
    {
        const auto jsonObject = json.object();
        const XJsonObject xjson(jsonObject);
        const auto errorField = xjson.getOptionalString("error");

        if (errorField && isDpopError(*errorField))
            return true;
    }

    return false;
}

bool NetworkUtils::hasDpopNonce(QNetworkReply* reply)
{
    return reply->hasRawHeader("DPoP-Nonce");
}

QString NetworkUtils::getDpopNonce(QNetworkReply* reply)
{
    return reply->rawHeader("DPoP-Nonce");
}

}

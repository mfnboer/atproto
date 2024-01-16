// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QString>

namespace ATProto {

class ATUri
{
public:
    using ErrorCb = std::function<void(const QString& error, const QString& msg)>;

    static constexpr char const* COLLECTION_FEED_GENERATOR = "app.bsky.feed.generator";
    static constexpr char const* COLLECTION_FEED_POST = "app.bsky.feed.post";
    static constexpr char const* COLLECTION_GRAPH_LIST = "app.bsky.graph.list";
    
    static ATUri fromHttpsPostUri(const QString& uri);
    static ATUri fromHttpsFeedUri(const QString& uri);
    static ATUri fromHttpsListUri(const QString& uri);
    static ATUri createAtUri(const QString& uri, const QObject& presence, const ErrorCb& errorCb);

    ATUri() = default;
    explicit ATUri(const QString& uri);

    bool isValid() const;
    const QString& getAuthority() const { return mAuthority; }
    const QString& getCollection() const { return mCollection; }
    const QString& getRkey() const { return mRkey; }
    bool authorityIsHandle() const { return mAuthorityIsHandle; }

    void setAuthority(const QString& authority) { mAuthority = authority; }
    void setCollection(const QString& collection) { mCollection = collection; }
    void setRKey(const QString& rKey) { mRkey = rKey; }
    void setAuthorityIsHandle(bool isHandle) { mAuthorityIsHandle = isHandle; }

    QString toString() const;

private:
    QString mAuthority;
    QString mCollection;
    QString mRkey;

    // Authority is a handle that must be resolved to a DID
    bool mAuthorityIsHandle = false;
};

}

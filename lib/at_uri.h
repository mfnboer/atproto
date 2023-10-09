// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>

namespace ATProto {

class ATUri
{
public:
    using ErrorCb = std::function<void(const QString& err)>;
    
    static ATUri fromHttpsPostUri(const QString& uri);
    static ATUri createAtUri(const QString& uri, const QObject& presence, const ErrorCb& errorCb);

    ATUri() = default;
    explicit ATUri(const QString& uri);

    const bool isValid() const;
    const QString& getAuthority() const { return mAuthority; }
    const QString& getCollection() const { return mCollection; }
    const QString& getRkey() const { return mRkey; }
    const bool authorityIsHandle() const { return mAuthorityIsHandle; }

    void setAuthority(const QString& authority) { mAuthority = authority; }
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

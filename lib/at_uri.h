// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>

namespace ATProto {

class ATUri
{
public:
    explicit ATUri(const QString& uri);

    const bool isValid() const;
    const QString& getAuthority() const { return mAuthority; }
    const QString& getCollection() const { return mCollection; }
    const QString& getRkey() const { return mRkey; }

private:
    QString mAuthority;
    QString mCollection;
    QString mRkey;
};

}

// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "test.h"

namespace ATProto {

ATProtoTest::ATProtoTest(QObject* parent) : QObject(parent)
{
    mNetwork = new QNetworkAccessManager(this);
    mNetwork->setAutoDeleteReplies(true);
}

}

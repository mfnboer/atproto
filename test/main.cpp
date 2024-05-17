// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "test_at_uri.h"
#include "test_rich_text_master.h"
#include "test_xjson.h"
#include <QTest>

int main(int argc, char *argv[])
{
    TestAtUri testAtUri;
    QTest::qExec(&testAtUri, argc, argv);

    TestRichTextMaster testRichTextMaster;
    QTest::qExec(&testRichTextMaster, argc, argv);

    TestXJson testXJson;
    QTest::qExec(&testXJson, argc, argv);

    return 0;
}

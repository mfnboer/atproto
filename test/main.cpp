// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "test_at_uri.h"
#include "test_rich_text_master.h"
#include <QTest>

int main(int argc, char *argv[])
{
    TestAtUri testAtUri;
    QTest::qExec(&testAtUri, argc, argv);

    TestRichTextMaster testRichTextMaster;
    QTest::qExec(&testRichTextMaster, argc, argv);

    return 0;
}

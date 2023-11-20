// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "test_at_uri.h"
#include "test_post_master.h"
#include <QObject>
#include <QTest>

int main(int argc, char *argv[])
{
    int status = 0;
    auto runTest = [&status, argc, argv](QObject* obj) {
        status |= QTest::qExec(obj, argc, argv);
    };

    runTest(std::make_unique<TestAtUri>().get());
    runTest(std::make_unique<TestPostMaster>().get());

    return status;
}

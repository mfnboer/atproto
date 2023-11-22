// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include <at_uri.h>
#include <QTest>

using namespace ATProto;

class TestAtUri : public QObject
{
    Q_OBJECT
private slots:
    void emptyUri()
    {
        ATUri atUri;
        QVERIFY(!atUri.isValid());
    }

    void postUri()
    {
        ATUri atUri("at://did:plc:zzmeflm2wzrrgcaam6bw3kaf/app.bsky.feed.post/3kdiw4gsx3f2k");
        QVERIFY(atUri.isValid());
        QCOMPARE(atUri.getAuthority(), "did:plc:zzmeflm2wzrrgcaam6bw3kaf");
        QCOMPARE(atUri.getCollection(), "app.bsky.feed.post");
        QCOMPARE(atUri.getRkey(), "3kdiw4gsx3f2k");
        QVERIFY(!atUri.authorityIsHandle());
        QCOMPARE(atUri.toString(), "at://did:plc:zzmeflm2wzrrgcaam6bw3kaf/app.bsky.feed.post/3kdiw4gsx3f2k");
    }

    void httpsHandlePostUri()
    {
        ATUri atUri = ATUri::fromHttpsPostUri("https://bsky.app/profile/skywalker.bsky.social/post/rkey");
        QVERIFY(atUri.isValid());
        QVERIFY(atUri.authorityIsHandle());
        QCOMPARE(atUri.getAuthority(), "skywalker.bsky.social");
        QCOMPARE(atUri.getCollection(), "app.bsky.feed.post");
        QCOMPARE(atUri.getRkey(), "rkey");
    }

    void httpsDidPostUri()
    {
        ATUri atUri = ATUri::fromHttpsPostUri("https://bsky.app/profile/did:plc:zzmeflm2wzrrgcaam6bw3kaf/post/rkey");
        QVERIFY(atUri.isValid());
        QVERIFY(!atUri.authorityIsHandle());
        QCOMPARE(atUri.getAuthority(), "did:plc:zzmeflm2wzrrgcaam6bw3kaf");
        QCOMPARE(atUri.getCollection(), "app.bsky.feed.post");
        QCOMPARE(atUri.getRkey(), "rkey");
    }
};

QTEST_MAIN(TestAtUri)

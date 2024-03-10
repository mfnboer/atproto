// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <rich_text_master.h>
#include <QTest>

using namespace ATProto;

class TestRichTextMaster : public QObject
{
    Q_OBJECT
private slots:
    void parsePartialMentions()
    {
        std::vector<RichTextMaster::ParsedMatch> matches;

        matches = RichTextMaster::parsePartialMentions("");
        QVERIFY(matches.empty());

        matches = RichTextMaster::parsePartialMentions("Hello world");
        QVERIFY(matches.empty());

        matches = RichTextMaster::parsePartialMentions("@sky");
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mType, RichTextMaster::ParsedMatch::Type::PARTIAL_MENTION);
        QCOMPARE(matches[0].mMatch, "@sky");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 4);
    }

    void parseMentions()
    {
        std::vector<RichTextMaster::ParsedMatch> matches;

        matches = RichTextMaster::parseMentions("");
        QVERIFY(matches.empty());

        matches = RichTextMaster::parseMentions("Hello world");
        QVERIFY(matches.empty());

        matches = RichTextMaster::parseMentions("@sky");
        QVERIFY(matches.empty());

        matches = RichTextMaster::parseMentions("@skywalkerapp.bsky.social");
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mType, RichTextMaster::ParsedMatch::Type::MENTION);
        QCOMPARE(matches[0].mMatch, "@skywalkerapp.bsky.social");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 25);

        matches = RichTextMaster::parseMentions("Hello @skywalkerapp.bsky.social !");
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mType, RichTextMaster::ParsedMatch::Type::MENTION);
        QCOMPARE(matches[0].mMatch, "@skywalkerapp.bsky.social");
        QCOMPARE(matches[0].mStartIndex, 6);
        QCOMPARE(matches[0].mEndIndex, 31);

        matches = RichTextMaster::parseMentions("Hello @skywalkerapp.bsky.social and @michelbestaat.bsky.social !");
        QCOMPARE(matches.size(), 2);
        QCOMPARE(matches[0].mType, RichTextMaster::ParsedMatch::Type::MENTION);
        QCOMPARE(matches[0].mMatch, "@skywalkerapp.bsky.social");
        QCOMPARE(matches[0].mStartIndex, 6);
        QCOMPARE(matches[0].mEndIndex, 31);
        QCOMPARE(matches[1].mType, RichTextMaster::ParsedMatch::Type::MENTION);
        QCOMPARE(matches[1].mMatch, "@michelbestaat.bsky.social");
        QCOMPARE(matches[1].mStartIndex, 36);
        QCOMPARE(matches[1].mEndIndex, 62);
    }

    void parseLinks()
    {
        std::vector<RichTextMaster::ParsedMatch> matches;

        matches = RichTextMaster::parseLinks("");
        QVERIFY(matches.empty());

        matches = RichTextMaster::parseLinks("Hello world");
        QVERIFY(matches.empty());

        matches = RichTextMaster::parseLinks("https://bsky.app");
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mType, RichTextMaster::ParsedMatch::Type::LINK);
        QCOMPARE(matches[0].mMatch, "https://bsky.app");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 16);

        matches = RichTextMaster::parseLinks("bsky.app");
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mType, RichTextMaster::ParsedMatch::Type::LINK);
        QCOMPARE(matches[0].mMatch, "bsky.app");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 8);

        matches = RichTextMaster::parseLinks("bsky.app and www.google.com");
        QCOMPARE(matches.size(), 2);
        QCOMPARE(matches[0].mType, RichTextMaster::ParsedMatch::Type::LINK);
        QCOMPARE(matches[0].mMatch, "bsky.app");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 8);
        QCOMPARE(matches[1].mType, RichTextMaster::ParsedMatch::Type::LINK);
        QCOMPARE(matches[1].mMatch, "www.google.com");
        QCOMPARE(matches[1].mStartIndex, 13);
        QCOMPARE(matches[1].mEndIndex, 27);
    }

    void invalidTLD()
    {
        auto matches = RichTextMaster::parseLinks("wwww.hello.aslkjaweioj1");
        QVERIFY(matches.empty());
    }

    void parseTags()
    {
        std::vector<RichTextMaster::ParsedMatch> matches;

        matches = RichTextMaster::parseTags("");
        QVERIFY(matches.empty());

        matches = RichTextMaster::parseTags("Hello world");
        QVERIFY(matches.empty());

        matches = RichTextMaster::parseTags("#tag");
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mType, RichTextMaster::ParsedMatch::Type::TAG);
        QCOMPARE(matches[0].mMatch, "#tag");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 4);

        matches = RichTextMaster::parseTags("#tag1 #tag2 #123 #️⃣tagX");
        QCOMPARE(matches.size(), 2);
        QCOMPARE(matches[0].mType, RichTextMaster::ParsedMatch::Type::TAG);
        QCOMPARE(matches[0].mMatch, "#tag1");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 5);
        QCOMPARE(matches[1].mType, RichTextMaster::ParsedMatch::Type::TAG);
        QCOMPARE(matches[1].mMatch, "#tag2");
        QCOMPARE(matches[1].mStartIndex, 6);
        QCOMPARE(matches[1].mEndIndex, 11);
    }

    void parseFacets()
    {
        std::vector<RichTextMaster::ParsedMatch> matches;
        matches = RichTextMaster::parseFacets("Hello @skywalkerapp.bsky.social bsky.app #sky.");
        QCOMPARE(matches.size(), 3);
        QCOMPARE(matches[0].mType, RichTextMaster::ParsedMatch::Type::MENTION);
        QCOMPARE(matches[0].mMatch, "@skywalkerapp.bsky.social");
        QCOMPARE(matches[0].mStartIndex, 6);
        QCOMPARE(matches[0].mEndIndex, 31);
        QCOMPARE(matches[1].mType, RichTextMaster::ParsedMatch::Type::LINK);
        QCOMPARE(matches[1].mMatch, "bsky.app");
        QCOMPARE(matches[1].mStartIndex, 32);
        QCOMPARE(matches[1].mEndIndex, 40);
        QCOMPARE(matches[2].mType, RichTextMaster::ParsedMatch::Type::TAG);
        QCOMPARE(matches[2].mMatch, "#sky");
        QCOMPARE(matches[2].mStartIndex, 41);
        QCOMPARE(matches[2].mEndIndex, 45);
    }

    void isHashtag()
    {
        QVERIFY(RichTextMaster::isHashtag("#tag"));
        QVERIFY(RichTextMaster::isHashtag("#TAG"));
        QVERIFY(RichTextMaster::isHashtag("#t42"));
        QVERIFY(RichTextMaster::isHashtag("#42T"));
        QVERIFY(RichTextMaster::isHashtag("#😊"));
    }

    void isNotHashtag()
    {
        QVERIFY(!RichTextMaster::isHashtag("tag"));
        QVERIFY(!RichTextMaster::isHashtag("#tag x"));
        QVERIFY(!RichTextMaster::isHashtag("#42"));
        QVERIFY(!RichTextMaster::isHashtag("#"));
        QVERIFY(!RichTextMaster::isHashtag("#tag;"));
        QVERIFY(!RichTextMaster::isHashtag("#tag."));
        QVERIFY(!RichTextMaster::isHashtag("#tag?"));
        QVERIFY(!RichTextMaster::isHashtag("#️⃣tag"));
    }
};

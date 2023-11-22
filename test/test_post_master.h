// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <post_master.h>
#include <QTest>

using namespace ATProto;

class TestPostMaster : public QObject
{
    Q_OBJECT
private slots:
    void parsePartialMentions()
    {
        std::vector<PostMaster::ParsedMatch> matches;

        matches = PostMaster::parsePartialMentions("");
        QVERIFY(matches.empty());

        matches = PostMaster::parsePartialMentions("Hello world");
        QVERIFY(matches.empty());

        matches = PostMaster::parsePartialMentions("@sky");
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mType, PostMaster::ParsedMatch::Type::PARTIAL_MENTION);
        QCOMPARE(matches[0].mMatch, "@sky");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 4);
    }

    void parseMentions()
    {
        std::vector<PostMaster::ParsedMatch> matches;

        matches = PostMaster::parseMentions("");
        QVERIFY(matches.empty());

        matches = PostMaster::parseMentions("Hello world");
        QVERIFY(matches.empty());

        matches = PostMaster::parseMentions("@sky");
        QVERIFY(matches.empty());

        matches = PostMaster::parseMentions("@skywalkerapp.bsky.social");
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mType, PostMaster::ParsedMatch::Type::MENTION);
        QCOMPARE(matches[0].mMatch, "@skywalkerapp.bsky.social");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 25);

        matches = PostMaster::parseMentions("Hello @skywalkerapp.bsky.social !");
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mType, PostMaster::ParsedMatch::Type::MENTION);
        QCOMPARE(matches[0].mMatch, "@skywalkerapp.bsky.social");
        QCOMPARE(matches[0].mStartIndex, 6);
        QCOMPARE(matches[0].mEndIndex, 31);

        matches = PostMaster::parseMentions("Hello @skywalkerapp.bsky.social and @michelbestaat.bsky.social !");
        QCOMPARE(matches.size(), 2);
        QCOMPARE(matches[0].mType, PostMaster::ParsedMatch::Type::MENTION);
        QCOMPARE(matches[0].mMatch, "@skywalkerapp.bsky.social");
        QCOMPARE(matches[0].mStartIndex, 6);
        QCOMPARE(matches[0].mEndIndex, 31);
        QCOMPARE(matches[1].mType, PostMaster::ParsedMatch::Type::MENTION);
        QCOMPARE(matches[1].mMatch, "@michelbestaat.bsky.social");
        QCOMPARE(matches[1].mStartIndex, 36);
        QCOMPARE(matches[1].mEndIndex, 62);
    }

    void parseLinks()
    {
        std::vector<PostMaster::ParsedMatch> matches;

        matches = PostMaster::parseLinks("");
        QVERIFY(matches.empty());

        matches = PostMaster::parseLinks("Hello world");
        QVERIFY(matches.empty());

        matches = PostMaster::parseLinks("https://bsky.app");
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mType, PostMaster::ParsedMatch::Type::LINK);
        QCOMPARE(matches[0].mMatch, "https://bsky.app");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 16);

        matches = PostMaster::parseLinks("bsky.app");
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mType, PostMaster::ParsedMatch::Type::LINK);
        QCOMPARE(matches[0].mMatch, "bsky.app");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 8);

        matches = PostMaster::parseLinks("bsky.app and www.google.com");
        QCOMPARE(matches.size(), 2);
        QCOMPARE(matches[0].mType, PostMaster::ParsedMatch::Type::LINK);
        QCOMPARE(matches[0].mMatch, "bsky.app");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 8);
        QCOMPARE(matches[1].mType, PostMaster::ParsedMatch::Type::LINK);
        QCOMPARE(matches[1].mMatch, "www.google.com");
        QCOMPARE(matches[1].mStartIndex, 13);
        QCOMPARE(matches[1].mEndIndex, 27);
    }

    void invalidTLD()
    {
        auto matches = PostMaster::parseLinks("wwww.hello.aslkjaweioj1");
        QVERIFY(matches.empty());
    }

    void parseTags()
    {
        std::vector<PostMaster::ParsedMatch> matches;

        matches = PostMaster::parseTags("");
        QVERIFY(matches.empty());

        matches = PostMaster::parseTags("Hello world");
        QVERIFY(matches.empty());

        matches = PostMaster::parseTags("#tag");
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].mType, PostMaster::ParsedMatch::Type::TAG);
        QCOMPARE(matches[0].mMatch, "#tag");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 4);

        matches = PostMaster::parseTags("#tag1 #tag2");
        QCOMPARE(matches.size(), 2);
        QCOMPARE(matches[0].mType, PostMaster::ParsedMatch::Type::TAG);
        QCOMPARE(matches[0].mMatch, "#tag1");
        QCOMPARE(matches[0].mStartIndex, 0);
        QCOMPARE(matches[0].mEndIndex, 5);
        QCOMPARE(matches[1].mType, PostMaster::ParsedMatch::Type::TAG);
        QCOMPARE(matches[1].mMatch, "#tag2");
        QCOMPARE(matches[1].mStartIndex, 6);
        QCOMPARE(matches[1].mEndIndex, 11);
    }

    void parseFacets()
    {
        std::vector<PostMaster::ParsedMatch> matches;
        matches = PostMaster::parseFacets("Hello @skywalkerapp.bsky.social bsky.app #sky.");
        QCOMPARE(matches.size(), 3);
        QCOMPARE(matches[0].mType, PostMaster::ParsedMatch::Type::MENTION);
        QCOMPARE(matches[0].mMatch, "@skywalkerapp.bsky.social");
        QCOMPARE(matches[0].mStartIndex, 6);
        QCOMPARE(matches[0].mEndIndex, 31);
        QCOMPARE(matches[1].mType, PostMaster::ParsedMatch::Type::LINK);
        QCOMPARE(matches[1].mMatch, "bsky.app");
        QCOMPARE(matches[1].mStartIndex, 32);
        QCOMPARE(matches[1].mEndIndex, 40);
        QCOMPARE(matches[2].mType, PostMaster::ParsedMatch::Type::TAG);
        QCOMPARE(matches[2].mMatch, "#sky");
        QCOMPARE(matches[2].mStartIndex, 41);
        QCOMPARE(matches[2].mEndIndex, 45);
    }
};

QTEST_MAIN(TestPostMaster)

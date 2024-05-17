// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <lexicon/chat_bsky_convo.h>
#include <xjson.h>
#include <QJsonDocument>
#include <QTest>

using namespace ATProto;

class TestXJson : public QObject
{
    Q_OBJECT
private slots:
    void requiredVariantMessageView()
    {
        auto json = QJsonDocument::fromJson(LOG_CREATE_MESSAGE);
        QVERIFY(!json.isEmpty());
        auto lcm = ChatBskyConvo::LogCreateMessage::fromJson(json.object());
        QVERIFY(lcm);
        auto messageView = std::get_if<ChatBskyConvo::MessageView::Ptr>(&lcm->mMessage);
        QVERIFY(messageView);
        QCOMPARE((*messageView)->mId, "m1");
    }

    void requiredVariantDeletedMessageView()
    {
        auto json = QJsonDocument::fromJson(LOG_CREATE_DELETED_MESSAGE);
        QVERIFY(!json.isEmpty());
        auto lcm = ChatBskyConvo::LogCreateMessage::fromJson(json.object());
        QVERIFY(lcm);
        auto messageView = std::get_if<ChatBskyConvo::DeletedMessageView::Ptr>(&lcm->mMessage);
        QVERIFY(messageView);
        QCOMPARE((*messageView)->mId, "m2");
    }

    void requiredVariantUnknown()
    {
        QTest::ignoreMessage(QtWarningMsg, "Unknown type: \"chat.bsky.convo.defs#unknown\" key: \"message\"");
        auto json = QJsonDocument::fromJson(LOG_CREATE_UNKNOWN);
        QVERIFY(!json.isEmpty());
        auto lcm = ChatBskyConvo::LogCreateMessage::fromJson(json.object());
        QVERIFY(lcm);
        auto messageView = std::get_if<ChatBskyConvo::MessageView::Ptr>(&lcm->mMessage);
        QVERIFY(messageView);
        QVERIFY(!*messageView);
        QCOMPARE(lcm->mMessage, VariantType{});
    }

private:
    using VariantType = std::variant<ChatBskyConvo::MessageView::Ptr, ChatBskyConvo::DeletedMessageView::Ptr>;

    static constexpr char const* LOG_CREATE_MESSAGE = R"##({
        "rev": "c1",
        "convoId": "c42",
        "message": {
            "$type": "chat.bsky.convo.defs#messageView",
            "id": "m1",
            "rev": "m42",
            "text": "foo",
            "sender": {
                "did": "did:sender"
            },
            "sentAt": "2024-04-14T20:48:40.913Z"
        }
    })##";

    static constexpr char const* LOG_CREATE_DELETED_MESSAGE = R"##({
        "rev": "c2",
        "convoId": "c43",
        "message": {
            "$type": "chat.bsky.convo.defs#deletedMessageView",
            "id": "m2",
            "rev": "m43",
            "sender": {
                "did": "did:sender"
            },
            "sentAt": "2024-04-14T20:48:40.913Z"
        }
    })##";

    static constexpr char const* LOG_CREATE_UNKNOWN = R"##({
        "rev": "c2",
        "convoId": "c43",
        "message": {
            "$type": "chat.bsky.convo.defs#unknown",
            "id": "m2",
            "rev": "m43",
            "sender": {
                "did": "did:sender"
            },
            "sentAt": "2024-04-14T20:48:40.913Z"
        }
    })##";
};

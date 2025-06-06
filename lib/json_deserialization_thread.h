// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>
#include <QThread>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace Xrpc {

class JsonDeserializationThread : public QThread
{
    Q_OBJECT

public:
    using Callback = std::function<void(QJsonDocument json)>;

    JsonDeserializationThread(QObject* parent = nullptr);
    void push(QByteArray data, Callback cb);
    void cancelAll();

signals:
    void taskDone(QJsonDocument json, Callback cb);

protected:
    virtual void run() override;

private:
    struct Task
    {
        Task() = default;
        Task(QByteArray data, Callback cb);

        QByteArray mData;
        Callback mCallback;
        bool mCancel = false;
    };

    Task pop();

    std::queue<Task> mTasks;
    mutable std::mutex mQueueMutex;
    std::condition_variable mQueueNotEmpty;
};

}

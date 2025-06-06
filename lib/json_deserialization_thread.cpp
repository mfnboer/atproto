// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "json_deserialization_thread.h"

namespace Xrpc {

using namespace std::chrono_literals;

JsonDeserializationThread::Task::Task(QByteArray data, Callback cb) :
    mData{std::move(data)},
    mCallback{std::move(cb)}
{
}

JsonDeserializationThread::JsonDeserializationThread(QObject* parent) :
    QThread(parent)
{
}

void JsonDeserializationThread::push(QByteArray data, Callback cb)
{
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        mTasks.push(Task{std::move(data), std::move(cb)});
    }

    mQueueNotEmpty.notify_one();
}

void JsonDeserializationThread::cancelAll()
{
    qDebug() << "Cancel JSON deserializer thread";

    {
        std::lock_guard<std::mutex> lock(mQueueMutex);

        while (!mTasks.empty())
            mTasks.pop();

        Task cancelTask;
        cancelTask.mCancel = true;
        mTasks.push(cancelTask);
    }

    mQueueNotEmpty.notify_one();
}

JsonDeserializationThread::Task JsonDeserializationThread::pop()
{
    std::unique_lock<std::mutex> lock(mQueueMutex);

    while (mTasks.empty())
        mQueueNotEmpty.wait(lock);

    Task task = std::move(mTasks.front());
    mTasks.pop();
    return task;
}

void JsonDeserializationThread::run()
{
    qDebug() << "JSON deserializer thread running";

    forever
    {
        Task task = pop();

        if (task.mCancel)
        {
            qDebug() << "JSON deserializer thread finished";
            return;
        }

        const auto startTime = std::chrono::high_resolution_clock::now();
        QJsonDocument json(QJsonDocument::fromJson(task.mData));
        const auto endTime = std::chrono::high_resolution_clock::now();
        qDebug() << "DESERIALIZE JSON DT:" << (endTime - startTime) / 1us;
        emit taskDone(std::move(json), std::move(task.mCallback));
    }
}

}

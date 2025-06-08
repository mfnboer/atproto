// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "time_monitor.h"
#include <QDebug>

namespace ATProto {

using namespace std::chrono_literals;

TimeMonitor::TimeMonitor(const QString& logBefore, const QString& logAfter) :
    mLogBefore(logBefore),
    mLogAfter(logAfter),
    mHandle(
        [this]{
            auto endTime = std::chrono::high_resolution_clock::now();
            auto durationUs = (endTime - mStartTime) / 1us;
            qDebug() << mLogBefore << ":" << durationUs << "us" << mLogAfter;
        }),
    mStartTime(std::chrono::high_resolution_clock::now())
{
}

}

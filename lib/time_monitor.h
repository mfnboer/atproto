// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "scoped_handle.h"

namespace ATProto {

class TimeMonitor
{
public:
    explicit TimeMonitor(const QString& logBefore, const QString& logAfter = "");

private:
    QString mLogBefore;
    QString mLogAfter;
    ScopedHandle mHandle;
    std::chrono::time_point<std::chrono::high_resolution_clock> mStartTime;
};

}

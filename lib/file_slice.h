// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QFile>
#include <QIODevice>

namespace ATProto {

class FileSlice : public QIODevice
{
public:
    FileSlice(const QString& name, qint64 start, qint64 length, QObject* parent = nullptr);

    bool open(OpenMode mode) override;
    void close() override;

    qint64 size() const override { return mLength; }
    bool isSequential() const override { return false; }
    qint64 pos() const override { return mPos; }
    bool seek(qint64 pos) override;

protected:
    qint64 readData(char*data, qint64 maxSize) override;
    qint64 writeData(const char*, qint64) override { return -1; }

private:
    QFile mFile;
    qint64 mStart;
    qint64 mLength;
    qint64 mPos = 0;
};

}

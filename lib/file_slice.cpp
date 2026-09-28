// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "file_slice.h"
#include <QDebug>

namespace ATProto {

FileSlice::FileSlice(const QString& name, qint64 start, qint64 length, QObject* parent) :
    QIODevice(parent),
    mFile(name),
    mStart(start),
    mLength(length)
{
    Q_ASSERT(mLength > 0);
}

bool FileSlice::open(OpenMode mode)
{
    Q_ASSERT(mode == QIODevice::ReadOnly);
    qDebug() << "Open file slice:" << mFile.fileName() << "start:" << mStart << "length:" << mLength;

    if (!QIODevice::open(QIODevice::ReadOnly))
        return false;

    if (!mFile.open(QIODevice::ReadOnly))
    {
        setErrorString(mFile.errorString());
        return false;
    }

    if (!mFile.seek(mStart))
    {
        setErrorString(QString("Failed to seek to start %1").arg(mStart));
        return false;
    }

    if (mStart + mLength > mFile.size())
    {
        qint64 newLength = mFile.size() - mStart;
        qDebug() << mFile.fileName() << "reduce length:" << mLength << "to:" << newLength;
        mLength = newLength;
    }

    mPos = 0;
    return true;
}

void FileSlice::close()
{
    mFile.close();
    QIODevice::close();
}

bool FileSlice::seek(qint64 pos)
{
    if (!QIODevice::seek(pos))
        return false;

    if (pos < 0 || pos > mLength)
        return false;

    if (!mFile.seek(mStart + pos))
        return false;

    mPos = pos;
    return true;
}

qint64 FileSlice::readData(char*data, qint64 maxSize)
{
    qint64 remaining = mLength - mPos;

    if (remaining <= 0)
        return -1;

    qint64 toRead = std::min(maxSize, remaining);
    qint64 bytesRead = mFile.read(data, toRead);

    if (bytesRead < 0)
        return -1;

    mPos += bytesRead;
    return bytesRead;
}

}
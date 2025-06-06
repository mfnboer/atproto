// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "xrpc_client.h"

namespace Xrpc {

using namespace std::chrono_literals;

static QNetworkAccessManager* makeNetwork(QObject* parent)
{
    auto* network = new QNetworkAccessManager(parent);
    network->setAutoDeleteReplies(true);
    network->setTransferTimeout(10000);
    return network;
}

Client::Client(const QString& host) :
    mNetwork(makeNetwork(this)),
    mPlcDirectoryClient(mNetwork.get()),
    mIdentityResolver(mNetwork.get()),
    mNetworkThread(new NetworkThread)
{
    qDebug() << "Host:" << host;
    qDebug() << "Device supports OpenSSL:" << QSslSocket::supportsSsl();
    qDebug() << "OpenSSL lib:" << QSslSocket::sslLibraryVersionString();
    qDebug() << "OpenSSL lib build:" << QSslSocket::sslLibraryBuildVersionString();

    if (!host.isEmpty())
        setPDS(host, "");

    if (mNetworkThread->moveToThread(mNetworkThread.get()))
        qDebug() << "Moved network thread";
    else
        qWarning() << "Failed to move network thread";

    qDebug() << "Thread:" << QThread::currentThreadId();

    connect(mNetworkThread.get(), &NetworkThread::requestSuccessJson, this,
        [](QJsonDocument json, NetworkThread::SuccessJsonCb cb) {
            const auto startTime = std::chrono::high_resolution_clock::now();
            cb(std::move(json));
            const auto endTime = std::chrono::high_resolution_clock::now();
            qDebug() << "REPLY JSON DT:" << (endTime - startTime) / 1us;
        });

    connect(mNetworkThread.get(), &NetworkThread::requestSuccessBytes, this,
        [](QByteArray bytes, NetworkThread::SuccessBytesCb cb, QString contentType) {
            const auto startTime = std::chrono::high_resolution_clock::now();
            cb(std::move(bytes), std::move(contentType));
            const auto endTime = std::chrono::high_resolution_clock::now();
            qDebug() << "REPLY BYTES DT:" << (endTime - startTime) / 1us;
        });

    connect(mNetworkThread.get(), &NetworkThread::requestError, this,
        [](QString error, QJsonDocument json, NetworkThread::ErrorCb cb) {
            cb(std::move(error), std::move(json));
        });

    mNetworkThread->start();

    //connect(this, &Client::sendRequestToNetwork, mNetworkThread.get(), &NetworkThread::sendRequest, Qt::QueuedConnection);
    connect(this, &Client::postDataToNetwork, mNetworkThread.get(), &NetworkThread::postData, Qt::QueuedConnection);
    connect(this, &Client::postJsonToNetwork, mNetworkThread.get(), &NetworkThread::postJson, Qt::QueuedConnection);
    connect(this, &Client::getToNetwork, mNetworkThread.get(), &NetworkThread::get, Qt::QueuedConnection);
    connect(this, &Client::pdsChanged, mNetworkThread.get(), &NetworkThread::setPDS, Qt::QueuedConnection);
    connect(this, &Client::userAgentChanged, mNetworkThread.get(), &NetworkThread::setUserAgent, Qt::QueuedConnection);
}

Client::~Client()
{
    qDebug() << "Destroy client";
    mNetworkThread->exit();
    mNetworkThread->wait(1000);
    qDebug() << "XRPC network thread stopped";
}

void Client::setUserAgent(const QString& userAgent)
{
    emit userAgentChanged(userAgent);
}

void Client::setPDS(const QString& pds, const QString& did)
{
    if (pds.startsWith("http"))
        mPDS = pds;
    else
        mPDS = "https://" + pds;

    mDid = did;
    qDebug() << "PDS:" << mPDS << "DID:" << did;
    emit pdsChanged(mPDS);
}

void Client::setPDSFromSession(const ATProto::ComATProtoServer::Session& session)
{
    const auto pds = session.getPDS();

    if (pds)
        setPDS(*pds, session.mDid);
    else
        qDebug() << "No PDS in session, handle:" << session.mHandle << "did:" << session.mDid;
}

void Client::setPDSFromDid(const QString& did, const SetPdsSuccessCb& successCb, const SetPdsErrorCb& errorCb)
{
    qDebug() << "Set PDS from DID:" << did;

    if (!mPDS.isEmpty() && mDid == did)
    {
        qDebug() << "PDS already set:" << mPDS << "DID:" << did;
        QTimer::singleShot(0, this, [successCb]{
            if (successCb)
                successCb();

            return; });
    }

    mPlcDirectoryClient.getPds(did,
        [this, presence=getPresence(), did, successCb](const QString& pds){
            if (!presence)
                return;

            setPDS(pds, did);

            if (successCb)
                successCb();
        },
        [did, errorCb](int, const QString& error){
            qWarning() << "Failed to set PDS:" << did << error;

            if (errorCb)
                errorCb(QString("Could not get PDS: %1").arg(did));
        });
}

void Client::setPDSFromHandle(const QString& handle, const SetPdsSuccessCb& successCb, const SetPdsErrorCb& errorCb)
{
    qDebug() << "Set PDS from handle:" << handle;

    mIdentityResolver.resolveHandle(handle,
        [this, presence=getPresence(), successCb, errorCb](const QString& did){
            if (!presence)
                return;

            setPDSFromDid(did, successCb, errorCb);
        },
        [this, presence=getPresence(), handle, successCb, errorCb](const QString& error){
            if (!presence)
                return;

            qWarning() << "Failed resolve handle:" << handle << "error:" << error;

            if (!mPDS.isEmpty())
            {
                qDebug() << "Initial point of contact:" << mPDS;

                if (successCb)
                    successCb();
            }
            else
            {
                if (errorCb)
                    errorCb(error);
            }
        });
}

void Client::post(const QString& service, const QJsonDocument& json, const NetworkThread::Params& rawHeaders,
                  const NetworkThread::SuccessJsonCb& successCb, const NetworkThread::ErrorCb& errorCb, const QString& accessJwt)
{
    Q_ASSERT(!service.isEmpty());
    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);
    emit postJsonToNetwork(service, json, rawHeaders, successCb, errorCb, accessJwt);
}

void Client::post(const QString& service, const NetworkThread::DataType& data, const QString& mimeType, const NetworkThread::Params& rawHeaders,
                  const NetworkThread::SuccessJsonCb& successCb, const NetworkThread::ErrorCb& errorCb, const QString& accessJwt)
{
    Q_ASSERT(!service.isEmpty());
    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);
    emit postDataToNetwork(service, data, mimeType, rawHeaders, successCb, errorCb, accessJwt);
}

void Client::get(const QString& service, const NetworkThread::Params& params, const NetworkThread::Params& rawHeaders,
                 const NetworkThread::SuccessJsonCb& successCb, const NetworkThread::ErrorCb& errorCb, const QString& accessJwt)
{
    getImpl(service, params, rawHeaders, successCb, errorCb, accessJwt);
}

void Client::get(const QString& service, const NetworkThread::Params& params, const NetworkThread::Params& rawHeaders,
                 const NetworkThread::SuccessBytesCb& successCb, const NetworkThread::ErrorCb& errorCb, const QString& accessJwt)
{
    getImpl(service, params, rawHeaders, successCb, errorCb, accessJwt);
}

template<typename Callback>
void Client::getImpl(const QString& service, const NetworkThread::Params& params, const NetworkThread::Params& rawHeaders,
                     const Callback& successCb, const NetworkThread::ErrorCb& errorCb, const QString& accessJwt)
{
    Q_ASSERT(!service.isEmpty());
    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);

    const auto startTime = std::chrono::high_resolution_clock::now();

    emit getToNetwork(service, params, rawHeaders, successCb, errorCb, accessJwt);

    const auto endTime = std::chrono::high_resolution_clock::now();
    qDebug() << "GET DT:" << (endTime - startTime) / 1us << service;
}

}

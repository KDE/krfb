/*
    Copyright (C) 2009-2011 Collabora Ltd. <info@collabora.co.uk>
      @author George Goldberg <george.goldberg@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "tubesrfbserver.h"
#include "tubesrfbclient.h"
#include "sockethelpers.h"

#include <KDebug>
#include <KRandom>

#include <TelepathyQt/Debug>
#include <TelepathyQt/Contact>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/ConnectionFactory>
#include <TelepathyQt/ContactFactory>
#include <TelepathyQt/ChannelFactory>
#include <TelepathyQt/OutgoingStreamTubeChannel>
#include <TelepathyQt/StreamTubeServer>

#ifdef KRFB_WITH_KDE_TELEPATHY
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingReady>
#include <KTp/Models/contacts-list-model.h>
#include <KTp/contact-factory.h>
#endif

struct TubesRfbServer::Private
{
    Tp::StreamTubeServerPtr stubeServer;
    QHash<quint16, Tp::ContactPtr> contactsPerPort;
    QHash<quint16, PendingTubesRfbClient*> clientsPerPort;
};

//static
TubesRfbServer *TubesRfbServer::instance;

//static
void TubesRfbServer::init()
{
    instance = new TubesRfbServer;
    //RfbServerManager takes care of deletion

    instance->startAndCheck();
}

TubesRfbServer::TubesRfbServer(QObject *parent)
    : RfbServer(parent), d(new Private)
{
    kDebug() << "starting";

    Tp::enableDebug(true);
    Tp::enableWarnings(true);
    Tp::registerTypes();

    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(
            QDBusConnection::sessionBus(),
            Tp::Features() << Tp::Account::FeatureCore
            << Tp::Account::FeatureAvatar
            << Tp::Account::FeatureCapabilities
            << Tp::Account::FeatureProtocolInfo
            << Tp::Account::FeatureProfile);

    Tp::ConnectionFactoryPtr connectionFactory = Tp::ConnectionFactory::create(
            QDBusConnection::sessionBus(),
            Tp::Features() << Tp::Connection::FeatureCore
            << Tp::Connection::FeatureSelfContact);

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(
            QDBusConnection::sessionBus());

#ifdef KRFB_WITH_KDE_TELEPATHY
    Tp::ContactFactoryPtr contactFactory = KTp::ContactFactory::create(
            Tp::Features() << Tp::Contact::FeatureAlias
            <<Tp::Contact::FeatureAvatarToken
            <<Tp::Contact::FeatureAvatarData
            <<Tp::Contact::FeatureSimplePresence
            <<Tp::Contact::FeatureCapabilities
            <<Tp::Contact::FeatureClientTypes);

    m_accountManager = Tp::AccountManager::create(
            QDBusConnection::sessionBus(),
            accountFactory,
            connectionFactory,
            channelFactory,
            contactFactory);

    d->stubeServer = Tp::StreamTubeServer::create(
            m_accountManager,
            QStringList() << QLatin1String("rfb"),
            QStringList(),
            QLatin1String("krfb_rfb_handler"),
            true);

    connect(m_accountManager->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            this,
            SLOT(onAccountManagerReady()));

    m_contactsListModel = new KTp::ContactsListModel(this);
#else
    Tp::ContactFactoryPtr contactFactory = Tp::ContactFactory::create(
            Tp::Contact::FeatureAlias);

    d->stubeServer = Tp::StreamTubeServer::create(
            QStringList() << QLatin1String("rfb"),
            QStringList(),
            QLatin1String("krfb_rfb_handler"),
            true,
            accountFactory,
            connectionFactory,
            channelFactory,
            contactFactory);
#endif  //KRFB_WITH_KDE_TELEPATHY

    connect(d->stubeServer.data(),
            SIGNAL(tubeRequested(Tp::AccountPtr,Tp::OutgoingStreamTubeChannelPtr,
                                 QDateTime,Tp::ChannelRequestHints)),
            SLOT(onTubeRequested()));
    connect(d->stubeServer.data(),
            SIGNAL(tubeClosed(Tp::AccountPtr,Tp::OutgoingStreamTubeChannelPtr,QString,QString)),
            SLOT(onTubeClosed()));

    connect(d->stubeServer.data(),
            SIGNAL(newTcpConnection(QHostAddress,quint16,Tp::AccountPtr,
                                    Tp::ContactPtr,Tp::OutgoingStreamTubeChannelPtr)),
            SLOT(onNewTcpConnection(QHostAddress,quint16,Tp::AccountPtr,
                                    Tp::ContactPtr,Tp::OutgoingStreamTubeChannelPtr)));
    connect(d->stubeServer.data(),
            SIGNAL(tcpConnectionClosed(QHostAddress,quint16,Tp::AccountPtr,Tp::ContactPtr,
                                       QString,QString,Tp::OutgoingStreamTubeChannelPtr)),
            SLOT(onTcpConnectionClosed(QHostAddress,quint16,Tp::AccountPtr,Tp::ContactPtr,
                                       QString,QString,Tp::OutgoingStreamTubeChannelPtr)));

    // Pick a random port in the private range (49152–65535)
    setListeningPort((KRandom::random() % 16383) + 49152);
    // Listen only on the loopback network interface
    setListeningAddress("127.0.0.1");
    setPasswordRequired(false);
}

TubesRfbServer::~TubesRfbServer()
{
    kDebug();
    stop();
    delete d;
}

#ifdef KRFB_WITH_KDE_TELEPATHY
KTp::ContactsListModel *TubesRfbServer::contactsListModel()
{
    return m_contactsListModel;
}
#endif

void TubesRfbServer::startAndCheck()
{
    if (!start()) {
        //try a few times with different ports
        bool ok = false;
        for (int i=0; !ok && i<5; i++) {
            setListeningPort((KRandom::random() % 16383) + 49152);
            ok = start();
        }

        if (!ok) {
            kError() << "Failed to start tubes rfb server";
            return;
        }
    }

    //TODO listeningAddress() should be a QHostAddress
    d->stubeServer->exportTcpSocket(QHostAddress(QString::fromAscii(listeningAddress())),
                                    listeningPort());
}

void TubesRfbServer::onTubeRequested()
{
    kDebug() << "Got a tube";
}

void TubesRfbServer::onTubeClosed()
{
    kDebug() << "tube closed";
}

void TubesRfbServer::onNewTcpConnection(const QHostAddress & sourceAddress,
                                        quint16 sourcePort,
                                        const Tp::AccountPtr & account,
                                        const Tp::ContactPtr & contact,
                                        const Tp::OutgoingStreamTubeChannelPtr & tube)
{
    Q_UNUSED(account);
    Q_UNUSED(tube);

    kDebug() << "CM signaled tube connection from" << sourceAddress << ":" << sourcePort;

    d->contactsPerPort[sourcePort] = contact;
    if (d->clientsPerPort.contains(sourcePort)) {
        kDebug() << "client already exists";
        d->clientsPerPort[sourcePort]->setContact(contact);
    }
}

void TubesRfbServer::onTcpConnectionClosed(const QHostAddress& sourceAddress,
                                           quint16 sourcePort,
                                           const Tp::AccountPtr& account,
                                           const Tp::ContactPtr& contact,
                                           const QString& error,
                                           const QString& message,
                                           const Tp::OutgoingStreamTubeChannelPtr& tube)
{
    Q_UNUSED(account);
    Q_UNUSED(contact);
    Q_UNUSED(tube);

    kDebug() << "Connection from" << sourceAddress << ":" << sourcePort << "closed."
             << error << message;

    d->clientsPerPort.remove(sourcePort);
    d->contactsPerPort.remove(sourcePort);
}

PendingRfbClient* TubesRfbServer::newClient(rfbClientPtr client)
{
    PendingTubesRfbClient *c = new PendingTubesRfbClient(client, this);
    quint16 port = peerPort(client->sock);

    kDebug() << "new tube client on port" << port;

    d->clientsPerPort[port] = c;
    if (d->contactsPerPort.contains(port)) {
        kDebug() << "already have a contact";
        c->setContact(d->contactsPerPort[port]);
    }

    return c;
}

#ifdef KRFB_WITH_KDE_TELEPATHY
void TubesRfbServer::onAccountManagerReady()
{
    m_contactsListModel->setAccountManager(m_accountManager);
}
#endif

#include "tubesrfbserver.moc"

/*
    Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
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
#include "sockethelpers.h"

#include <QtGui/QApplication>
#include <KDebug>
#include <KMessageBox>
#include <KLocale>

#include <TelepathyQt4/Connection>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>

/* workaround for QtDBus bug */
struct StreamTubeAddress
{
    QString address;
    uint port;
};
Q_DECLARE_METATYPE(StreamTubeAddress);

//Marshall the StreamTubeAddress data into a D-Bus argument
QDBusArgument &operator<<(QDBusArgument &argument,
        const StreamTubeAddress &streamTubeAddress)
{
    argument.beginStructure();
    argument << streamTubeAddress.address << streamTubeAddress.port;
    argument.endStructure();
    return argument;
}

// Retrieve the StreamTubeAddress data from the D-Bus argument
const QDBusArgument &operator>>(const QDBusArgument &argument,
        StreamTubeAddress &streamTubeAddress)
{
    argument.beginStructure();
    argument >> streamTubeAddress.address >> streamTubeAddress.port;
    argument.endStructure();
    return argument;
}

//**************

class TubesRfbClient : public RfbClient
{
public:
    TubesRfbClient(rfbClientPtr client, QObject* parent = 0);

    virtual QString name();
    void setContact(const Tp::ContactPtr & contact);

protected:
    virtual rfbNewClientAction doHandle();

private:
    Tp::ContactPtr m_contact;
    bool m_doHandleCalled;
};

TubesRfbClient::TubesRfbClient(rfbClientPtr client, QObject* parent)
    : RfbClient(client, parent), m_doHandleCalled(false)
{
}

QString TubesRfbClient::name()
{
    if (m_contact) {
        return m_contact->alias();
    } else {
        return RfbClient::name();
    }
}

void TubesRfbClient::setContact(const Tp::ContactPtr & contact)
{
    m_contact = contact;
    if (m_doHandleCalled) {
        //doHandle has already been called, so we need to call the parent's implementation here
        rfbNewClientAction action = RfbClient::doHandle();

        //we were previously on hold, so we now need to act if
        //the parent's doHandle() says we must do something else
        if (action == RFB_CLIENT_ACCEPT) {
            rfbStartOnHoldClient(rfbClient());
        } else if (action == RFB_CLIENT_REFUSE) {
            rfbRefuseOnHoldClient(rfbClient());
        } //else if action == RFB_CLIENT_ON_HOLD there is nothing to do
    }
}

rfbNewClientAction TubesRfbClient::doHandle()
{
    if (!m_contact) {
        //no associated contact yet, hold.
        m_doHandleCalled = true; //act when a contact is set
        return RFB_CLIENT_ON_HOLD;
    } else {
        //we have a contact, begin handling
        return RfbClient::doHandle();
    }
}

//**************

struct TubesRfbServer::Private
{
    Tp::ChannelPtr channel;
    QHash<int, Tp::ContactPtr> contactsPerPort;
    QHash<int, TubesRfbClient*> clientsPerPort;
};

TubesRfbServer::TubesRfbServer(const Tp::ChannelPtr & channel, QObject *parent)
    : RfbServer(parent), d(new Private)
{
    kDebug() << "starting ";

    /* Registering struct containing the tube address */
    qDBusRegisterMetaType<StreamTubeAddress>();

    d->channel = channel;
    connect(d->channel->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation *)),
            SLOT(onChannelReady(Tp::PendingOperation *)));

    setListeningPort(6789);
    setListeningAddress("127.0.0.1");  // Listen only on the loopback network interface
    setPasswordRequired(false);
}

TubesRfbServer::~TubesRfbServer()
{
    kDebug();
    delete d;
}

RfbClient* TubesRfbServer::newClient(rfbClientPtr client)
{
    kDebug() << "new tubes client";

    TubesRfbClient *c = new TubesRfbClient(client, this);
    int port = peerPort(client->sock);

    d->clientsPerPort[port] = c;
    if (d->contactsPerPort.contains(port)) {
        kDebug() << "already have a contact";
        c->setContact(d->contactsPerPort[port]);
    }

    return c;
}

/************************** TP TUBES CODE ************************************/

void TubesRfbServer::close()
{
    kDebug();
    d->channel->requestClose();
}

void TubesRfbServer::cleanup()
{
    kDebug();

    d->clientsPerPort.clear();
    d->contactsPerPort.clear();

    stop();
    deleteLater();
}

void TubesRfbServer::onChannelReady(Tp::PendingOperation *op)
{
    kDebug();

    if (op->isError()) {
        kWarning() << "Getting channel ready faied:" << op->errorName() << op->errorMessage();
        KMessageBox::error(QApplication::activeWindow(),
                           i18n("An error occurred sharing your desktop."),
                           i18n("Error"));
        cleanup();
        return;
    }

    Tp::Contacts contacts = d->channel->groupContacts();

    Tp::ContactManager *contactManager = d->channel->connection()->contactManager();

    if (!contactManager) {
        kWarning() << "Invalid Contact Manager.";
        KMessageBox::error(QApplication::activeWindow(),
                           i18n("An unknown error occurred sharing your desktop."),
                           i18n("Error"));
        close();
        return;
    }

    QSet<Tp::Contact::Feature> features;
    features << Tp::Contact::FeatureAlias;

    connect(contactManager->upgradeContacts(contacts.toList(), features),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onContactsUpgraded(Tp::PendingOperation*)));
}

void TubesRfbServer::onContactsUpgraded(Tp::PendingOperation *op)
{
    kDebug();

    if (op->isError()) {
        kWarning() << "Upgrading contacts failed:" << op->errorName() << op->errorMessage();
        KMessageBox::error(QApplication::activeWindow(),
                           i18n("An unknown error occurred sharing your desktop."),
                           i18n("Error"));
        close();
        return;
    }

    offerTube();
}

void TubesRfbServer::offerTube()
{
    kDebug() << "Channel is ready!";

    //start the rfb server
    if (!start()) {
        kWarning() << "Could not start rfb server";
        KMessageBox::error(QApplication::activeWindow(),
                           i18n("Failed to activate the rfb server."),
                           i18n("Error"));
        close();
        return;
    }

    connect(d->channel.data(),
            SIGNAL(invalidated(Tp::DBusProxy*, const QString&, const QString&)),
            SLOT(onChannelInvalidated(Tp::DBusProxy*, const QString&,
                 const QString&)));

    /* Interface used to control the tube state */
    Tp::Client::ChannelInterfaceTubeInterface *tubeInterface = d->channel->tubeInterface();

    /* Interface used to control stream tube */
    Tp::Client::ChannelTypeStreamTubeInterface *streamTubeInterface = d->channel->streamTubeInterface();

    if (streamTubeInterface && tubeInterface) {
        kDebug() << "Offering tube";

        connect(tubeInterface,
                SIGNAL(TubeChannelStateChanged(uint)),
                SLOT(onTubeStateChanged(uint)));

        // Offer the stream tube
        StreamTubeAddress streamTubeAddress;
        streamTubeAddress.address = listeningAddress();
        streamTubeAddress.port = listeningPort();

        kDebug() << "Offering:" << streamTubeAddress.port << streamTubeAddress.address;

        QDBusVariant address;
        address.setVariant(qVariantFromValue(streamTubeAddress));

        QDBusPendingReply<> ret = streamTubeInterface->Offer(
                uint(Tp::SocketAddressTypeIPv4),
                address,
                uint(Tp::SocketAccessControlPort),
                QVariantMap());

        connect(new QDBusPendingCallWatcher(ret, this), SIGNAL(finished(QDBusPendingCallWatcher*)),
                SLOT(onOfferTubeFinished(QDBusPendingCallWatcher*)));
        connect(streamTubeInterface,
                SIGNAL(NewRemoteConnection(uint, QDBusVariant, uint)),
                SLOT(onNewRemoteConnection(uint, QDBusVariant, uint)));
    }
}

void TubesRfbServer::onOfferTubeFinished(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<void> reply = *watcher;
    if (reply.isError()) {
        kWarning() << "Offer tube failed:" << reply.error();

        if (reply.error().name() == TELEPATHY_ERROR_NOT_AVAILABLE) {
            KMessageBox::error(QApplication::activeWindow(),
                               i18n("An error occurred sharing your desktop. The person you are "
                                    "trying to share your desktop with does not have the required "
                                    "software installed to access it."),
                               i18n("Error"));
        } else {
            KMessageBox::error(QApplication::activeWindow(),
                               i18n("An unknown error occurred sharing your desktop."),
                               i18n("Error"));
        }
     } else {
         kDebug() << "Offer Tube succeeded.";
     }
}

void TubesRfbServer::onTubeStateChanged(uint state)
{
    kDebug() << "Tube state changed:" << state;
}

void TubesRfbServer::onNewRemoteConnection(uint handle, QDBusVariant connectionParam, uint connectionId)
{
    Q_UNUSED(connectionId);

    QVariant v = connectionParam.variant();
    kDebug() << "variant:" << v;
    StreamTubeAddress ipv4address = qdbus_cast<StreamTubeAddress>(v);

    kDebug() << "NewRemoteConnection: port:" << ipv4address.port << ipv4address.address;

    Q_FOREACH(const Tp::ContactPtr & c, d->channel->groupContacts()) {
        if (c->handle().value(0) == handle) {
            d->contactsPerPort[ipv4address.port] = c;
            if (d->clientsPerPort.contains(ipv4address.port)) {
                kDebug() << "client already exists";
                d->clientsPerPort[ipv4address.port]->setContact(c);
            }
            break;
        }
    }
}

void TubesRfbServer::onChannelInvalidated(Tp::DBusProxy *proxy,
        const QString &errorName, const QString &errorMessage)
{
    Q_UNUSED(proxy);
    kDebug() << "Tube channel invalidated:" << errorName << errorMessage;
    cleanup();
}

#include "tubesrfbserver.moc"

/*
    Copyright (C) 2009-2010 Collabora Ltd <info@collabora.co.uk>
      @author George Goldberg <george.goldberg@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
    Copyright (C) 2001-2003 by Tim Jansen <tim@tjansen.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "invitationsrfbserver.h"
#include "invitationsrfbclient.h"
#include "krfbconfig.h"
#include "rfbservermanager.h"
#include <QTimer>
#include <QApplication>
#include <QHostInfo>
#include <QDebug>

#include <KLocalizedString>
#include <KUser>
#include <KRandom>
#include <KStringHandler>
#include <KWallet/KWallet>

#include <dnssd/publicservice.h>
using KWallet::Wallet;

//static
InvitationsRfbServer *InvitationsRfbServer::instance;

//static
void InvitationsRfbServer::init()
{
    instance = new InvitationsRfbServer;
    instance->m_publicService = new KDNSSD::PublicService(
            i18n("%1@%2 (shared desktop)",
                KUser().loginName(),
                QHostInfo::localHostName()),
            "_rfb._tcp",
            KrfbConfig::port());
    instance->setListeningAddress("0.0.0.0");
    instance->setListeningPort(KrfbConfig::port());
    instance->setPasswordRequired(true);

    instance->m_wallet = nullptr;
    if (KrfbConfig::noWallet()) {
        instance->walletOpened(false);
    } else {
        instance->openKWallet();
    }
}

const QString& InvitationsRfbServer::desktopPassword() const
{
    return m_desktopPassword;
}

void InvitationsRfbServer::setDesktopPassword(const QString& password)
{
    m_desktopPassword = password;
}

const QString& InvitationsRfbServer::unattendedPassword() const
{
    return m_unattendedPassword;
}

void InvitationsRfbServer::setUnattendedPassword(const QString& password)
{
    m_unattendedPassword = password;
}

bool InvitationsRfbServer::allowUnattendedAccess() const
{
    return m_allowUnattendedAccess;
}

bool InvitationsRfbServer::start()
{
    if(RfbServer::start()) {
        if(KrfbConfig::publishService())
            m_publicService->publishAsync();
        return true;
    }
    return false;
}

void InvitationsRfbServer::stop()
{
    if(m_publicService->isPublished())
        m_publicService->stop();
    RfbServer::stop();
}

void InvitationsRfbServer::toggleUnattendedAccess(bool allow)
{
    m_allowUnattendedAccess = allow;
}

InvitationsRfbServer::InvitationsRfbServer()
{
    m_desktopPassword = readableRandomString(4)+'-'+readableRandomString(3);
    m_unattendedPassword = readableRandomString(4)+'-'+readableRandomString(3);
    KConfigGroup krfbConfig(KSharedConfig::openConfig(),"Security");
    m_allowUnattendedAccess = krfbConfig.readEntry(
            "allowUnattendedAccess", QVariant(false)).toBool();
}

InvitationsRfbServer::~InvitationsRfbServer()
{
    stop();

    KConfigGroup krfbConfig(KSharedConfig::openConfig(), "Security");
    krfbConfig.writeEntry("allowUnattendedAccess", m_allowUnattendedAccess);

    if (!KrfbConfig::noWallet() && m_wallet) {
        closeKWallet();
    } else {
        krfbConfig.writeEntry("desktopPassword",
                KStringHandler::obscure(m_desktopPassword));
        krfbConfig.writeEntry("unattendedPassword",
                KStringHandler::obscure(m_unattendedPassword));
        krfbConfig.writeEntry("allowUnattendedAccess",
                m_allowUnattendedAccess);
    }
}

PendingRfbClient* InvitationsRfbServer::newClient(rfbClientPtr client)
{
    return new PendingInvitationsRfbClient(client, this);
}

void InvitationsRfbServer::openKWallet()
{
    m_wallet = Wallet::openWallet(Wallet::NetworkWallet(), 0, Wallet::Asynchronous);
    if (m_wallet) {
        connect(instance->m_wallet, &KWallet::Wallet::walletOpened,
                this, &InvitationsRfbServer::walletOpened);
    }
}

void InvitationsRfbServer::closeKWallet()
{
    if (m_wallet && m_wallet->isOpen()) {
        if ((m_wallet->currentFolder() == "krfb") ||
                ((m_wallet->hasFolder("krfb") || m_wallet->createFolder("krfb")) &&
                m_wallet->setFolder("krfb")) ) {
            m_wallet->writePassword("desktopSharingPassword", m_desktopPassword);
            m_wallet->writePassword("unattendedAccessPassword", m_unattendedPassword);
        }
        delete m_wallet; // closes the wallet
        m_wallet = nullptr;
    }
}

void InvitationsRfbServer::walletOpened(bool opened)
{
    QString desktopPassword;
    QString unattendedPassword;
    Q_ASSERT(m_wallet);
    if( opened &&
            ( m_wallet->hasFolder("krfb") || m_wallet->createFolder("krfb") ) &&
            m_wallet->setFolder("krfb") ) {

        if(m_wallet->readPassword("desktopSharingPassword", desktopPassword)==0 &&
                !desktopPassword.isEmpty()) {
            m_desktopPassword = desktopPassword;
            emit passwordChanged(m_desktopPassword);
        }

        if(m_wallet->readPassword("unattendedAccessPassword", unattendedPassword)==0 &&
                !unattendedPassword.isEmpty()) {
            m_unattendedPassword = unattendedPassword;
        }

    } else {

        qDebug() << "Could not open KWallet, Falling back to config file";
        KConfigGroup krfbConfig(KSharedConfig::openConfig(),"Security");

        desktopPassword = KStringHandler::obscure(krfbConfig.readEntry(
                "desktopPassword", QString()));
        if(!desktopPassword.isEmpty()) {
            m_desktopPassword = desktopPassword;
            emit passwordChanged(m_desktopPassword);
        }

        unattendedPassword = KStringHandler::obscure(krfbConfig.readEntry(
                "unattendedPassword", QString()));
        if(!unattendedPassword.isEmpty()) {
            m_unattendedPassword = unattendedPassword;
        }

    }
}

// a random string that doesn't contain i, I, o, O, 1, l, 0
// based on KRandom::randomString()
QString InvitationsRfbServer::readableRandomString(int length)
{
    QString str;
    while (length) {
        int r = KRandom::random() % 62;
        r += 48;
        if (r > 57) {
            r += 7;
        }
        if (r > 90) {
            r += 6;
        }
        char c = char(r);
        if ((c == 'i') ||
                (c == 'I') ||
                (c == '1') ||
                (c == 'l') ||
                (c == 'o') ||
                (c == 'O') ||
                (c == '0')) {
            continue;
        }
        str += c;
        length--;
    }
    return str;
}

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
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtNetwork/QHostInfo>
#include <KNotification>
#include <KLocale>
#include <KMessageBox>
#include <KUser>
#include <KRandom>
#include <DNSSD/PublicService>

//static
InvitationsRfbServer *InvitationsRfbServer::instance;

//static
void InvitationsRfbServer::init()
{
    instance = new InvitationsRfbServer;
    instance->m_publicService = new DNSSD::PublicService(
            i18n("%1@%2 (shared desktop)",
                KUser().loginName(),
                QHostInfo::localHostName()),
            "_rfb._tcp",
            KrfbConfig::port());
    instance->setListeningAddress("0.0.0.0");
    instance->setListeningPort(KrfbConfig::port());
    instance->setPasswordRequired(true);
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
        m_publicService->publishAsync();
        return true;
    }
    return false;
}

void InvitationsRfbServer::stop(bool disconnectClients)
{
    if(m_publicService->isPublished())
        m_publicService->stop();
    RfbServer::stop(disconnectClients);
}

void InvitationsRfbServer::toggleUnattendedAccess(bool allow)
{
    m_allowUnattendedAccess = allow;
}

InvitationsRfbServer::InvitationsRfbServer()
{
    m_desktopPassword = readableRandomString(4)+"-"+readableRandomString(3);
    m_unattendedPassword = readableRandomString(4)+"-"+readableRandomString(3);
    m_allowUnattendedAccess = false;
}

PendingRfbClient* InvitationsRfbServer::newClient(rfbClientPtr client)
{
    return new PendingInvitationsRfbClient(client, this);
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

#include "invitationsrfbserver.moc"

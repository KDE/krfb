/*
    Copyright (C) 2010 Collabora Ltd <info@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    Copyright (C) 2001-2002 Tim Jansen <tim@tjansen.de>

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
#include "trayicon.h"

#include "mainwindow.h"
#include "rfbservermanager.h"
#include "rfbclient.h"

#include <QIcon>
#include <QMenu>

#include <KAboutApplicationDialog>
#include <KActionCollection>
#include <QDialog>
#include <KHelpMenu>
#include <KLocalizedString>
#include <KStandardAction>
#include <KToggleAction>
#include <KConfigGroup>

class ClientActions
{
public:
    ClientActions(RfbClient *client, QMenu *menu, QAction *before);
    virtual ~ClientActions();

private:
    QMenu *m_menu = nullptr;
    QAction *m_title = nullptr;
    QAction *m_disconnectAction = nullptr;
    QAction *m_enableControlAction = nullptr;
    QAction *m_separator = nullptr;
};

ClientActions::ClientActions(RfbClient* client, QMenu* menu, QAction* before)
    : m_menu(menu)
{
    m_title = m_menu->insertSection(before, client->name());

    m_disconnectAction = new QAction(i18n("Disconnect"), m_menu);
    m_menu->insertAction(before, m_disconnectAction);

    QObject::connect(m_disconnectAction, &QAction::triggered, client, &RfbClient::closeConnection);

    if (client->controlCanBeEnabled()) {
        m_enableControlAction = new KToggleAction(i18n("Enable Remote Control"), m_menu);
        m_enableControlAction->setChecked(client->controlEnabled());
        m_menu->insertAction(before, m_enableControlAction);

        QObject::connect(m_enableControlAction, &KToggleAction::triggered,
                         client, &RfbClient::setControlEnabled);
        QObject::connect(client, &RfbClient::controlEnabledChanged,
                         m_enableControlAction, &KToggleAction::setChecked);
    } else {
        m_enableControlAction = nullptr;
    }

    m_separator = m_menu->insertSeparator(before);
}

ClientActions::~ClientActions()
{
    m_menu->removeAction(m_title);
    delete m_title;

    m_menu->removeAction(m_disconnectAction);
    delete m_disconnectAction;

    if (m_enableControlAction) {
        m_menu->removeAction(m_enableControlAction);
        delete m_enableControlAction;
    }

    m_menu->removeAction(m_separator);
    delete m_separator;
}

//**********

TrayIcon::TrayIcon(QWidget *mainWindow)
    : KStatusNotifierItem(mainWindow)
{
    setIconByPixmap(QIcon::fromTheme(QStringLiteral("krfb")).pixmap(22, 22, QIcon::Disabled));

    setToolTipTitle(i18n("Desktop Sharing - disconnected"));
    setCategory(KStatusNotifierItem::ApplicationStatus);

    connect(RfbServerManager::instance(), &RfbServerManager::clientConnected,
            this, &TrayIcon::onClientConnected);
    connect(RfbServerManager::instance(), &RfbServerManager::clientDisconnected,
            this, &TrayIcon::onClientDisconnected);

    m_aboutAction = KStandardAction::aboutApp(this, SLOT(showAbout()), this);
    contextMenu()->addAction(m_aboutAction);
}

void TrayIcon::onClientConnected(RfbClient* client)
{
    if (m_clientActions.isEmpty()) { //first client connected
        setIconByName(QStringLiteral("krfb"));
        setToolTipTitle(i18n("Desktop Sharing - connected with %1", client->name()));
        setStatus(KStatusNotifierItem::Active);
    } else { //Nth client connected, N != 1
        setToolTipTitle(i18n("Desktop Sharing - connected"));
    }

    m_clientActions[client] = new ClientActions(client, contextMenu(), m_aboutAction);
}

void TrayIcon::onClientDisconnected(RfbClient* client)
{
    ClientActions *actions = m_clientActions.take(client);
    delete actions;

    if (m_clientActions.isEmpty()) {
        setIconByPixmap(QIcon::fromTheme(QStringLiteral("krfb")).pixmap(22, 22, QIcon::Disabled));
        setToolTipTitle(i18n("Desktop Sharing - disconnected"));
        setStatus(KStatusNotifierItem::Passive);
    } else if (m_clientActions.size() == 1) { //clients number dropped back to 1
        RfbClient *client = m_clientActions.constBegin().key();
        setToolTipTitle(i18n("Desktop Sharing - connected with %1", client->name()));
    }
}

void TrayIcon::showAbout()
{
    KHelpMenu menu;
    menu.aboutApplication();
}

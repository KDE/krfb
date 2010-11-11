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

#include "invitedialog.h"
#include "manageinvitationsdialog.h"
#include "rfbservermanager.h"
#include "rfbclient.h"

#include <KAboutApplicationDialog>
#include <KActionCollection>
#include <KDialog>
#include <KGlobal>
#include <KLocale>
#include <KMenu>
#include <KStandardAction>
#include <KDebug>


class ClientActions
{
public:
    ClientActions(RfbClient *client, KMenu *menu, QAction *before);
    virtual ~ClientActions();

private:
    KMenu *m_menu;
    QAction *m_title;
    QAction *m_disconnectAction;
    QAction *m_enableControlAction;
    QAction *m_separator;
};

ClientActions::ClientActions(RfbClient* client, KMenu* menu, QAction* before)
    : m_menu(menu)
{
    m_title = m_menu->addTitle(client->name(), before);

    m_disconnectAction = new KAction(i18n("Disconnect"), m_menu);
    m_menu->insertAction(before, m_disconnectAction);

    QObject::connect(m_disconnectAction, SIGNAL(triggered()), client, SLOT(closeConnection()));

    if (client->controlCanBeEnabled()) {
        m_enableControlAction = new KToggleAction(i18n("Enable Remote Control"), m_menu);
        m_enableControlAction->setChecked(client->controlEnabled());
        m_menu->insertAction(before, m_enableControlAction);

        QObject::connect(m_enableControlAction, SIGNAL(triggered(bool)),
                         client, SLOT(setControlEnabled(bool)));
        QObject::connect(client, SIGNAL(controlEnabledChanged(bool)),
                         m_enableControlAction, SLOT(setChecked(bool)));
    } else {
        m_enableControlAction = NULL;
    }

    m_separator = m_menu->insertSeparator(before);
}

ClientActions::~ClientActions()
{
    kDebug();

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
    setIconByPixmap(KIcon("krfb").pixmap(22, 22, KIcon::Disabled));

    setToolTipTitle(i18n("Desktop Sharing - disconnected"));
    setCategory(KStatusNotifierItem::ApplicationStatus);

    connect(RfbServerManager::instance(), SIGNAL(clientConnected(RfbClient*)),
            this, SLOT(onClientConnected(RfbClient*)));
    connect(RfbServerManager::instance(), SIGNAL(clientDisconnected(RfbClient*)),
            this, SLOT(onClientDisconnected(RfbClient*)));

    m_aboutAction = KStandardAction::aboutApp(this, SLOT(showAbout()), actionCollection());
    contextMenu()->addAction(m_aboutAction);
}

void TrayIcon::onClientConnected(RfbClient* client)
{
    kDebug();

    if (m_clientActions.isEmpty()) { //first client connected
        setIconByName("krfb");
        setToolTipTitle(i18n("Desktop Sharing - connected with %1", client->name()));
        setStatus(KStatusNotifierItem::Active);
    } else { //Nth client connected, N != 1
        setToolTipTitle(i18n("Desktop Sharing - connected"));
    }

    m_clientActions[client] = new ClientActions(client, contextMenu(), m_aboutAction);
}

void TrayIcon::onClientDisconnected(RfbClient* client)
{
    kDebug();

    ClientActions *actions = m_clientActions.take(client);
    delete actions;

    if (m_clientActions.isEmpty()) {
        setIconByPixmap(KIcon("krfb").pixmap(22, 22, KIcon::Disabled));
        setToolTipTitle(i18n("Desktop Sharing - disconnected"));
        setStatus(KStatusNotifierItem::Passive);
    } else if (m_clientActions.size() == 1) { //clients number dropped back to 1
        RfbClient *client = m_clientActions.constBegin().key();
        setToolTipTitle(i18n("Desktop Sharing - connected with %1", client->name()));
    }
}

void TrayIcon::showAbout()
{
    KDialog *dlg = new KAboutApplicationDialog(KGlobal::mainComponent().aboutData());
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    dlg->show();
}

#include "trayicon.moc"

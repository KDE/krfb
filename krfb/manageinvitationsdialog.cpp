/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "manageinvitationsdialog.h"

#include "invitationmanager.h"
#include "invitation.h"
#include "krfbconfig.h"
#include "personalinvitedialog.h"
#include "ui_configtcp.h"
#include "ui_configsecurity.h"

#include <KConfigDialog>
#include <KGlobal>
#include <KIconLoader>
#include <KLocale>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <KSystemTimeZone>
#include <KToolInvocation>
#include <KStandardAction>
#include <KActionCollection>

#include <QtGui/QWidget>
#include <QtGui/QToolTip>
#include <QtGui/QCursor>
#include <QtCore/QDateTime>
#include <QtNetwork/QNetworkInterface>

class TCP: public QWidget, public Ui::TCP
{
public:
    TCP(QWidget *parent = 0) : QWidget(parent) {
        setupUi(this);
    }
};

class Security: public QWidget, public Ui::Security
{
public:
    Security(QWidget *parent = 0) : QWidget(parent) {
        setupUi(this);
    }
};


ManageInvitationsDialog::ManageInvitationsDialog(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, false);

    QWidget *mainWidget = new QWidget;
    m_ui.setupUi(mainWidget);
    m_ui.pixmapLabel->setPixmap(KIcon("krfb").pixmap(128));
    setCentralWidget(mainWidget);

    connect(m_ui.helpLabel, SIGNAL(linkActivated(QString)), SLOT(showWhatsthis()));
    connect(m_ui.newPersonalInvitationButton, SIGNAL(clicked()), SLOT(inviteManually()));
    connect(m_ui.newEmailInvitationButton, SIGNAL(clicked()), SLOT(inviteByMail()));
    connect(InvitationManager::self(), SIGNAL(invitationNumChanged(int)), SLOT(reloadInvitations()));
    connect(m_ui.deleteAllButton, SIGNAL(clicked()), SLOT(deleteAll()));
    connect(m_ui.deleteOneButton, SIGNAL(clicked()), SLOT(deleteCurrent()));
    connect(m_ui.invitationWidget, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()));

    KStandardAction::quit(QCoreApplication::instance(), SLOT(quit()), actionCollection());
    KStandardAction::preferences(this, SLOT(showConfiguration()), actionCollection());

    setupGUI(QSize(550, 330));
    setAutoSaveSettings();

    reloadInvitations();
}

ManageInvitationsDialog::~ManageInvitationsDialog()
{
}

void ManageInvitationsDialog::showWhatsthis()
{
    QToolTip::showText(QCursor::pos(),
                       i18n("An invitation creates a one-time password that allows the receiver to connect to your desktop.\n"
                            "It is valid for only one successful connection and will expire after an hour if it has not been used. \n"
                            "When somebody connects to your computer a dialog will appear and ask you for permission.\n"
                            "The connection will not be established before you accept it. In this dialog you can also\nrestrict "
                            "the other person to view your desktop only, without the ability to move your\nmouse pointer or press "
                            "keys.\nIf you want to create a permanent password for Desktop Sharing, allow 'Uninvited Connections' \n"
                            "in the configuration."));

}

void ManageInvitationsDialog::inviteManually()
{
    Invitation inv = InvitationManager::self()->addInvitation();
    PersonalInviteDialog *pid = new PersonalInviteDialog(this);
    pid->setPassword(inv.password());
    pid->setExpiration(inv.expirationTime());
    pid->show();
}

void ManageInvitationsDialog::inviteByMail()
{
    int r = KMessageBox::warningContinueCancel(this,
            i18n("When sending an invitation by email, note that everybody who reads this email "
                 "will be able to connect to your computer for one hour, or until the first "
                 "successful connection took place, whichever comes first. \n"
                 "You should either encrypt the email or at least send it only in a "
                 "secure network, but not over the Internet."),
            i18n("Send Invitation via Email"),
            KStandardGuiItem::cont(),
            KStandardGuiItem::cancel(),
            "showEmailInvitationWarning");

    if (r == KMessageBox::Cancel) {
        return;
    }

    QList<QNetworkInterface> ifl = QNetworkInterface::allInterfaces();
    QString host;
    int port = KrfbConfig::port();
    foreach(const QNetworkInterface & nif, ifl) {
        if (nif.flags() & QNetworkInterface::IsLoopBack) {
            continue;
        }

        if (nif.flags() & QNetworkInterface::IsRunning) {
            if (!nif.addressEntries().isEmpty()) {
                host = nif.addressEntries()[0].ip().toString();
            }
        }
    }

    Invitation inv = InvitationManager::self()->addInvitation();
    KUrl invUrl(QString("vnc://invitation:%1@%2:%3").arg(inv.password()).arg(host).arg(port));
    KToolInvocation::invokeMailer(QString(), QString(), QString(),
                                  i18n("Desktop Sharing (VNC) invitation"),
                                  ki18n("You have been invited to a VNC session. If you have the KDE Remote "
                                        "Desktop Connection installed, just click on the link below.\n\n"
                                        "%1\n\n"
                                        "Otherwise you can use any VNC client with the following parameters:\n\n"
                                        "Host: %2:%3\n"
                                        "Password: %4\n\n"
                                        "For security reasons this invitation will expire at %5 (%6).")
                                  .subs(invUrl.url())
                                  .subs(host)
                                  .subs(QString::number(port))
                                  .subs(inv.password())
                                  .subs(KGlobal::locale()->formatDateTime(inv.expirationTime()))
                                  .subs(KSystemTimeZones::local().name())
                                  .toString());

}

void ManageInvitationsDialog::reloadInvitations()
{
    m_ui.invitationWidget->clear();
    KLocale *loc = KGlobal::locale();
    foreach(const Invitation & inv, InvitationManager::self()->invitations()) {
        QStringList strs;
        strs <<  loc->formatDateTime(inv.creationTime()) << loc->formatDateTime(inv.expirationTime());
        QTreeWidgetItem *it = new QTreeWidgetItem(strs);
        m_ui.invitationWidget->addTopLevelItem(it);
        it->setData(0, Qt::UserRole + 1, inv.creationTime());
    }
    m_ui.invitationWidget->resizeColumnToContents(0);
    m_ui.deleteAllButton->setEnabled(InvitationManager::self()->activeInvitations() > 0);
}

void ManageInvitationsDialog::showConfiguration()
{
    if (KConfigDialog::showDialog("settings")) {
        return;
    }

    KConfigDialog *dialog = new KConfigDialog(this, "settings", KrfbConfig::self());
    dialog->addPage(new TCP, i18n("Network"), "network-workgroup");
    dialog->addPage(new Security, i18n("Security"), "security-high");
    dialog->setHelp(QString(), "krfb");
    dialog->show();
}

void ManageInvitationsDialog::deleteAll()
{
    if (KMessageBox::warningContinueCancel(this,
                                           i18n("<qt>Are you sure you want to delete all invitations?</qt>"),
                                           i18n("Confirm delete Invitations"),
                                           KStandardGuiItem::ok(),
                                           KStandardGuiItem::cancel(),
                                           QString("krfbdeleteallinv")) !=
            KMessageBox::Continue) {
        return;
    }

    InvitationManager::self()->removeAllInvitations();
}

void ManageInvitationsDialog::deleteCurrent()
{
    if (KMessageBox::warningContinueCancel(this,
                                           i18n("<qt>Are you sure you want to delete this invitation?</qt>"),
                                           i18n("Confirm delete Invitations"),
                                           KStandardGuiItem::ok(),
                                           KStandardGuiItem::cancel(),
                                           QString("krfbdeleteoneinv")) !=
            KMessageBox::Continue) {
        return;
    }

    // disable updates while deleting items, otherwise the list would invalidate itself
    disconnect(InvitationManager::self(), SIGNAL(invitationNumChanged(int)),
               this, SLOT(reloadInvitations()));

    QList<QTreeWidgetItem *> itl = m_ui.invitationWidget->selectedItems();
    foreach(QTreeWidgetItem * itm, itl) {
        foreach(const Invitation & inv, InvitationManager::self()->invitations()) {
            if (inv.creationTime() == itm->data(0, Qt::UserRole + 1)) {
                InvitationManager::self()->removeInvitation(inv);
            }
        }
    }

    // update it manually
    reloadInvitations();

    connect(InvitationManager::self(), SIGNAL(invitationNumChanged(int)),
            SLOT(reloadInvitations()));
}

void ManageInvitationsDialog::selectionChanged()
{
    m_ui.deleteOneButton->setEnabled(m_ui.invitationWidget->selectedItems().size() > 0);
}

void ManageInvitationsDialog::readProperties(const KConfigGroup& group)
{
    if (group.readEntry("Visible", true)) {
        show();
    }
    KMainWindow::readProperties(group);
}

void ManageInvitationsDialog::saveProperties(KConfigGroup& group)
{
    group.writeEntry("Visible", isVisible());
    KMainWindow::saveProperties(group);
}

#include "manageinvitationsdialog.moc"


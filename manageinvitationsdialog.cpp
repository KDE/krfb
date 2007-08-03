/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/
#include "manageinvitationsdialog.h"
#include "manageinvitationsdialog.moc"

#include "personalinvitedialog.h"
#include "invitationmanager.h"
#include "invitation.h"
#include "krfbconfig.h"
#include "krfbserver.h"

#include <QWidget>
#include <QToolTip>
#include <QCursor>
#include <QDateTime>
#include <QNetworkInterface>

#include <KStandardGuiItem>
#include <KIconLoader>
#include <KLocale>
#include <KGlobal>
#include <KConfigDialog>
#include <KMessageBox>
#include <KToolInvocation>

// settings dialog
#include "ui_configtcp.h"
#include "ui_configsecurity.h"

class TCP: public QWidget, public Ui::TCP {
    public:
        TCP(QWidget *parent=0) :QWidget(parent)
        {
            setupUi(this);
        }
};

class Security: public QWidget, public Ui::Security {
    public:
        Security(QWidget *parent=0) :QWidget(parent)
        {
            setupUi(this);
        }
};


ManageInvitationsDialog::ManageInvitationsDialog(QWidget *parent)
 : KDialog(parent)
{
    setCaption(i18n("Invitation"));
    setButtons(User1|Close|Help);
    setDefaultButton(NoDefault);

    setMinimumSize(500, 330);

    setupUi(mainWidget());
    pixmapLabel->setPixmap(KIcon("krfb").pixmap(128));

    setButtonGuiItem( User1, KStandardGuiItem::configure() );

    connect( helpLabel, SIGNAL( linkActivated ( QString ) ),
             SLOT( showWhatsthis() ));
    connect( newPersonalInvitationButton, SIGNAL( clicked() ),
             SLOT( inviteManually() ));
    connect( newEmailInvitationButton, SIGNAL( clicked() ),
             SLOT( inviteByMail() ));
    connect( InvitationManager::self(), SIGNAL( invitationNumChanged( int )),
             SLOT( reloadInvitations() ));
    connect( this, SIGNAL(user1Clicked()),SLOT(showConfiguration()));
    connect( deleteAllButton, SIGNAL( clicked() ),
             SLOT( deleteAll() ));
    connect( deleteOneButton, SIGNAL( clicked() ),
             SLOT( deleteCurrent() ));
    connect( invitationWidget, SIGNAL(itemSelectionChanged ()),
             SLOT( selectionChanged() ));

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
    pid->exec();
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
    if (r == KMessageBox::Cancel)
        return;

    QList<QNetworkInterface> ifl = QNetworkInterface::allInterfaces();
    QString host;
    int port = KrfbConfig::port();
    foreach (QNetworkInterface nif, ifl) {
        if (nif.flags() & QNetworkInterface::IsLoopBack) continue;
        if (nif.flags() & QNetworkInterface::IsRunning) {
            host = nif.addressEntries()[0].ip().toString();
        }
    }

    Invitation inv = InvitationManager::self()->addInvitation();
    KToolInvocation::invokeMailer(QString(), QString(), QString(),
            i18n("Desktop Sharing (VNC) invitation"),
            ki18n("You have been invited to a VNC session. If you have the KDE Remote "
                  "Desktop Connection installed, just click on the link below.\n\n"
                  "vnc://invitation:%1@%2:%3\n\n"
                  "Otherwise you can use any VNC client with the following parameters:\n\n"
                  "Host: %4:%5\n"
                  "Password: %6\n\n"
                  "For security reasons this invitation will expire at %7.")
            .subs(inv.password())
            .subs(host)
            .subs(port)
            .subs(host)
            .subs(port)
            .subs(inv.password())
            .subs(KGlobal::locale()->formatDateTime(inv.expirationTime()))
            .toString());

}

void ManageInvitationsDialog::reloadInvitations()
{
    invitationWidget->clear();
    KLocale *loc = KGlobal::locale();
    foreach(Invitation inv, InvitationManager::self()->invitations()) {
        QStringList strs;
        strs <<  loc->formatDateTime(inv.creationTime()) << loc->formatDateTime(inv.expirationTime());
        QTreeWidgetItem *it = new QTreeWidgetItem(strs);
        invitationWidget->addTopLevelItem(it);
        it->setData(0,Qt::UserRole+1, inv.creationTime());
    }
    invitationWidget->resizeColumnToContents(0);
    deleteAllButton->setEnabled(InvitationManager::self()->activeInvitations() > 0);
}

void ManageInvitationsDialog::showConfiguration()
{
    if(KConfigDialog::showDialog("settings"))
        return;

    KConfigDialog *dialog = new KConfigDialog(this, "settings", KrfbConfig::self());
    dialog->addPage(new TCP, i18n("Network"), "network");
    dialog->addPage(new Security, i18n("Security"), "encrypted");
    connect(dialog, SIGNAL(settingsChanged(QString)),KrfbServer::self(),SLOT(updateSettings()));
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
        KMessageBox::Continue)
    {
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
        KMessageBox::Continue)
    {
        return;
    }
    QList<QTreeWidgetItem *> itl = invitationWidget->selectedItems();
    foreach(QTreeWidgetItem *itm, itl) {
        foreach(Invitation inv, InvitationManager::self()->invitations()) {
            if (inv.creationTime() == itm->data(0,Qt::UserRole+1)) {
                InvitationManager::self()->removeInvitation(inv);
            }
        }
    }

}

void ManageInvitationsDialog::selectionChanged()
{
    deleteOneButton->setEnabled(invitationWidget->selectedItems().size() > 0);
}


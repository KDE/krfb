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

#include <QWidget>
#include <QToolTip>
#include <QCursor>

#include <KStandardDirs>
#include <KStandardGuiItem>
#include <KIconLoader>


ManageInvitationsDialog::ManageInvitationsDialog(QWidget *parent)
 : KDialog(parent)
{
    setCaption(i18n("Invitation"));
    setButtons(User1|Close|Help);
    setDefaultButton(NoDefault);
    setModal(false);

    QWidget *main = new QWidget(this);
    setupUi(main);
    setMainWidget( main );
    pixmapLabel->setPixmap(KStandardDirs::locate("data", "krfb/pics/connection-side-image.png"));

    setButtonGuiItem( User1, KStandardGuiItem::configure() );

    connect( helpLabel, SIGNAL( linkActivated ( QString ) ),
             SLOT( showWhatsthis() ));
    connect( newPersonalInvitationButton, SIGNAL( clicked() ),
             SLOT( inviteManually() ));
    connect( newEmailInvitationButton, SIGNAL( clicked() ),
             SLOT( inviteByMail() ));

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
}


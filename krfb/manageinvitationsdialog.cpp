//
// C++ Implementation: manageinvitationsdialog
//
// Description:
//
//
// Author: Alessandro Praduroux <pradu@pradu.it>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "manageinvitationsdialog.h"
#include "manageinvitationsdialog.moc"

#include <QWidget>
#include <QToolTip>
#include <QCursor>
#include <KStandardGuiItem>
#include <KIconLoader>

ManageInvitationsDialog::ManageInvitationsDialog(QWidget *parent)
 : KDialog(parent)
{
    setCaption(i18n("Invitation"));
    setButtons(User1|Close|Help);
    setDefaultButton(NoDefault);
    setModal(true);

    QWidget *main = new QWidget(this);
    setupUi(main);
    setMainWidget( main );

    pixmapLabel->setPixmap( UserIcon( "connection-side-image.png" ) );

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
}

void ManageInvitationsDialog::inviteByMail()
{
}


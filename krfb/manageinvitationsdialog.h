//
// C++ Interface: manageinvitationsdialog
//
// Description:
//
//
// Author: Alessandro Praduroux <pradu@pradu.it>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MANAGEINVITATIONSDIALOG_H
#define MANAGEINVITATIONSDIALOG_H

#include <KDialog>
#include "ui_rfbmanageinvitations.h"

/**
	@author Alessandro Praduroux <pradu@pradu.it>
*/
class ManageInvitationsDialog : public KDialog, private Ui::ManageInvitationsDialog
{
Q_OBJECT
public:
    ManageInvitationsDialog(QWidget *parent = 0);

    ~ManageInvitationsDialog();

public Q_SLOTS:
    void showWhatsthis();
    void inviteManually();
    void inviteByMail();

private:

};

#endif

/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef MANAGEINVITATIONSDIALOG_H
#define MANAGEINVITATIONSDIALOG_H

#include <KDialog>
#include "ui_manageinvitations.h"

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
    void reloadInvitations();
    void showConfiguration();
    void deleteAll();
    void deleteCurrent();
    void selectionChanged();

private:

};

#endif

/***************************************************************************
                               configuration.h
                             -------------------
    begin                : Tue Dec 11 2001
    copyright            : (C) 2001-2002 by Tim Jansen
    email                : tim@tjansen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "invitation.h"

#include "configurationdialog.h"
#include "manageinvitations.h"
#include "personalinvitation.h"
#include "invite.h"

#include <kconfig.h>
#include <qtimer.h>
#include <qobject.h>
#include <qvalidator.h>
#include <qstring.h>

enum krfb_mode {
	KRFB_UNKNOWN_MODE = 0,
	KRFB_STAND_ALONE,
	KRFB_STAND_ALONE_CMDARG,
	KRFB_KINETD_MODE,
	KRFB_INVITATION_MODE
};

class ConfigurationDialog2 : public ConfigurationDialog {
	Q_OBJECT
public:
	virtual void closeEvent(QCloseEvent *);
signals:
	void closed();
};
class ManageInvitationsDialog2 : public ManageInvitationsDialog {
	Q_OBJECT
public:
	virtual void closeEvent(QCloseEvent *);
signals:
	void closed();
};
class InvitationDialog2 : public InvitationDialog {
	Q_OBJECT
public:
	virtual void closeEvent(QCloseEvent *);
signals:
	void closed();
};
class PersonalInvitationDialog2 : public PersonalInvitationDialog {
	Q_OBJECT
public:
	virtual void closeEvent(QCloseEvent *);
signals:
	void closed();
};

/**
 * This class stores the app's configuration, manages the
 * standalone-config-dialog and all the invitation dialogs
 * @author Tim Jansen
 */
class Configuration : public QObject {
   	Q_OBJECT
public:
	Configuration(krfb_mode mode);
	Configuration(bool oneConnection, bool askOnConnect,
		      bool allowDesktopControl, QString password);
	~Configuration();

	krfb_mode mode() const;
	bool oneConnection() const;
	bool askOnConnect() const;
	bool allowDesktopControl() const;
	bool showInvitationDialogOnStartup() const;
	QString password() const;

        void setOnceConnection(bool oneConnection);
        void setAskOnConnect(bool askOnConnect);
        void setAllowDesktopControl(bool allowDesktopControl);
	void setPassword(QString password);
	void reload();

	QValueList<Invitation> &invitations();
signals:
  	void passwordChanged();

public slots:
	void showConfigDialog();
	void showManageInvitationsDialog();
	void showInvitationDialog();
	void showPersonalInvitationDialog();
	void inviteEmail();

	void invalidateOldInvitations();

private:
        void loadFromKConfig();
        void loadFromDialogs();
        void saveToKConfig();
        void saveToDialogs();
	Invitation createInvitation();

	krfb_mode m_mode;

        ConfigurationDialog2 confDlg;
	ManageInvitationsDialog2 invMngDlg;
	InvitationDialog2 invDlg;
	PersonalInvitationDialog2 persInvDlg;
	QTimer expirationTimer;

	bool askOnConnectFlag;
	bool allowDesktopControlFlag;
	bool allowUninvitedFlag;
	bool oneConnectionFlag;
	
	bool showInvDlgOnStartupFlag;

	QString passwordString;
	QValueList<Invitation> invitationList;
private slots:
	void configOkPressed();
	void configCancelPressed();
	void configApplyPressed();
	void configChanged();

	void invMngDlgClosed();
	void invMngDlgDeleteOnePressed();
	void invMngDlgDeleteAllPressed();
	
	void invDlgClosed();

	void persInvDlgClosed();
};

#endif

/***************************************************************************
                               configuration.h
                             -------------------
    begin                : Tue Dec 11 2001
    copyright            : (C) 2001-2003 by Tim Jansen
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

#include "manageinvitations.h"
#include "personalinvitedialog.h"
#include "invitedialog.h"

#include <dcopref.h>
#include <kconfig.h>
#include <qtimer.h>
#include <qobject.h>
#include <qvalidator.h>
#include <qstring.h>

#include <dcopobject.h>

enum krfb_mode {
	KRFB_UNKNOWN_MODE = 0,
	KRFB_KINETD_MODE,
	KRFB_INVITATION_MODE,
	KRFB_CONFIGURATION_MODE
};


/**
 * This class stores the app's configuration, manages the
 * standalone-config-dialog and all the invitation dialogs
 * @author Tim Jansen
 */
class Configuration : public QObject, public DCOPObject {
		K_DCOP
   	Q_OBJECT
public:
	Configuration(krfb_mode mode);
	virtual ~Configuration();

	krfb_mode mode() const;
	bool askOnConnect() const;
	bool allowDesktopControl() const;
	bool allowUninvitedConnections() const;
	bool enableSLP() const;
	QString password() const;
	QString hostname() const;
	int port() const;
	int preferredPort() const;
	bool disableBackground() const;
	bool disableXShm() const;

        void setAllowUninvited(bool allowUninvited);
	void setEnableSLP(bool e);
        void setAskOnConnect(bool askOnConnect);
	void setPassword(QString password);
	void setPreferredPort(int p);
	void setDisableBackground(bool disable);
	void setDisableXShm(bool disable);
	void save();
	void update();

	QValueList<Invitation> &invitations();
	void removeInvitation(QValueList<Invitation>::iterator it);
signals:
	void invitationFinished();
	void invitationNumChanged(int num);

public slots:
        void setAllowDesktopControl(bool allowDesktopControl);
	void showManageInvitationsDialog();
	void showInvitationDialog();
	void showPersonalInvitationDialog();
	void showConfigurationModule();
	void inviteEmail();

private:
        void loadFromKConfig();
        void loadFromDialogs();
        void saveToKConfig();
        void saveToDialogs();
	Invitation createInvitation();
	void closeInvDlg();
	void invalidateOldInvitations();
	void setKInetdEnabled(const QDateTime &date);
	void setKInetdEnabled(bool enabled);
	void setKInetdServiceRegistrationEnabled(bool enabled);
	void getPortFromKInetd();
	void setKInetdPort(int port);
	void doKinetdConf();

	krfb_mode m_mode;

	ManageInvitationsDialog invMngDlg;
	InviteDialog invDlg;
	PersonalInviteDialog persInvDlg;
	QTimer refreshTimer;

	bool askOnConnectFlag;
	bool allowDesktopControlFlag;
	bool allowUninvitedFlag;
	bool enableSLPFlag;

	int portNum, preferredPortNum;

	DCOPRef kinetdRef;

	QString passwordString;
	QValueList<Invitation> invitationList;

	bool disableBackgroundFlag;
	bool disableXShmFlag;

k_dcop:
    // Connected to the DCOP signal
    void updateKConfig();	
private slots:
        void refreshTimeout();

	void invMngDlgDeleteOnePressed();
	void invMngDlgDeleteAllPressed();
};

#endif

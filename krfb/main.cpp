/***************************************************************************
                                   main.cpp
                             -------------------
    begin                : Sat Dec  8 03:23:02 CET 2001
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

#include "trayicon.h"
#include "configuration.h"
#include "krfbifaceimpl.h"
#include "rfbcontroller.h"

#include <kpixmap.h>
#include <kaction.h>
#include <kdebug.h>
#include <kapplication.h>
#include <knotifyclient.h>
#include <ksystemtray.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kaboutapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qobject.h>
#include <qwindowdefs.h>
#include <qcstring.h>
#include <qdatastream.h>
#include <dcopclient.h>

#include <signal.h>

#define VERSION "0.7"

static const char *description = I18N_NOOP("VNC-compatible server to share "
					   "KDE desktops");
#define ARG_ONE_SESSION "one-session"
#define ARG_PASSWORD "password"
#define ARG_DONT_CONFIRM_CONNECT "dont-confirm-connect"
#define ARG_REMOTE_CONTROL "remote-control"
#define ARG_STAND_ALONE "stand-alone"
#define ARG_KINETD "kinetd"


static KCmdLineOptions options[] =
{
	{ "o", 0, 0},
	{ ARG_ONE_SESSION, I18N_NOOP("Terminate when the first session is finished."), 0},
	{ "w", 0, 0},
	{ ARG_PASSWORD " ", I18N_NOOP("Set the password."), ""},
	{ "d", 0, 0},
	{ ARG_DONT_CONFIRM_CONNECT, I18N_NOOP("Allow connections without asking the user."), 0},
	{ "c", 0, 0},
	{ ARG_REMOTE_CONTROL, I18N_NOOP("Allow remote side to control this computer."), 0},
	{ "s", 0, 0},
	{ ARG_STAND_ALONE, I18N_NOOP("Standalone mode: do not use daemon."), 0},
	{ ARG_KINETD " ", I18N_NOOP("Used for calling from kinetd."), 0},
	{ 0, 0, 0 }
};

/*
 * KRfb can run in 4 different modes:
 * - stand-alone
 *   + To get there call KRfb with kinetd disabled in KConfig (use KControl module)
 *   + traditional mode like <0.7 versions
 *   + uses non-kcm configuration dialog
 *   + invitation on start-up can be disabled
 * - stand-alone with command line args
 *   + to get there call krfb any cmd line args (except --kinetd)
 *   + behaves like stand-alone, but without configuration & invitations
 * - kinetd mode
 *   + started using the --kinetd argument
 *   + used for starting from kinetd
 *   + takes socket fd from cmd args
 *   + exits after connection is finished
 *   + config option calls kcontrol module
 * - invitation mode
 *   + started when krfb is called while kinetd is enabled and no cmd line arg is given
 *   + displays only invitation dialog and finishs then
 *   + does not accept connections, no tray icon
 *
 * TODO:
 * - fix bug on 'close connection' in kinetd mode
 */

void checkKInetd(bool &kinetdAvailable, bool &krfbAvailable) {
	kinetdAvailable = false;
	krfbAvailable = false;

	DCOPClient *d = KApplication::dcopClient();

	QByteArray sdata, rdata;
	QCString replyType;
	QDataStream arg(sdata, IO_WriteOnly);
	arg << QString("krfb");
	if (!d->call ("kded", "kinetd", "isInstalled(QString)", sdata, replyType, rdata))
		return;

	if (replyType != "bool")
		return;

	QDataStream answer(rdata, IO_ReadOnly);
	answer >> krfbAvailable;
	kinetdAvailable = true;
}

int main(int argc, char *argv[])
{
	enum krfb_mode mode = KRFB_UNKNOWN_MODE;

	KAboutData aboutData( "krfb", I18N_NOOP("Desktop Sharing"),
		VERSION, description, KAboutData::License_GPL,
		"(c) 2001-2002, Tim Jansen\n"
		"(c) 2001, Johannes E. Schindelin\n"
		"(c) 2000, heXoNet Support GmbH, D-66424 Homburg\n"
		"(c) 2000, Const Kaplinsky\n"
		"(c) 2000, Tridia Corporation\n"
		"(c) 1999, AT&T Laboratories Cambridge\n",
                0, "http://www.tjansen.de/krfb", "ml@tjansen.de");
	aboutData.addAuthor("Tim Jansen", "", "tim@tjansen.de");
        aboutData.addAuthor("Ian Reinhart Geiser", "DCOP interface", "geiseri@kde.org");
	aboutData.addCredit("Johannes E. Schindelin",
			    I18N_NOOP("libvncserver"));
	aboutData.addCredit("Const Kaplinsky",
			    I18N_NOOP("TightVNC encoder"));
	aboutData.addCredit("Tridia Corporation",
			    I18N_NOOP("ZLib encoder"));
	aboutData.addCredit("AT&T Laboratories Cambridge",
			    I18N_NOOP("original VNC encoders and "
				      "protocol design"));
	aboutData.addCredit("Jens Wagner (heXoNet Support GmbH)",
			    I18N_NOOP("X11 update scanner, "
				      "original code base"));
	aboutData.addCredit("Jason Spisak",
			    I18N_NOOP("Connection side image"),
			    "kovalid@yahoo.com");
	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;

	Configuration *config;
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	QString fdString;
	if (args->isSet(ARG_ONE_SESSION) ||
	    args->isSet(ARG_PASSWORD) ||
	    args->isSet(ARG_REMOTE_CONTROL) ||
	    args->isSet(ARG_DONT_CONFIRM_CONNECT)) {

		bool oneConnection = args->isSet(ARG_ONE_SESSION);
		bool askOnConnect = !args->isSet(ARG_DONT_CONFIRM_CONNECT);
		bool allowDesktopControl = args->isSet(ARG_REMOTE_CONTROL);
		QString password = args->getOption(ARG_PASSWORD);
		mode = KRFB_STAND_ALONE_CMDARG;
		config = new Configuration(oneConnection, askOnConnect,
					   allowDesktopControl, password);
	}
	else {
		if (args->isSet(ARG_STAND_ALONE)) {
			mode = KRFB_STAND_ALONE;
		}
		else if (args->isSet(ARG_KINETD)) {
			fdString = args->getOption(ARG_KINETD);
			mode = KRFB_KINETD_MODE;
		}
		if (mode == KRFB_UNKNOWN_MODE) {
			if (Configuration::earlyDaemonMode()) {
				bool kinetdA, krfbA;
				mode = KRFB_INVITATION_MODE;
				checkKInetd(kinetdA, krfbA);
				if (!kinetdA) {
					KMessageBox::error(0,
						i18n("Cannot find KInetD. "
							"Have you restarted KDE after installation?"),
						i18n("Desktop Sharing Error"));
					return 1;
				}
				if (!krfbA) {
					KMessageBox::error(0,
						i18n("Cannot find KInetD service for Desktop Sharing (KRfb). "
							"Have you restarted KDE after installation?"),
						i18n("Desktop Sharing Error"));
					return 1;
				}
			}
			else
				mode = KRFB_STAND_ALONE;
		}
		config = new Configuration(mode);

		if ((mode == KRFB_KINETD_MODE) &&
		    (!config->allowUninvitedConnections()) &&
		    (config->invitations().size() == 0)) {
			KNotifyClient::event("UnexpectedConnection");
			return 1;
		}
	}
	args->clear();


	if (mode == KRFB_INVITATION_MODE) {
		config->showInvitationDialog();
		QObject::connect(config, SIGNAL(invitationFinished()),
				 &app, SLOT(quit()));
		return app.exec();
	}

	if (!RFBController::checkX11Capabilities())
		return 1;

	TrayIcon trayicon(new KAboutApplication(&aboutData),
			  config);
	KRfbIfaceImpl dcopiface(config);
	RFBController controller(config);

	QObject::connect(&app, SIGNAL(lastWindowClosed()),
			 &controller, SLOT(closeConnection()));

	QObject::connect(&app, SIGNAL(lastWindowClosed()),
			 &app, SLOT(quit()));

	QObject::connect(&trayicon, SIGNAL(connectionClosed()),
			 &controller, SLOT(closeConnection()));
	QObject::connect(&trayicon, SIGNAL(showConfigure()),
			 config, SLOT(showConfigDialog()));
	QObject::connect(&trayicon, SIGNAL(showManageInvitations()),
			 config, SLOT(showManageInvitationsDialog()));
	QObject::connect(&controller, SIGNAL(portProbed(int)),
			 config, SLOT(setPort(int)));

	QObject::connect(&dcopiface, SIGNAL(connectionClosed()),
			 &controller, SLOT(closeConnection()));

	QObject::connect(&dcopiface, SIGNAL(exitApp()),
			 &app, SLOT(quit()));

	QObject::connect(config, SIGNAL(passwordChanged()),
			 &controller, SLOT(passwordChanged()));

	QObject::connect(&controller, SIGNAL(portProbed(int)),
			 &dcopiface, SLOT(setPort(int)));

	QObject::connect(&controller, SIGNAL(sessionEstablished()),
			 &trayicon, SLOT(openConnection()));

	if (config->oneConnection() || (mode == KRFB_KINETD_MODE)) {
		QObject::connect(&controller, SIGNAL(sessionRefused()),
				 &app, SLOT(quit()));
		QObject::connect(&controller, SIGNAL(sessionFinished()),
				 &app, SLOT(quit()));
	} else {
		QObject::connect(&controller, SIGNAL(sessionFinished()),
				 &trayicon, SLOT(closeConnection()));
	}

	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs, SIGPIPE);
	sigprocmask(SIG_BLOCK, &sigs, 0);

	if (mode == KRFB_KINETD_MODE) {
		bool ok;
		int fdNum = fdString.toInt(&ok);
		if (!ok) {
			kdError() << "kinetd fd was not numeric." << endl;
			return 2;
		}
		controller.startServer(fdNum);
	}
	else
		controller.startServer();

	if (config->showInvitationDialogOnStartup())
		config->showInvitationDialog();

	return app.exec();
}


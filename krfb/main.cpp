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

#define VERSION "1.0"

static const char *description = I18N_NOOP("VNC-compatible server to share "
					   "KDE desktops");
#define ARG_KINETD "kinetd"


static KCmdLineOptions options[] =
{
	{ ARG_KINETD " ", I18N_NOOP("Used for calling from kinetd."), 0},
	{ 0, 0, 0 }
};

/*
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
	KAboutData aboutData( "krfb", I18N_NOOP("Desktop Sharing"),
		VERSION, description, KAboutData::License_GPL,
		"(c) 2001-2002, Tim Jansen\n"
		"(c) 2001, Johannes E. Schindelin\n"
		"(c) 2000, heXoNet Support GmbH, D-66424 Homburg\n"
		"(c) 2000-2001, Const Kaplinsky\n"
		"(c) 2000, Tridia Corporation\n"
		"(c) 1999, AT&T Laboratories Cambridge\n",
                0, "", "ml@tjansen.de");
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
	if (!args->isSet(ARG_KINETD)) {
		bool kinetdA, krfbA;
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

		config = new Configuration(KRFB_INVITATION_MODE);
		config->showInvitationDialog();
		QObject::connect(config, SIGNAL(invitationFinished()),
				 &app, SLOT(quit()));
		return app.exec();
	}
	fdString = args->getOption(ARG_KINETD);
	config = new Configuration(KRFB_KINETD_MODE);	
	args->clear();

	if ((!config->allowUninvitedConnections()) && (config->invitations().size() == 0)) {
		KNotifyClient::event("UnexpectedConnection");
		return 1;
        }

	if (!RFBController::checkX11Capabilities())
		return 1;

	TrayIcon trayicon(new KAboutApplication(&aboutData),
			  config);
	KRfbIfaceImpl dcopiface(config);
	RFBController controller(config);

	QObject::connect(&app, SIGNAL(lastWindowClosed()), // dont show passivepopup
			 &trayicon, SLOT(prepareQuit()));
	QObject::connect(&app, SIGNAL(lastWindowClosed()),
			 &controller, SLOT(closeConnection()));

	QObject::connect(&trayicon, SIGNAL(showManageInvitations()),
			 config, SLOT(showManageInvitationsDialog()));
	QObject::connect(&trayicon, SIGNAL(enableDesktopControl(bool)),
			 &controller, SLOT(enableDesktopControl(bool)));
	QObject::connect(&trayicon, SIGNAL(diconnectedMessageDisplayed()),
			 &app, SLOT(quit()));

	QObject::connect(&dcopiface, SIGNAL(exitApp()),
			 &controller, SLOT(closeConnection()));

	QObject::connect(&controller, SIGNAL(sessionRefused()),
			 &app, SLOT(quit()));
	QObject::connect(&controller, SIGNAL(sessionEstablished()),
			 &trayicon, SLOT(showConnectedMessage()));
	QObject::connect(&controller, SIGNAL(sessionFinished()),
			 &trayicon, SLOT(showDisconnectedMessage()));
	QObject::connect(&controller, SIGNAL(desktopControlSettingChanged(bool)),
			 &trayicon, SLOT(setDesktopControlSetting(bool)));
	QObject::connect(&controller, SIGNAL(quitApp()),
			 &app, SLOT(quit()));

	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs, SIGPIPE);
	sigprocmask(SIG_BLOCK, &sigs, 0);

	bool ok;
	int fdNum = fdString.toInt(&ok);
	if (!ok) {
	  kdError() << "kinetd fd was not numeric." << endl;
	  return 2;
	}
	controller.startServer(fdNum);

	return app.exec();
}


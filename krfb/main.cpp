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
#include <ksystemtray.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kaboutapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qobject.h>
#include <qwindowdefs.h>

#include <signal.h>

#define VERSION "0.6"

static const char *description = I18N_NOOP("VNC-compatible server to share "
					   "KDE desktops");
#define ARG_ONE_SESSION "one-session"
#define ARG_PASSWORD "password"
#define ARG_DONT_CONFIRM_CONNECT "dont-confirm-connect"
#define ARG_REMOTE_CONTROL "remote-control"
	
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
	{ 0, 0, 0 }
};


int main(int argc, char *argv[])
{
	KAboutData aboutData( "krfb", I18N_NOOP("Desktop Sharing"),
		VERSION, description, KAboutData::License_GPL,
		"(c) 2001-2002, Tim Jansen\n"
                "(c) 2001 Johannes E. Schindelin\n"
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
			    I18N_NOOP("x11 update scanner, "
				      "original code base"));
	aboutData.addCredit("Jason Spisak", 
			    I18N_NOOP("Connection side image"), 
			    "kovalid@yahoo.com");
	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);

 	KApplication app;
	if (!RFBController::checkX11Capabilities())
		return 1;

	Configuration *config;
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if (args->isSet(ARG_ONE_SESSION) ||
	    args->isSet(ARG_PASSWORD) ||
	    args->isSet(ARG_REMOTE_CONTROL) ||
	    args->isSet(ARG_DONT_CONFIRM_CONNECT)) {

		bool oneConnection = args->isSet(ARG_ONE_SESSION);
		bool askOnConnect = !args->isSet(ARG_DONT_CONFIRM_CONNECT);
		bool allowDesktopControl = args->isSet(ARG_REMOTE_CONTROL);
		QString password = args->getOption(ARG_PASSWORD);
		config = new Configuration(oneConnection, askOnConnect, 
					   allowDesktopControl, password);
	}
	else
		config = new Configuration();
	args->clear();

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
			 config, SLOT(showDialog()));

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

	if (config->oneConnection()) {
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

	controller.startServer();

	return app.exec();
}

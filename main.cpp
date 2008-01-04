/***************************************************************************
                                   main.cpp
                             -------------------
    begin                : Sat Dec  8 03:23:02 CET 2001
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

#include "trayicon.h"
//#include "configuration.h"
#include "krfbserver.h"
#include "manageinvitationsdialog.h"

#include <QPixmap>
#include <kaction.h>
#include <kdebug.h>
#include <KNotification>
#include <ksystemtrayicon.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kaboutapplicationdialog.h>
#include <klocale.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <qwindowdefs.h>
#include <KIcon>

#include <signal.h>

#define VERSION "1.0"

static const char description[] = I18N_NOOP("VNC-compatible server to share "
					   "KDE desktops");


int main(int argc, char *argv[])
{
	KAboutData aboutData( "krfb", 0, ki18n("Desktop Sharing"),
        VERSION, ki18n(description), KAboutData::License_GPL,
        ki18n("(c) 2007, Alessandro Praduroux\n"
            "(c) 2001-2003, Tim Jansen\n"
        "(c) 2001, Johannes E. Schindelin\n"
        "(c) 2000, heXoNet Support GmbH, D-66424 Homburg\n"
        "(c) 2000-2001, Const Kaplinsky\n"
        "(c) 2000, Tridia Corporation\n"
        "(c) 1999, AT&T Laboratories Boston\n"));
    aboutData.addAuthor(ki18n("Alessandro Praduroux"), ki18n("KDE4 porting"), "pradu@pradu.it");
    aboutData.addAuthor(ki18n("Tim Jansen"), KLocalizedString(), "tim@tjansen.de");
    aboutData.addAuthor(ki18n("Ian Reinhart Geiser"), ki18n("DCOP interface"), "geiseri@kde.org");
	aboutData.addCredit(ki18n("Johannes E. Schindelin"),
			    ki18n("libvncserver"));
	aboutData.addCredit(ki18n("Const Kaplinsky"),
			    ki18n("TightVNC encoder"));
	aboutData.addCredit(ki18n("Tridia Corporation"),
			    ki18n("ZLib encoder"));
	aboutData.addCredit(ki18n("AT&T Laboratories Boston"),
			    ki18n("original VNC encoders and "
				      "protocol design"));
	aboutData.addCredit(ki18n("Jens Wagner (heXoNet Support GmbH)"),
			    ki18n("X11 update scanner, "
				      "original code base"));
	aboutData.addCredit(ki18n("Jason Spisak"),
			    ki18n("Connection side image"),
			    "kovalid@yahoo.com");
	aboutData.addCredit(ki18n("Karl Vogel"),
			    ki18n("KDesktop background deactivation"));
	KCmdLineArgs::init(argc, argv, &aboutData);

	KApplication app;
	QApplication::setWindowIcon(KIcon("krfb"));
    TrayIcon trayicon(new ManageInvitationsDialog);

	KrfbServer *server = KrfbServer::self(); // initialize the server manager
    if (!server->checkX11Capabilities()) {
        return 1;
    }

	QObject::connect(&trayicon, SIGNAL(enableDesktopControl(bool)),
			 server, SLOT(enableDesktopControl(bool)));
	QObject::connect(server, SIGNAL(sessionEstablished(QString)),
			 &trayicon, SLOT(showConnectedMessage(QString)));
	QObject::connect(server, SIGNAL(sessionFinished()),
			 &trayicon, SLOT(showDisconnectedMessage()));
	QObject::connect(server, SIGNAL(desktopControlSettingChanged(bool)),
			 &trayicon, SLOT(setDesktopControlSetting(bool)));
    QObject::connect(&trayicon, SIGNAL(quitSelected()),
			 server, SLOT(disconnectAndQuit()));
    QObject::connect(server, SIGNAL(quitApp()),
                      &app, SLOT(quit()));

	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs, SIGPIPE);
	sigprocmask(SIG_BLOCK, &sigs, 0);

	return app.exec();
}


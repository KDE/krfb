/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sat Dec  8 03:23:02 CET 2001
    copyright            : (C) 2001 by Tim Jansen
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

#include <kpixmap.h>
#include <kaction.h>
#include <kapplication.h>
#include <ksystemtray.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <qobject.h>

#define VERSION "0.1"

static const char *description = I18N_NOOP("Krfb");

	
static KCmdLineOptions options[] =
{
	{ 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{
	KAboutData aboutData( "krfb", I18N_NOOP("Krfb"),
		VERSION, description, KAboutData::License_GPL,
		"(c) 2001, Tim Jansen", 0, 0, "tim@tjansen.de");
	aboutData.addAuthor("Tim Jansen",0, "tim@tjansen.de");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options );

 	KApplication app;
 	TrayIcon trayicon;
	Configuration config;

	QObject::connect(&trayicon, SIGNAL(showConfigure()),
			 &config, SLOT(showDialog()));

	QObject::connect(&app, SIGNAL(lastWindowClosed()),
			 &app, SLOT(quit()));

	return app.exec();
}

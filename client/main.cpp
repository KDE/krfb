/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Thu Dec 20 15:11:42 CET 2001
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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <qwindowdefs.h>

#include "kvncview.h"
#include "krdc.h" 


#define VERSION "0.1"

static const char *description = I18N_NOOP("Remote Desktop Connection");
	

static KCmdLineOptions options[] =
{
	{ "+[host]", I18N_NOOP("An optional argument 'arg1'."), 0 },
	{ 0, 0, 0 }
};




int main(int argc, char *argv[])
{
	KAboutData aboutData( "krdc", I18N_NOOP("Remote Desktop Connection"),
			      VERSION, description, KAboutData::License_GPL,
			      "(c) 2001, Tim Jansen", 0, 0, "tim@tjansen.de");
	aboutData.addAuthor("Tim Jansen",0, "tim@tjansen.de");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); 

	KApplication a;

	Quality quality = QUALITY_HIGH;

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	KRDC *krdc;

	if (args->count() > 0) {
		QString host = args->arg(0);
		krdc = new KRDC(0, host, quality);		
	}
	else
		krdc = new KRDC();

	a.setMainWidget(krdc);

	if (!krdc->start())
		return 0;
	
	return a.exec();
}

/***************************************************************************
                          trayicon.h  -  description
                             -------------------
    begin                : Tue Dec 11 2001
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

#ifndef TRAYICON_H
#define TRAYICON_H

#include "configuration.h"

#include <qwidget.h>
#include <kpixmap.h>
#include <kaction.h>
#include <ksystemtray.h>

class KDialog;

/**
  *@author Tim Jansen
  */

class TrayIcon : public KSystemTray {
   	Q_OBJECT
public: 
	TrayIcon(KDialog*, Configuration*);
	~TrayIcon();

public slots:
  	void closeConnection();
	void openConnection();

signals:
  	void connectionClosed();
	void showConfigure();

private:
  	KPixmap trayIconOpen;
  	KPixmap trayIconClosed;
	KDialog* aboutDialog;
	KActionCollection actionCollection;
  	KAction* closeConnectionAction;
  	KAction* configureAction;
  	KAction* aboutAction;

private slots:
	void showAbout();
};

#endif

/***************************************************************************
                          krfb.cpp  -  description
                             -------------------
    begin                : Thu Dec  6 21:46:30 CET 2001
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
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>
#include <kmessagebox.h>

#include "krfb.h"

extern "C"
{
  KPanelApplet* init( QWidget *parent, const QString configFile)
  {
    KGlobal::locale()->insertCatalogue("krfb");
    return new Krfb(configFile, KPanelApplet::Normal,
                      KPanelApplet::About | KPanelApplet::Help | KPanelApplet::Preferences,
                      parent, "krfb");
  }
}

Krfb::Krfb(const QString& configFile, Type type, int actions, QWidget *parent, const char *name) 
       : KPanelApplet(configFile, type, actions, parent, name)
{
   // Get the current application configuration handle
   ksConfig = config();
   mainView = new myview(this);
   mainView->show();
}

Krfb::~Krfb()
{
}

void Krfb::about()
{
   KMessageBox::information(0, i18n("This is an about box"));
}

void Krfb::help()
{
   KMessageBox::information(0, i18n("This is a help box"));
}

void Krfb::preferences()
{
   KMessageBox::information(0, i18n("This is a preferences box"));  
}

int Krfb::widthForHeight( int height ) const
{
   return width();
}

int Krfb::heightForWidth( int width ) const
{
   return height();
}

void Krfb::resizeEvent( QResizeEvent *_Event)
{
   
}

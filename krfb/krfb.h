/***************************************************************************
                          krfb.h  -  description
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

#ifndef KRFB_H
#define KRFB_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapp.h>
#include <qwidget.h>

/** Krfb is the base class of the project */
class Krfb : public QWidget
{
  Q_OBJECT 
  public:
    /** construtor */
    Krfb(QWidget* parent=0, const char *name=0);
    /** destructor */
    ~Krfb();
};

#endif

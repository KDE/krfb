
/***************************************************************************
                                 kcm_krfb.h
                                ------------
    begin                : Sat Mar 02 2002
    copyright            : (C) 2002 by Tim Jansen
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

#ifndef _KCM_KRFB_H_
#define _KCM_KRFB_H_

#include <qobject.h>
#include <kcmodule.h>
#include "configurationwidget.h"

class KcmKRfb : public KCModule {
	Q_OBJECT
private:
	ConfigurationWidget confWidget;
	KAboutData *about;
	void checkKInetd(bool &available, bool &enabled);
	void setKInetd(bool enabled);
public:
	KcmKRfb(QWidget *p, const char *name, const QStringList &);
	~KcmKRfb();

	void load();
	void save();
	void defaults();
	QString quickHelp() const;
	const KAboutData *aboutData() const;
private slots:
	void configChanged();
};


#endif


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
#include <qdatetime.h>
#include <kcmodule.h>
#include "configurationwidget.h"
#include "../krfb/configuration.h"

class KcmKRfb : public KCModule {
	Q_OBJECT
private:
	Configuration m_configuration;
	ConfigurationWidget *m_confWidget;
	void checkKInetd(bool&, bool&);
public:
	KcmKRfb(QWidget *p, const char *name, const QStringList &);

	void load();
	void save();
	void defaults();
	QString quickHelp() const;
private slots:
	void setInvitationNum(int num);
	void configChanged();
};


#endif

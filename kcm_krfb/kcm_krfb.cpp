
/***************************************************************************
                               kcm_krfb.cpp
                              --------------
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

#include "kcm_krfb.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qcstring.h>
#include <qdatastream.h>
#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kgenericfactory.h>
#include <kdatastream.h>
#include <kdebug.h>
#include <dcopclient.h>

#define VERSION "0.7"


typedef KGenericFactory<KcmKRfb, QWidget> KcmKRfbFactory;
K_EXPORT_COMPONENT_FACTORY( libkcm_krfb, KcmKRfbFactory("kcm_krfb") );

KcmKRfb::KcmKRfb(QWidget *p, const char *name, const QStringList &) :
	KCModule(p, name),
	confWidget(this) {

	QVBoxLayout *l = new QVBoxLayout(this, 0, KDialog::spacingHint());
	l->add(&confWidget);

	setButtons(Default|Apply|Reset);

	about = new KAboutData( "kcm_krfb", I18N_NOOP("Desktop Sharing Control Module"),
		VERSION,
		I18N_NOOP("Configure desktop sharing"), KAboutData::License_GPL,
		"(c) 2002, Tim Jansen\n",
		0, "http://www.tjansen.de/krfb", "tim@tjansen.de");
	about->addAuthor("Tim Jansen", 0, "tim@tjansen.de");

	load();

	connect(confWidget.passwordInput, SIGNAL(textChanged(const QString&)), SLOT(configChanged()) );
	connect(confWidget.runOnDemandRB, SIGNAL(toggled(bool)), SLOT(configChanged()) );
	connect(confWidget.allowUninvitedCB, SIGNAL(clicked()), SLOT(configChanged()) );
	connect(confWidget.confirmConnectionsCB, SIGNAL(clicked()), SLOT(configChanged()) );
	connect(confWidget.allowDesktopControlCB, SIGNAL(clicked()), SLOT(configChanged()) );
}

KcmKRfb::~KcmKRfb() {
	delete about;
}

void KcmKRfb::configChanged() {
	emit changed(true);
}

void KcmKRfb::checkKInetd(bool &available, bool &enabled) {
	available = false;
	enabled = false;

	DCOPClient *d = KApplication::dcopClient();

	QByteArray sdata, rdata;
	QCString replyType;
	QDataStream arg(sdata, IO_WriteOnly);
	arg << QString("krfb");
	if (!d->call ("kded", "kinetd", "isEnabled(QString)", sdata, replyType, rdata))
		return;

	if (replyType != "bool")
		return;

	QDataStream answer(rdata, IO_ReadOnly);
	answer >> enabled;
	available = true;
}

void KcmKRfb::setKInetd(bool enabled) {
	DCOPClient *d = KApplication::dcopClient();

	QByteArray sdata;
	QDataStream arg(sdata, IO_WriteOnly);
	arg << QString("krfb");
	arg << enabled;
	if (!d->send ("kded", "kinetd", "setEnabled(QString,bool)", sdata)) {
		confWidget.runInBackgroundRB->setEnabled(false);
		confWidget.runOnDemandRB->setChecked(true);
	}
}


void KcmKRfb::load() {
	bool kinetdAvailable, kinetdEnabled;
	checkKInetd(kinetdAvailable, kinetdEnabled);

	if (!kinetdAvailable) {
		confWidget.runInBackgroundRB->setEnabled(false);
		confWidget.runOnDemandRB->setChecked(true);
	}
	else {
		confWidget.runInBackgroundRB->setEnabled(true);
		confWidget.runInBackgroundRB->setChecked(kinetdEnabled);
		confWidget.runOnDemandRB->setChecked(!kinetdEnabled);
	}

	KConfig c("krfbrc");
	confWidget.allowUninvitedCB->setChecked(c.readBoolEntry("allowUninvited", false));
	confWidget.confirmConnectionsCB->setChecked(c.readBoolEntry("confirmUninvitedConnection", false));
	confWidget.allowDesktopControlCB->setChecked(c.readBoolEntry("allowDesktopControl", false));
	confWidget.passwordInput->setText(c.readEntry("uninvitedPassword", ""));
}

void KcmKRfb::save() {
	KConfig c("krfbrc");

	setKInetd(!confWidget.runOnDemandRB->isChecked());

	c.writeEntry("allowUninvited", confWidget.allowUninvitedCB->isChecked());
	c.writeEntry("confirmUninvitedConnection", confWidget.confirmConnectionsCB->isChecked());
	c.writeEntry("allowDesktopControl", confWidget.allowDesktopControlCB->isChecked());
	c.writeEntry("uninvitedPassword", confWidget.passwordInput->text());
}

void KcmKRfb::defaults() {
	bool kinetdAvailable, kinetdEnabled;
	checkKInetd(kinetdAvailable, kinetdEnabled);

	if (!kinetdAvailable) {
		confWidget.runInBackgroundRB->setEnabled(false);
		confWidget.runOnDemandRB->setChecked(true);
	}
	else {
		confWidget.runInBackgroundRB->setEnabled(true);
		confWidget.runInBackgroundRB->setChecked(true);
		confWidget.runOnDemandRB->setChecked(false);
	}
	confWidget.allowUninvitedCB->setChecked(false);
	confWidget.confirmConnectionsCB->setChecked(false);
	confWidget.allowDesktopControlCB->setChecked(false);
	confWidget.passwordInput->setText("");
}

const KAboutData *KcmKRfb::aboutData() const
{
	return about;
}

QString KcmKRfb::quickHelp() const
{
	return i18n("<h1>desktop sharing</h1> This module allows you to configure"
	            " the KDE desktop sharing.");
}



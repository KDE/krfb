
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
#include "kcm_krfb.moc"

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
	m_configuration(KRFB_CONFIGURATION_MODE) {

        m_confWidget = new ConfigurationWidget(this);

	QVBoxLayout *l = new QVBoxLayout(this, 0, KDialog::spacingHint());
	l->add(m_confWidget);

	setButtons(Default|Apply|Reset);

	m_about = new KAboutData( "kcm_krfb", I18N_NOOP("Desktop Sharing Control Module"),
		VERSION,
		I18N_NOOP("Configure desktop sharing"), KAboutData::License_GPL,
		"(c) 2002, Tim Jansen\n",
		0, "http://www.tjansen.de/krfb", "tim@tjansen.de");
	m_about->addAuthor("Tim Jansen", 0, "tim@tjansen.de");

	load();

	connect(m_confWidget->passwordInput, SIGNAL(textChanged(const QString&)), SLOT(configChanged()) );
	connect(m_confWidget->allowUninvitedCB, SIGNAL(clicked()), SLOT(configChanged()) );
	connect(m_confWidget->confirmConnectionsCB, SIGNAL(clicked()), SLOT(configChanged()) );
	connect(m_confWidget->allowDesktopControlCB, SIGNAL(clicked()), SLOT(configChanged()) );
	connect((QObject*)m_confWidget->createInvitation, SIGNAL(clicked()), 
		&m_configuration, SLOT(showInvitationDialog()) );
	connect((QObject*)m_confWidget->manageInvitations, SIGNAL(clicked()), 
		&m_configuration, SLOT(showManageInvitationsDialog()) );
}

KcmKRfb::~KcmKRfb() {
	delete m_about;
}

void KcmKRfb::configChanged() {
	emit changed(true);
}

void KcmKRfb::checkKInetd(bool &kinetdAvailable, bool &krfbAvailable) {
	kinetdAvailable = false;
	krfbAvailable = false;

	DCOPClient *d = KApplication::dcopClient();

	QByteArray sdata, rdata;
	QCString replyType;
	QDataStream arg(sdata, IO_WriteOnly);
	arg << QString("krfb");
	if (!d->call ("kded", "kinetd", "isInstalled(QString)", sdata, replyType, rdata))
		return;

	if (replyType != "bool")
		return;

	QDataStream answer(rdata, IO_ReadOnly);
	answer >> krfbAvailable;
	kinetdAvailable = true;
}

void KcmKRfb::load() {
	bool kinetdAvailable, krfbAvailable;
	checkKInetd(kinetdAvailable, krfbAvailable);

	m_confWidget->allowUninvitedCB->setChecked(m_configuration.allowUninvitedConnections());
	m_confWidget->confirmConnectionsCB->setChecked(m_configuration.askOnConnect());
	m_confWidget->allowDesktopControlCB->setChecked(m_configuration.allowDesktopControl());
	m_confWidget->passwordInput->setText(m_configuration.password());
}

void KcmKRfb::save() {

        m_configuration.update();
	bool allowUninvited = m_confWidget->allowUninvitedCB->isChecked();
	m_configuration.setAllowUninvited(allowUninvited);
	m_configuration.setAskOnConnect(m_confWidget->confirmConnectionsCB->isChecked());
	m_configuration.setAllowDesktopControl(m_confWidget->allowDesktopControlCB->isChecked());
	m_configuration.setPassword(m_confWidget->passwordInput->text());
	m_configuration.save();
}

void KcmKRfb::defaults() {
	bool kinetdAvailable, krfbAvailable;
	checkKInetd(kinetdAvailable, krfbAvailable);

	m_confWidget->allowUninvitedCB->setChecked(false);
	m_confWidget->confirmConnectionsCB->setChecked(false);
	m_confWidget->allowDesktopControlCB->setChecked(false);
	m_confWidget->passwordInput->setText("");
}

const KAboutData *KcmKRfb::aboutData() const
{
	return m_about;
}

QString KcmKRfb::quickHelp() const
{
	return i18n("<h1>Desktop Sharing Configuration</h1> This module allows you to configure"
	            " the KDE desktop sharing.");
}



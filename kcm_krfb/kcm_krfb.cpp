
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
#include <qlabel.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qdatastream.h>
#include <kdialog.h>
#include <knuminput.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <QDBusInterface>
#include <QDBusReply>
#define VERSION "0.7"


typedef KGenericFactory<KcmKRfb, QWidget> KcmKRfbFactory;
K_EXPORT_COMPONENT_FACTORY(krfb, KcmKRfbFactory("kcm_krfb"))



KcmKRfb::KcmKRfb(QWidget *p, const QStringList &) :
    KCModule(KcmKRfbFactory::componentData(), p),
    m_configuration(KRFB_CONFIGURATION_MODE)
{

    KGlobal::locale()->insertCatalog("krfb");
    m_confWidget = new ConfigurationWidget(this);

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setSpacing(KDialog::spacingHint());
    l->setMargin(0);
    l->add(m_confWidget);

    setButtons(Default | Apply | Default);

    KAboutData *about = new KAboutData("kcm_krfb", 0, ki18n("Desktop Sharing Control Module"),
                                       VERSION,
                                       ki18n("Configure desktop sharing"), KAboutData::License_GPL,
                                       ki18n("(c) 2002, Tim Jansen\n"),
                                       KLocalizedString(), "http://www.tjansen.de/krfb", "tim@tjansen.de");
    about->addAuthor(ki18n("Tim Jansen"), KLocalizedString(), "tim@tjansen.de");
    setAboutData(about);

    load();

    connect(m_confWidget->passwordInput, SIGNAL(textChanged(const QString &)), SLOT(configChanged()));
    connect(m_confWidget->allowUninvitedCB, SIGNAL(clicked()), SLOT(configChanged()));
    connect(m_confWidget->enableSLPCB, SIGNAL(clicked()), SLOT(configChanged()));
    connect(m_confWidget->confirmConnectionsCB, SIGNAL(clicked()), SLOT(configChanged()));
    connect(m_confWidget->allowDesktopControlCB, SIGNAL(clicked()), SLOT(configChanged()));
    connect(m_confWidget->autoPortCB, SIGNAL(clicked()), SLOT(configChanged()));
    connect(m_confWidget->portInput, SIGNAL(valueChanged(int)), SLOT(configChanged()));
    connect((QObject *)m_confWidget->manageInvitations, SIGNAL(clicked()),
            &m_configuration, SLOT(showManageInvitationsDialog()));
    connect(&m_configuration, SIGNAL(invitationNumChanged(int)),
            this, SLOT(setInvitationNum(int)));
    setInvitationNum(m_configuration.invitations().size());
    connect(m_confWidget->disableBackgroundCB, SIGNAL(clicked()), SLOT(configChanged()));
}

void KcmKRfb::configChanged()
{
    emit changed(true);
}

void KcmKRfb::setInvitationNum(int num)
{
    if (num == 0) {
        m_confWidget->invitationNumLabel->setText(i18n("You have no open invitation."));
    } else {
        m_confWidget->invitationNumLabel->setText(i18n("Open invitations: %1", num));
    }
}

void KcmKRfb::checkKInetd(bool &kinetdAvailable, bool &krfbAvailable)
{
    kinetdAvailable = false;
    krfbAvailable = false;
    //TODO verify it when kinetd will port
    QDBusInterface kinetd("org.kde.kded", "/modules/kinetd", "org.kde.kinetd");
    QDBusReply<bool> reply = kinetd.call("isInstalled", "krfb");

    if (!reply.isValid()) {
        return;
    }

    krfbAvailable = reply;
    kinetdAvailable = true;
}

void KcmKRfb::load()
{
    bool kinetdAvailable, krfbAvailable;
    checkKInetd(kinetdAvailable, krfbAvailable);

    m_confWidget->allowUninvitedCB->setChecked(m_configuration.allowUninvitedConnections());
    m_confWidget->enableSLPCB->setChecked(m_configuration.enableSLP());
    m_confWidget->confirmConnectionsCB->setChecked(m_configuration.askOnConnect());
    m_confWidget->allowDesktopControlCB->setChecked(m_configuration.allowDesktopControl());
    m_confWidget->passwordInput->setText(m_configuration.password());
    m_confWidget->autoPortCB->setChecked(m_configuration.preferredPort() < 0);
    m_confWidget->portInput->setValue(m_configuration.preferredPort() > 0 ?
                                      m_configuration.preferredPort() : 5900);
    m_confWidget->disableBackgroundCB->setChecked(m_configuration.disableBackground());
    emit changed(false);
}

void KcmKRfb::save()
{

    m_configuration.update();
    bool allowUninvited = m_confWidget->allowUninvitedCB->isChecked();
    m_configuration.setAllowUninvited(allowUninvited);
    m_configuration.setEnableSLP(m_confWidget->enableSLPCB->isChecked());
    m_configuration.setAskOnConnect(m_confWidget->confirmConnectionsCB->isChecked());
    m_configuration.setAllowDesktopControl(m_confWidget->allowDesktopControlCB->isChecked());
    m_configuration.setPassword(m_confWidget->passwordInput->text());

    if (m_confWidget->autoPortCB->isChecked()) {
        m_configuration.setPreferredPort(-1);
    } else {
        m_configuration.setPreferredPort(m_confWidget->portInput->value());
    }

    m_configuration.setDisableBackground(m_confWidget->disableBackgroundCB->isChecked());
    m_configuration.save();
#if 0
    kapp->dcopClient()->emitDCOPSignal("KRFB::ConfigChanged", "KRFB_ConfigChanged()", QByteArray());
#endif
    emit changed(false);
}

void KcmKRfb::defaults()
{
    bool kinetdAvailable, krfbAvailable;
    checkKInetd(kinetdAvailable, krfbAvailable);

    m_confWidget->allowUninvitedCB->setChecked(false);
    m_confWidget->enableSLPCB->setChecked(true);
    m_confWidget->confirmConnectionsCB->setChecked(false);
    m_confWidget->allowDesktopControlCB->setChecked(false);
    m_confWidget->passwordInput->setText("");
    m_confWidget->autoPortCB->setChecked(true);
    m_confWidget->portInput->setValue(5900);
    m_confWidget->disableBackgroundCB->setChecked(false);
    emit changed(true);
}

QString KcmKRfb::quickHelp() const
{
    return i18n("<h1>Desktop Sharing</h1> This module allows you to configure"
                " the KDE desktop sharing.");
}



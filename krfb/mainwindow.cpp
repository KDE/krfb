/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
   Copyright (C) 2013 Amandeep Singh <aman.dedman@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "mainwindow.h"
#include "invitationsrfbserver.h"

#include "krfbconfig.h"
#include "ui_configtcp.h"
#include "ui_configsecurity.h"

#include <KConfigDialog>
#include <KIcon>
#include <KLocale>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <KSystemTimeZone>
#include <KToolInvocation>
#include <KStandardAction>
#include <KActionCollection>
#include <KLineEdit>
#include <KNewPasswordDialog>

#include <QtGui/QWidget>
#include <QtNetwork/QNetworkInterface>

class TCP: public QWidget, public Ui::TCP
{
public:
    TCP(QWidget *parent = 0) : QWidget(parent) {
        setupUi(this);
    }
};

class Security: public QWidget, public Ui::Security
{
public:
    Security(QWidget *parent = 0) : QWidget(parent) {
        setupUi(this);
    }
};


MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, false);

    m_passwordEditable = false;
    m_passwordLineEdit = new KLineEdit(this);
    m_passwordLineEdit->setVisible(false);
    m_passwordLineEdit->setAlignment(Qt::AlignHCenter);

    QWidget *mainWidget = new QWidget;
    m_ui.setupUi(mainWidget);
    m_ui.krfbIconLabel->setPixmap(KIcon("krfb").pixmap(128));
    setCentralWidget(mainWidget);

    connect(m_ui.passwordEditButton,SIGNAL(clicked()),
            this,SLOT(editPassword()));
    connect(m_ui.enableSharingCheckBox,SIGNAL(toggled(bool)),
            this, SLOT(toggleDesktopSharing(bool)));
    connect(m_ui.enableUnattendedCheckBox, SIGNAL(toggled(bool)),
            InvitationsRfbServer::instance, SLOT(toggleUnattendedAccess(bool)));
    connect(m_ui.unattendedPasswordButton, SIGNAL(clicked()),
            this, SLOT(editUnattendedPassword()));

    // Figure out the address
    int port = KrfbConfig::port();
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    foreach(const QNetworkInterface & interface, interfaceList) {
        if(interface.flags() & QNetworkInterface::IsLoopBack)
            continue;

        if(interface.flags() & QNetworkInterface::IsRunning &&
                !interface.addressEntries().isEmpty())
            m_ui.addressDisplayLabel->setText(QString("%1 : %2")
                    .arg(interface.addressEntries().first().ip().toString())
                    .arg(port));
    }

    //Figure out the password
    m_ui.passwordDisplayLabel->setText(
            InvitationsRfbServer::instance->desktopPassword());

    KStandardAction::quit(QCoreApplication::instance(), SLOT(quit()), actionCollection());
    KStandardAction::preferences(this, SLOT(showConfiguration()), actionCollection());

    setupGUI();
    setFixedSize(QSize(600, 360));

   // setAutoSaveSettings();
}

MainWindow::~MainWindow()
{
}

void MainWindow::editPassword()
{
    if(m_passwordEditable) {
        m_passwordEditable = false;
        m_ui.passwordEditButton->setIcon(KIcon("document-properties"));
        m_ui.passwordGridLayout->removeWidget(m_passwordLineEdit);
        InvitationsRfbServer::instance->setDesktopPassword(
                m_passwordLineEdit->text());
        m_ui.passwordDisplayLabel->setText(
                InvitationsRfbServer::instance->desktopPassword());
        m_passwordLineEdit->setVisible(false);
    } else {
        m_passwordEditable = true;
        m_ui.passwordEditButton->setIcon(KIcon("document-save"));
        m_ui.passwordGridLayout->addWidget(m_passwordLineEdit,0,0);
        m_passwordLineEdit->setText(
                InvitationsRfbServer::instance->desktopPassword());
        m_passwordLineEdit->setVisible(true);
        m_passwordLineEdit->setFocus(Qt::MouseFocusReason);
    }
}

void MainWindow::editUnattendedPassword()
{
    KNewPasswordDialog dialog(this);
    dialog.setPrompt(i18n("Enter a new password for Unattended Access"));
    if(dialog.exec()) {
        InvitationsRfbServer::instance->setUnattendedPassword(dialog.password());
    }
}


void MainWindow::toggleDesktopSharing(bool enable)
{
    if(enable) {
        if(!InvitationsRfbServer::instance->start()) {
            KMessageBox::error(0,
                    i18n("Failed to start the krfb server. Desktop sharing "
                        "will not work. Try setting another port in the settings "
                        "and restart krfb."));
        }
        connect(qApp, SIGNAL(aboutToQuit()), InvitationsRfbServer::instance, SLOT(stop()));
    } else {
        disconnect(qApp, SIGNAL(aboutToQuit()), InvitationsRfbServer::instance, SLOT(stop()));
        InvitationsRfbServer::instance->stop();
        if(m_passwordEditable) {
            m_passwordEditable = false;
            m_passwordLineEdit->setVisible(false);
            m_ui.passwordEditButton->setIcon(KIcon("document-properties"));
        }
    }
}

void MainWindow::showConfiguration()
{
    if (KConfigDialog::showDialog("settings")) {
        return;
    }

    KConfigDialog *dialog = new KConfigDialog(this, "settings", KrfbConfig::self());
    dialog->addPage(new TCP, i18n("Network"), "network-workgroup");
    dialog->addPage(new Security, i18n("Security"), "security-high");
    dialog->setHelp(QString(), "krfb");
    dialog->show();
}

void MainWindow::readProperties(const KConfigGroup& group)
{
    if (group.readEntry("Visible", true)) {
        show();
    }
    KMainWindow::readProperties(group);
}

void MainWindow::saveProperties(KConfigGroup& group)
{
    group.writeEntry("Visible", isVisible());
    KMainWindow::saveProperties(group);
}

#include "mainwindow.moc"

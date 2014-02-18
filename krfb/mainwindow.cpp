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
#include <QtGui/QSizePolicy>
#include <QtNetwork/QNetworkInterface>

#ifdef KRFB_WITH_KDE_TELEPATHY
#include "tubesrfbserver.h"
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingChannelRequest>
#include <KTp/actions.h>
#include <KTp/Widgets/contact-view-widget.h>
#include <KTp/Models/contacts-list-model.h>
#include <KTp/Models/contacts-filter-model.h>
#endif

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
    m_ui.enableUnattendedCheckBox->setChecked(
            InvitationsRfbServer::instance->allowUnattendedAccess());

    setCentralWidget(mainWidget);

    connect(m_ui.passwordEditButton,SIGNAL(clicked()),
            this,SLOT(editPassword()));
    connect(m_ui.enableSharingCheckBox,SIGNAL(toggled(bool)),
            this, SLOT(toggleDesktopSharing(bool)));
    connect(m_ui.enableUnattendedCheckBox, SIGNAL(toggled(bool)),
            InvitationsRfbServer::instance, SLOT(toggleUnattendedAccess(bool)));
    connect(m_ui.unattendedPasswordButton, SIGNAL(clicked()),
            this, SLOT(editUnattendedPassword()));
    connect(m_ui.addressAboutButton, SIGNAL(clicked()),
            this, SLOT(aboutConnectionAddress()));
    connect(m_ui.unattendedAboutButton, SIGNAL(clicked()),
            this, SLOT(aboutUnattendedMode()));
    connect(InvitationsRfbServer::instance, SIGNAL(passwordChanged(const QString&)),
            this, SLOT(passwordChanged(const QString&)));

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


#ifdef KRFB_WITH_KDE_TELEPATHY

    m_contactViewWidget = new KTp::ContactViewWidget(
            TubesRfbServer::instance->contactsListModel(), this);

    m_contactViewWidget->setEnabled(false);
    connect(m_ui.enableSharingCheckBox, SIGNAL(toggled(bool)),
            m_contactViewWidget, SLOT(setEnabled(bool)));
    m_contactViewWidget->setIconSize(QSize(32,32));
    m_contactViewWidget->setMinimumWidth(120);
    m_contactViewWidget->setMaximumWidth(360);
    m_contactViewWidget->setMinimumHeight(300);
    m_contactViewWidget->contactFilterLineEdit()->setClickMessage(i18n("Search in Contacts..."));
    m_contactViewWidget->filter()->setPresenceTypeFilterFlags(KTp::ContactsFilterModel::ShowOnlyConnected);
    m_contactViewWidget->filter()->setTubesFilterStrings(QStringList("rfb"));
    m_contactViewWidget->filter()->setCapabilityFilterFlags(KTp::ContactsFilterModel::FilterByTubes);

    m_contactViewWidget->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    m_ui.tpContactsLayout->addWidget(m_contactViewWidget);
    connect(m_contactViewWidget, SIGNAL(contactDoubleClicked(const Tp::AccountPtr &, const KTp::ContactPtr &)),
        this, SLOT(onContactDoubleClicked(const Tp::AccountPtr &, const KTp::ContactPtr &)));
#endif


    KStandardAction::quit(QCoreApplication::instance(), SLOT(quit()), actionCollection());
    KStandardAction::preferences(this, SLOT(showConfiguration()), actionCollection());

    setupGUI();

    setAutoSaveSettings();
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
            KMessageBox::error(this,
                    i18n("Failed to start the krfb server. Desktop sharing "
                        "will not work. Try setting another port in the settings "
                        "and restart krfb."));
        }
    } else {
        InvitationsRfbServer::instance->stop();
        if(m_passwordEditable) {
            m_passwordEditable = false;
            m_passwordLineEdit->setVisible(false);
            m_ui.passwordEditButton->setIcon(KIcon("document-properties"));
        }
    }
}

void MainWindow::passwordChanged(const QString& password)
{
    m_passwordLineEdit->setText(password);
    m_ui.passwordDisplayLabel->setText(password);
}

void MainWindow::aboutConnectionAddress()
{
    KMessageBox::about(this,
            i18n("This field contains the address of your computer and the port number, separated by a colon.\n\nThe address is just a hint - you can use any address that can reach your computer.\n\nDesktop Sharing tries to guess your address from your network configuration, but does not always succeed in doing so.\n\nIf your computer is behind a firewall it may have a different address or be unreachable for other computers."),
            i18n("KDE Desktop Sharing"));
}

void MainWindow::aboutUnattendedMode()
{
    KMessageBox::about(this,
            i18n("Any remote user with normal desktop sharing password will have to be authenticated.\n\nIf unattended access is on, and the remote user provides unattended mode password, desktop sharing access will be granted without explicit confirmation."),
            i18n("KDE Desktop Sharing"));
}

#ifdef KRFB_WITH_KDE_TELEPATHY

void MainWindow::onContactDoubleClicked(const Tp::AccountPtr &account, const KTp::ContactPtr &contact)
{
    Tp::PendingOperation *op = KTp::Actions::startDesktopSharing(account, contact);
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(pendingDesktopShareFinished(Tp::PendingOperation*)));
}

void MainWindow::pendingDesktopShareFinished(Tp::PendingOperation *operation)
{
    if(operation->isError()) {
        KMessageBox::error(this,
                operation->errorName() + ": " + operation->errorMessage());
    }
}

#endif

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

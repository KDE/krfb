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
#include "ui_configframebuffer.h"

#include <KConfigDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardAction>
#include <KActionCollection>
#include <KLineEdit>
#include <KNewPasswordDialog>
#include <KPluginLoader>
#include <KPluginMetaData>

#include <QIcon>
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSizePolicy>
#include <QVector>
#include <QSet>
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

class ConfigFramebuffer: public QWidget, public Ui::Framebuffer
{
public:
    ConfigFramebuffer(QWidget *parent = 0) : QWidget(parent) {
        setupUi(this);
        // hide the line edit with framebuffer string
        kcfg_preferredFrameBufferPlugin->hide();
        // fill drop-down combo with a list of real existing plugins
        this->fillFrameBuffersCombo();
        // initialize combo with currently configured framebuffer plugin
        cb_preferredFrameBufferPlugin->setCurrentText(KrfbConfig::preferredFrameBufferPlugin());
        // connect signals between combo<->lineedit
        // if we change selection in combo, lineedit is updated
        QObject::connect(cb_preferredFrameBufferPlugin, &QComboBox::currentTextChanged,
                         kcfg_preferredFrameBufferPlugin, &QLineEdit::setText);
    }

    void fillFrameBuffersCombo() {
        const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(
                    QStringLiteral("krfb"), [](const KPluginMetaData & md) {
            return md.serviceTypes().contains(QStringLiteral("krfb/framebuffer"));
        });
        QSet<QString> unique;
        QVectorIterator<KPluginMetaData> i(plugins);
        i.toBack();
        while (i.hasPrevious()) {
            const KPluginMetaData &metadata = i.previous();
            if (unique.contains(metadata.pluginId())) continue;
            cb_preferredFrameBufferPlugin->addItem(metadata.pluginId());
            unique.insert(metadata.pluginId());
        }
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
    m_ui.krfbIconLabel->setPixmap(QIcon::fromTheme("krfb").pixmap(128));
    m_ui.enableUnattendedCheckBox->setChecked(
            InvitationsRfbServer::instance->allowUnattendedAccess());

    setCentralWidget(mainWidget);

    connect(m_ui.passwordEditButton, &QToolButton::clicked,
            this, &MainWindow::editPassword);
    connect(m_ui.enableSharingCheckBox, &QCheckBox::toggled,
            this, &MainWindow::toggleDesktopSharing);
    connect(m_ui.enableUnattendedCheckBox, &QCheckBox::toggled,
            InvitationsRfbServer::instance, &InvitationsRfbServer::toggleUnattendedAccess);
    connect(m_ui.unattendedPasswordButton, &QPushButton::clicked,
            this, &MainWindow::editUnattendedPassword);
    connect(m_ui.addressAboutButton, &QToolButton::clicked,
            this, &MainWindow::aboutConnectionAddress);
    connect(m_ui.unattendedAboutButton, &QToolButton::clicked,
            this, &MainWindow::aboutUnattendedMode);
    connect(InvitationsRfbServer::instance, &InvitationsRfbServer::passwordChanged,
            this, &MainWindow::passwordChanged);

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

    if (KrfbConfig::allowDesktopControl()) {
      m_ui.enableSharingCheckBox->setChecked(true);
    }

    setAutoSaveSettings();
}

MainWindow::~MainWindow()
{
}

void MainWindow::editPassword()
{
    if(m_passwordEditable) {
        m_passwordEditable = false;
        m_ui.passwordEditButton->setIcon(QIcon::fromTheme("document-properties"));
        m_ui.passwordGridLayout->removeWidget(m_passwordLineEdit);
        InvitationsRfbServer::instance->setDesktopPassword(
                m_passwordLineEdit->text());
        m_ui.passwordDisplayLabel->setText(
                InvitationsRfbServer::instance->desktopPassword());
        m_passwordLineEdit->setVisible(false);
    } else {
        m_passwordEditable = true;
        m_ui.passwordEditButton->setIcon(QIcon::fromTheme("document-save"));
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
            m_ui.passwordEditButton->setIcon(QIcon::fromTheme("document-properties"));
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

void MainWindow::showConfiguration()
{
    static QString s_prevFramebufferPlugin;
    // ^^ needs to be static, because lambda will be called long time
    //    after showConfiguration() ends, so auto variable would go out of scope
    // save previously selected framebuffer plugin config
    s_prevFramebufferPlugin = KrfbConfig::preferredFrameBufferPlugin();

    if (KConfigDialog::showDialog("settings")) {
        return;
    }

    KConfigDialog *dialog = new KConfigDialog(this, "settings", KrfbConfig::self());
    dialog->addPage(new TCP, i18n("Network"), "network-wired");
    dialog->addPage(new Security, i18n("Security"), "security-high");
    dialog->addPage(new ConfigFramebuffer, i18n("Screen capture"), "video-display");
    dialog->show();
    connect(dialog, &KConfigDialog::settingsChanged, [this] () {
        // check if framebuffer plugin config has changed
        if (s_prevFramebufferPlugin != KrfbConfig::preferredFrameBufferPlugin()) {
            KMessageBox::information(this, i18n("To apply framebuffer plugin setting, "
                                                "you need to restart the program."));
        }
    });
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

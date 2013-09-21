/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
   Copyright (C) 2013 Amandeep Singh <aman.dedman@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "mainwindow.h"

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
#include <KRandom>

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
    m_password = readableRandomString(4) + '-' + readableRandomString(3);
    m_ui.passwordDisplayLabel->setText(m_password);

    KStandardAction::quit(QCoreApplication::instance(), SLOT(quit()), actionCollection());
    KStandardAction::preferences(this, SLOT(showConfiguration()), actionCollection());

    setupGUI();
    setFixedSize(QSize(580, 250));

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
        m_password = m_passwordLineEdit->text();
        m_ui.passwordDisplayLabel->setText(m_password);
        m_passwordLineEdit->setVisible(false);
        m_ui.passwordGridLayout->removeWidget(m_passwordLineEdit);
    } else {
        m_passwordEditable = true;
        m_ui.passwordEditButton->setIcon(KIcon("document-save"));
        m_passwordLineEdit->setText(m_password);
        m_passwordLineEdit->setVisible(true);
        m_ui.passwordGridLayout->addWidget(m_passwordLineEdit,0,0);
    }
}

void MainWindow::toggleDesktopSharing(bool enable)
{
    if(enable) {
        //TODO Start Server
        kWarning() << "This build is broken. No incoming request can be accepted.";
    } else {
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

// a random string that doesn't contain i, I, o, O, 1, l, 0
// based on KRandom::randomString()
QString MainWindow::readableRandomString(int length)
{
    QString str;
    while (length) {
        int r = KRandom::random() % 62;
        r += 48;
        if (r > 57) {
            r += 7;
        }
        if (r > 90) {
            r += 6;
        }
        char c = char(r);
        if ((c == 'i') ||
                (c == 'I') ||
                (c == '1') ||
                (c == 'l') ||
                (c == 'o') ||
                (c == 'O') ||
                (c == '0')) {
            continue;
        }
        str += c;
        length--;
    }
    return str;
}

#include "mainwindow.moc"

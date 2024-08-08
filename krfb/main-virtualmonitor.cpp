/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

   SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "krfb_version.h"
#include "rfbserver.h"
#include "rfbservermanager.h"
#include "sockethelpers.h"
#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>
#include <KNotification>
#include <KWindowSystem>
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QTimer>
#include <signal.h>

class VirtualMonitorRfbClient : public RfbClient
{
public:
    explicit VirtualMonitorRfbClient(rfbClientPtr client, QObject *parent = nullptr)
        : RfbClient(client, parent)
    {
    }
};

class PendingVirtualMonitorRfbClient : public PendingRfbClient
{
public:
    explicit PendingVirtualMonitorRfbClient(rfbClientPtr client, QObject *parent = nullptr)
        : PendingRfbClient(client, parent)
    {
    }
    ~PendingVirtualMonitorRfbClient() override
    {
    }

    static QByteArray password;

protected:
    void processNewClient() override
    {
        qDebug() << "new client!";
        const QString host = peerAddress(m_rfbClient->sock) + QLatin1Char(':') + QString::number(peerPort(m_rfbClient->sock));

        KNotification::event(QStringLiteral("NewConnectionAutoAccepted"), i18n("Creating a Virtual Monitor from %1", host));
    }
    bool checkPassword(const QByteArray &encryptedPassword) override
    {
        bool b = vncAuthCheckPassword(password, encryptedPassword);
        if (b) {
            QTimer::singleShot(0, this, [this] {
                accept(new VirtualMonitorRfbClient(m_rfbClient, parent()));
            });
        }
        return b;
    }
};

QByteArray PendingVirtualMonitorRfbClient::password;

class VirtualMonitorRfbServer : public RfbServer
{
public:
    PendingRfbClient *newClient(rfbClientPtr client) override
    {
        qDebug() << "new client request";
        return new PendingVirtualMonitorRfbClient(client, this);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("krfb");

    KAboutData aboutData(QStringLiteral("krfb-virtualmonitor"),
                         i18n("Remote Virtual Monitor"),
                         QStringLiteral(KRFB_VERSION_STRING),
                         i18n("Offer a Virtual Monitor that can be accessed remotely"),
                         KAboutLicense::GPL,
                         i18n("(c) 2009-2010, Collabora Ltd.\n"
                              "(c) 2007, Alessandro Praduroux\n"
                              "(c) 2001-2003, Tim Jansen\n"
                              "(c) 2001, Johannes E. Schindelin\n"
                              "(c) 2000-2001, Const Kaplinsky\n"
                              "(c) 2000, Tridia Corporation\n"
                              "(c) 1999, AT&T Laboratories Boston\n"));
    aboutData.addAuthor(QStringLiteral("Aleix Pol i Gonzalez"), i18n("Virtual Monitor implementation"), QStringLiteral("aleixpol@kde.org"));
    aboutData.addAuthor(i18n("George Kiagiadakis"), QString(), QStringLiteral("george.kiagiadakis@collabora.co.uk"));
    aboutData.addAuthor(i18n("Alessandro Praduroux"), i18n("KDE4 porting"), QStringLiteral("pradu@pradu.it"));
    aboutData.addAuthor(i18n("Tim Jansen"), i18n("Original author"), QStringLiteral("tim@tjansen.de"));
    aboutData.addCredit(i18n("Johannes E. Schindelin"), i18n("libvncserver"));
    aboutData.addCredit(i18n("Const Kaplinsky"), i18n("TightVNC encoder"));
    aboutData.addCredit(i18n("Tridia Corporation"), i18n("ZLib encoder"));
    aboutData.addCredit(i18n("AT&T Laboratories Boston"),
                        i18n("original VNC encoders and "
                             "protocol design"));
    KAboutData::setApplicationData(aboutData);

    KCrash::initialize();

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    const QCommandLineOption resolutionOption({QStringLiteral("resolution")}, i18n("Logical resolution of the new monitor"), i18n("resolution"));
    parser.addOption(resolutionOption);
    const QCommandLineOption nameOption({QStringLiteral("name")}, i18n("Name of the monitor"), i18n("name"));
    parser.addOption(nameOption);
    const QCommandLineOption passwordOption({QStringLiteral("password")}, i18n("Password for the client to connect to it"), i18n("password"));
    parser.addOption(passwordOption);
    const QCommandLineOption scaleOption({QStringLiteral("scale")},
                                         i18n("The device-pixel-ratio of the device, the scaling factor"),
                                         i18n("dpr"),
                                         QStringLiteral("1"));
    parser.addOption(scaleOption);
    const QCommandLineOption portOption({QStringLiteral("port")}, i18n("The port we will be listening to"), i18n("number"), QStringLiteral("9999"));
    parser.addOption(portOption);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    app.setQuitOnLastWindowClosed(false);

    if (!KWindowSystem::isPlatformWayland()) {
        qCritical() << "Virtual Monitors are only supported on Wayland";
        return 1;
    }
    if (!parser.isSet(nameOption)) {
        qCritical() << "error: please define --name";
        return 2;
    } else {
        if (!parser.isSet(passwordOption)) {
            qCritical() << "error: please define --password";
            return 3;
        }
        if (!parser.isSet(resolutionOption)) {
            qCritical() << "error: please define --resolution";
            return 4;
        }
    }
    if (!parser.isSet(portOption)) {
        qCritical() << "error: please define --port";
        return 5;
    }
    const QString res = parser.value(resolutionOption);
    const auto resSplit = res.split(QLatin1Char('x'));
    if (resSplit.size() != 2) {
        qCritical() << "error: the resolution should be formatted as WIDTHxHEIGHT (e.g. --resolution 1920x1080)";
        return 6;
    }

    if (parser.isSet(nameOption)) {
        RfbServerManager::s_pluginArgs = {
            {QStringLiteral("name"), parser.value(nameOption)},
            {QStringLiteral("resolution"), QSize(resSplit[0].toInt(), resSplit[1].toInt())},
            {QStringLiteral("scale"), parser.value(scaleOption).toDouble()},
        };
    }

    VirtualMonitorRfbServer server;
    server.setPasswordRequired(true);
    server.setListeningPort(parser.value(portOption).toInt());
    PendingVirtualMonitorRfbClient::password = parser.value(passwordOption).toUtf8();

    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGPIPE);
    sigprocmask(SIG_BLOCK, &sigs, nullptr);
    if (!server.start()) {
        return 1;
    }

    return app.exec();
}

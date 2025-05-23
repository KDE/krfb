/*
    SPDX-FileCopyrightText: 2001-2003 Tim Jansen <tim@tjansen.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mainwindow.h"
#include "trayicon.h"
#include "invitationsrfbserver.h"
#include "krfbconfig.h"
#include "krfb_version.h"
#include "krfbdebug.h"

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>
#include <KMessageBox>
#include <KWindowSystem>

#include <QPixmap>
#include <qwindowdefs.h>
#include <QtGui/private/qtx11extras_p.h>

#include <csignal>
#include <X11/extensions/XTest.h>
#include <QCommandLineParser>
#include <QCommandLineOption>


static bool checkX11Capabilities()
{
    int bp1, bp2, majorv, minorv;
    Bool r = XTestQueryExtension(QX11Info::display(), &bp1, &bp2,
                                 &majorv, &minorv);

    if ((!r) || (((majorv * 1000) + minorv) < 2002)) {
        KMessageBox::error(nullptr,
                           i18n("Your X11 Server does not support the required XTest extension "
                                "version 2.2. Sharing your desktop is not possible."),
                           i18n("Desktop Sharing Error"));
        return false;
    }

    return true;
}

static void checkOldX11PluginConfig() {
    if (KrfbConfig::preferredFrameBufferPlugin() == QStringLiteral("x11")) {
        qCDebug(KRFB) << "Detected deprecated configuration: preferredFrameBufferPlugin = x11";
        KConfigSkeletonItem *config_item = KrfbConfig::self()->findItem(
                    QStringLiteral("preferredFrameBufferPlugin"));
        if (config_item) {
            config_item->setProperty(QStringLiteral("xcb"));
            KrfbConfig::self()->save();
            qCDebug(KRFB) << "  Fixed preferredFrameBufferPlugin from x11 to xcb.";
        }
    }
}

static void checkWaylandPluginConfig()
{
    if (KrfbConfig::preferredFrameBufferPlugin() != QStringLiteral("pw")) {
        qWarning() << "Wayland: Detected invalid configuration: "
                    "preferredFrameBufferPlugin is not pipewire: "
                   << KrfbConfig::preferredFrameBufferPlugin();
        KConfigSkeletonItem *config_item = KrfbConfig::self()->findItem(
                    QStringLiteral("preferredFrameBufferPlugin"));
        if (config_item) {
            config_item->setProperty(QStringLiteral("pw"));
            KrfbConfig::self()->save();
            qCDebug(KRFB) << "Wayland: Fixed preferredFrameBufferPlugin to \"pw\".";
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("krfb");

    KAboutData aboutData(QStringLiteral("krfb"),
                         i18n("Desktop Sharing"),
                         QStringLiteral(KRFB_VERSION_STRING),
                         i18n("VNC-compatible server to share desktops"),
                         KAboutLicense::GPL,
                         i18n("(c) 2009-2010, Collabora Ltd.\n"
                               "(c) 2007, Alessandro Praduroux\n"
                               "(c) 2001-2003, Tim Jansen\n"
                               "(c) 2001, Johannes E. Schindelin\n"
                               "(c) 2000-2001, Const Kaplinsky\n"
                               "(c) 2000, Tridia Corporation\n"
                               "(c) 1999, AT&T Laboratories Boston\n"));
    aboutData.addAuthor(i18n("George Goldberg"),
                        i18n("Telepathy tubes support"),
                        QStringLiteral("george.goldberg@collabora.co.uk"));
    aboutData.addAuthor(i18n("George Kiagiadakis"),
                        QString(),
                        QStringLiteral("george.kiagiadakis@collabora.co.uk"));
    aboutData.addAuthor(i18n("Alessandro Praduroux"), i18n("KDE4 porting"), QStringLiteral("pradu@pradu.it"));
    aboutData.addAuthor(i18n("Tim Jansen"), i18n("Original author"), QStringLiteral("tim@tjansen.de"));
    aboutData.addCredit(i18n("Johannes E. Schindelin"),
                        i18n("libvncserver"));
    aboutData.addCredit(i18n("Const Kaplinsky"),
                        i18n("TightVNC encoder"));
    aboutData.addCredit(i18n("Tridia Corporation"),
                        i18n("ZLib encoder"));
    aboutData.addCredit(i18n("AT&T Laboratories Boston"),
                        i18n("original VNC encoders and "
                              "protocol design"));
    KAboutData::setApplicationData(aboutData);

    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("krfb")));

    KCrash::initialize();

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    const QCommandLineOption nodialogOption(QStringList{ QStringLiteral("nodialog") }, i18n("Do not show the invitations management dialog at startup"));
    parser.addOption(nodialogOption);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    KDBusService service(KDBusService::Unique, &app);

    app.setQuitOnLastWindowClosed(false);

    if (KWindowSystem::isPlatformX11()) {
        if (!checkX11Capabilities()) {
            return 1;
        }

        // upgrade the configuration
        checkOldX11PluginConfig();
    } else if (KWindowSystem::isPlatformWayland()) {
        // check that default plugin in Wayland is PipeWire
        checkWaylandPluginConfig();
    } else {
        KMessageBox::error(nullptr,
                           i18n("Desktop Sharing is not running under an X11 Server or Wayland.\n"
                                "Other display servers are currently not supported."),
                           i18n("Desktop Sharing Error"));
        return 1;
    }

    //init the core
    InvitationsRfbServer::init();

    //init the GUI
    MainWindow mainWindow;
    TrayIcon trayicon(&mainWindow);

    QObject::connect(&service, &KDBusService::activateRequested, &mainWindow, [&mainWindow](const QStringList & /*arguments*/, const QString & /*workdir*/) {
        if (!mainWindow.isVisible()) {
            mainWindow.setVisible(true);
        } else {
            KWindowSystem::updateStartupId(mainWindow.windowHandle());
            KWindowSystem::activateWindow(mainWindow.windowHandle());
        }
    });

    if (KrfbConfig::startMinimized()) {
      mainWindow.hide();
    } else if (app.isSessionRestored() && KMainWindow::canBeRestored(1)) {
        mainWindow.restore(1, false);
    } else if (!parser.isSet(nodialogOption)) {
        mainWindow.show();
    }

    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGPIPE);
    sigprocmask(SIG_BLOCK, &sigs, nullptr);

    return app.exec();
}

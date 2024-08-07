/*
    SPDX-FileCopyrightText: 2009-2010 Collabora Ltd <info@collabora.co.uk>
    SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>
    SPDX-FileContributor: George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RFBSERVERMANAGER_H
#define RFBSERVERMANAGER_H

#include "rfb.h"
#include "framebuffer.h"
#include <QObject>
#include <QVariantMap>

class RfbClient;
struct RfbServerManagerStatic;
class RfbServer;

class RfbServerManager : public QObject
{
    Q_OBJECT
public:
    static RfbServerManager *instance();

    QSharedPointer<FrameBuffer> framebuffer() const;
    static QVariantMap s_pluginArgs;
Q_SIGNALS:
    void clientConnected(RfbClient *cc);
    void clientDisconnected(RfbClient *cc);

private Q_SLOTS:
    void init();
    void updateFrameBuffer();
    void updateScreens();
    void cleanup();

private:
    void registerServer(RfbServer *server);
    void unregisterServer(RfbServer *server);

    rfbScreenInfoPtr newScreen();

    void addClient(RfbClient *cc);
    void removeClient(RfbClient *cc);

    RfbServerManager();
    ~RfbServerManager() override;
    Q_DISABLE_COPY(RfbServerManager)

    friend class RfbServer;
    friend struct RfbServerManagerStatic;

    struct Private;
    Private *const d;
};

#endif // RFBSERVERMANAGER_H

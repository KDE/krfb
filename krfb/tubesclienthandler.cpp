/* This file is part of the KDE project
   Copyright (C) 2009 Collabora Ltd <info@collabora.co.uk>
    @author George Goldberg <george.goldberg@collabora.co.uk>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "tubesclienthandler.h"
#include "tubesrfbserver.h"

#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Debug>

#include <KDebug>

using namespace Tp;

static inline Tp::ChannelClassList channelClassList()
{
    QMap<QString, QDBusVariant> filter0;
    filter0[TELEPATHY_INTERFACE_CHANNEL ".ChannelType"] =
            QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAM_TUBE);
    filter0[TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAM_TUBE ".Service"] = QDBusVariant("rfb");
    filter0[TELEPATHY_INTERFACE_CHANNEL ".Requested"] = QDBusVariant(true);

    return Tp::ChannelClassList() << Tp::ChannelClass(filter0);
}

TubesClientHandler::TubesClientHandler()
  : AbstractClientHandler(channelClassList(), false)
{
    kDebug();

    Tp::enableDebug(false);
    Tp::enableWarnings(true);

    /* Registering telepathy types */
    registerTypes();
}

TubesClientHandler::~TubesClientHandler()
{
    kDebug();
}

bool TubesClientHandler::bypassApproval() const
{
    // Don't bypass approval of channels.
    return false;
}

void TubesClientHandler::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                                        const Tp::AccountPtr &account,
                                        const Tp::ConnectionPtr &connection,
                                        const QList<Tp::ChannelPtr> &channels,
                                        const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                                        const QDateTime &userActionTime,
                                        const QVariantMap &handlerInfo)
{
    kDebug();

    Q_UNUSED(account);
    Q_UNUSED(connection);
    Q_UNUSED(requestsSatisfied);
    Q_UNUSED(userActionTime);
    Q_UNUSED(handlerInfo);

    foreach(const Tp::ChannelPtr &channel, channels) {
        kDebug() << "Incoming channel: " << channel;

        QVariantMap properties = channel->immutableProperties();

        if (properties[TELEPATHY_INTERFACE_CHANNEL ".ChannelType"] ==
               TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAM_TUBE) {

            kDebug() << "Channel is a stream tube. Handling: " << channel;
            new TubesRfbServer(channel);
        }
    }
    context->setFinished();
}


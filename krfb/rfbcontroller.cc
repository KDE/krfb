/***************************************************************************
                              rfbcontroller.cpp
                             -------------------
    begin                : Sun Dec 9 2001
    copyright            : (C) 2001 by Tim Jansen
    email                : tim@tjansen.de
 ***************************************************************************/

/***************************************************************************
 * Contains portions & concept from rfb's x0rfbserver.cc
 * Copyright (C) 2000 heXoNet Support GmbH, D-66424 Homburg.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rfbcontroller.h"

#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>

#include <kdebug.h>
#include <qwindowdefs.h>
#include <qtimer.h>
#include <qglobal.h>

RFBController::RFBController(Configuration *c) :
	configuration(c),
	idleUpdateScheduled(false)
{
	start();
}

RFBController::~RFBController() {
	delete serversocket;
	delete connection;
	delete socket;
}

void RFBController::start() {
	serversocket = new KServerSocket(configuration->port(), false);
	connect(serversocket, SIGNAL(accepted()), SLOT(accepted()));
	serversocket->bindAndListen();
	// TODO: error message if bindAndListen fails (port in use)
}

void RFBController::rebind() {
	if (serversocket) {
		delete serversocket;
		start();
	}
}

void RFBController::idleSlot() {
	idleUpdateScheduled = false;
	if (connection) {
		connection->sendIncrementalFramebufferUpdate();
		checkWritable();
	}
}

// called when KServerSocket accepted a connection. Closes KServerSocket.
void RFBController::accepted() {
	int sockFd = serversocket->socket();
	KSocket *s;
	if (sockFd < 0)
		kdDebug("Negative server socket?");

	int one = 1;
	setsockopt(sockFd, IPPROTO_TCP, TCP_NODELAY, 
		   (char *)&one, sizeof(one));
	fcntl(sockFd, F_SETFL, O_NONBLOCK);
	s = new KSocket(sockFd);

	// TODO: ASK USER FOR PERMISSION HERE BEFORE GOING ON

	delete serversocket;
	serversocket = 0;

	socket = s;
	connect(s, SIGNAL(readEvent(KSocket*)), SLOT(sockedReadable()));
	connect(s, SIGNAL(writeEvent(KSocket*)), SLOT(sockedWritable()));
	connect(s, SIGNAL(closeEvent(KSocket*)), SLOT(sockedClosed()));
	s->enableRead(true);
	connection = new RFBConnection(qt_xdisplay(), sockFd, 
				       configuration->password());
	checkWritable();
	emit sessionEstablished();
}

void RFBController::checkWritable() {
	BufferedConnection *bc = connection->bufferedConnection;	
	socket->enableWrite((bc->senderBuffer.end - bc->senderBuffer.pos) > 0);
}

void RFBController::prepareIdleUpdate() {
	if (!idleUpdateScheduled)
		QTimer::singleShot(0, this, SLOT(idleSlot()));
	idleUpdateScheduled = true;
}

void RFBController::socketReadable() {
	ASSERT(socket);
	int fd = socket->socket();
	BufferedConnection *bc = connection->bufferedConnection;
	prepareIdleUpdate();
	int count = read(fd,
			 bc->receiverBuffer.data,
                         bc->receiverBuffer.size);
	if (count >= 0)
		bc->receiverBuffer.end += count;
	else {
		// TODO: what to do if write failed
	}
	while (connection->currentState && bc->hasReceiverBufferData()) {
		connection->update();
		checkWritable();
	}
	bc->receiverBuffer.pos = 0;
	bc->receiverBuffer.end = 0;

	if (!connection->currentState)
		closeSession();
}

void RFBController::socketWritable() {
	ASSERT(socket);
	int fd = socket->socket();
	BufferedConnection *bc = connection->bufferedConnection;
	ASSERT((bc->senderBuffer.end - bc->senderBuffer.pos) > 0);
	// TODO: what to do if fd < 0?
	prepareIdleUpdate();
	int count = write(fd,
			  bc->senderBuffer.data + bc->senderBuffer.pos,
			  bc->senderBuffer.end - bc->senderBuffer.pos);
	if (count >= 0)
		bc->senderBuffer.pos += count;
	else {
		// TODO: what to do if write failed
	}
	checkWritable();
}

void RFBController::closeSession() {
	if (!connection)
		return;
	delete connection;
	delete socket;
	connection = 0;
	socket = 0;
	emit sessionFinished();
	start();
}

#include "rfbcontroller.moc"

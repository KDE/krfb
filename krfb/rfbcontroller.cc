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
	socket(0),
	connection(0),
	idleUpdateScheduled(false)
{
	startServer();
}

RFBController::~RFBController() {
	delete serversocket;
	if (socket)
		delete socket;
	if (connection) 
		delete connection;
}

void RFBController::startServer() {
	serversocket = new KServerSocket(configuration->port(), false);
	connect(serversocket, SIGNAL(accepted(KSocket*)), SLOT(accepted(KSocket*)));
	serversocket->bindAndListen();
	// TODO: error message if bindAndListen fails (port in use)
}

void RFBController::rebind() {
	delete serversocket;
	startServer();
}

/* called when KServerSocket accepted a connection.
   Refuses the connection if there is already a connection.
 */
void RFBController::accepted(KSocket *s) {
	int sockFd = s->socket();
	if (sockFd < 0)
		kdError() << "Got negative socket (error)" << endl;

	if (socket) {
		kdWarning() << "refuse 2nd connection" << endl;
		delete s;
		return;
	}

	int one = 1;
	setsockopt(sockFd, IPPROTO_TCP, TCP_NODELAY, 
		   (char *)&one, sizeof(one));
	fcntl(sockFd, F_SETFL, O_NONBLOCK);
	socket = s;

	// TODO: ASK USER FOR PERMISSION HERE BEFORE GOING ON
	// TODO: also handle case if remote closes while dialog is shown

	connect(s, SIGNAL(readEvent(KSocket*)), SLOT(socketReadable()));
	connect(s, SIGNAL(writeEvent(KSocket*)), SLOT(socketWritable()));
	connect(s, SIGNAL(closeEvent(KSocket*)), SLOT(closeSession()));
	s->enableRead(true);
	s->enableWrite(true);
	connection = new RFBConnection(qt_xdisplay(), sockFd, 
				       configuration->password(),
				       configuration->allowDesktopControl());

	emit sessionEstablished();
}

void RFBController::checkWriteBuffer() {
	BufferedConnection *bc = connection->bufferedConnection;	
	socket->enableWrite((bc->senderBuffer.end - bc->senderBuffer.pos) > 0);
}

void RFBController::idleSlot() {
kdDebug() << "Idle 1" << endl;
	idleUpdateScheduled = false;
	if (connection) {
kdDebug() << "Idle 2" << endl;
                connection->scanUpdates();
		connection->sendIncrementalFramebufferUpdate();
		checkWriteBuffer();
	}
}

void RFBController::prepareIdleUpdate() {
	kdDebug() << "test schedule, isScheduled? " << idleUpdateScheduled<< endl;
	BufferedConnection *bc = connection->bufferedConnection;	
	if (((bc->senderBuffer.end - bc->senderBuffer.pos) == 0) && 
	    !idleUpdateScheduled) {
		QTimer::singleShot(0, this, SLOT(idleSlot()));
		idleUpdateScheduled = true;
kdDebug() << "Scheduled!" << endl;
	}
}

void RFBController::socketReadable() {
	ASSERT(socket);
	int fd = socket->socket();
	BufferedConnection *bc = connection->bufferedConnection;
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
		checkWriteBuffer();
	}
	bc->receiverBuffer.pos = 0;
	bc->receiverBuffer.end = 0;

	if (!connection->currentState)
		closeSession();
}

void RFBController::socketWritable() {
	ASSERT(socket);
kdDebug() << "write slot" << endl;
	int fd = socket->socket();
	BufferedConnection *bc = connection->bufferedConnection;
	ASSERT((bc->senderBuffer.end - bc->senderBuffer.pos) > 0);
	// TODO: what to do if fd < 0?
	int count = write(fd,
			  bc->senderBuffer.data + bc->senderBuffer.pos,
			  bc->senderBuffer.end - bc->senderBuffer.pos);
	if (count >= 0) {
		bc->senderBuffer.pos += count;
	} else {
		// TODO: what to do if write failed
	}
kdDebug() << "written " << count << " left " << (bc->senderBuffer.end - bc->senderBuffer.pos) << endl;
	prepareIdleUpdate();
	checkWriteBuffer();
}

void RFBController::closeSession() {
	if (!socket)
		return;
	if (connection) {
		delete connection;
		connection = 0;
		emit sessionFinished();
	}
	delete socket;
	socket = 0;
}

#include "rfbcontroller.moc"

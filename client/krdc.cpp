/***************************************************************************
                          krdc.cpp  -  main window
                             -------------------
    begin                : Tue May 13 23:07:42 CET 2002
    copyright            : (C) 2002 by Tim Jansen
    email                : tim@tjansen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "newconnectiondialog.h"
#include "krdc.h"
#include <kdebug.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <qlayout.h> 



KRDC::KRDC(QWidget *w, const QString &host, Quality q) : 
  QWidget(w, 0),
  m_view(0),
  m_host(host),
  m_quality(q)
{
	m_scrollView = new QScrollView(this, "Remote View");
	QVBoxLayout *vbl = new QVBoxLayout(this);
	vbl->addWidget(m_scrollView);
}

bool KRDC::start()
{
	KConfig *config = KApplication::kApplication()->config();
	QString vncServerHost;
	int vncServerPort = 5900;

	if (!m_host.isNull()) {
		parseHost(m_host, vncServerHost, vncServerPort);
		if (m_quality == QUALITY_UNKNOWN)
			m_quality = QUALITY_HIGH;
	} else {
		NewConnectionDialog ncd(0, 0, true);
		QStringList list = config->readListEntry("serverCompletions");
		ncd.serverInput->completionObject()->setItems(list);
		list = config->readListEntry("serverHistory");
		ncd.serverInput->setHistoryItems(list);

		if ((ncd.exec() == QDialog::Rejected) ||
		    (ncd.serverInput->currentText().length() == 0)) {
			return false;
		}
		QString host = ncd.serverInput->currentText();
		parseHost(host, vncServerHost, vncServerPort);
		int ci = ncd.qualityCombo->currentItem();
		if (ci == 0) 
			m_quality = QUALITY_HIGH;
		else if (ci == 1)
			m_quality = QUALITY_MEDIUM;
		else if (ci == 2)
			m_quality = QUALITY_LOW;
		else {
			kdDebug() << "Unknown quality";
			return false;
		}
			
		ncd.serverInput->addToHistory(host);
		list = ncd.serverInput->completionObject()->items();
		config->writeEntry("serverCompletions", list);
		list = ncd.serverInput->historyItems();
		config->writeEntry("serverHistory", list);
	}

	configureApp(m_quality);

	setFixedSize(640, 480);
	m_view = new KVncView(m_scrollView, 0, vncServerHost, vncServerPort, 
			      &m_appData);
	m_scrollView->addChild(m_view);
	connect(m_view, SIGNAL(changeSize(int,int)), SLOT(setSize(int,int)));
	show();
	m_view->start();
	return true;
}

void KRDC::configureApp(Quality q) {
	m_appData.shareDesktop = True;
	m_appData.viewOnly = False;

	if (q == QUALITY_LOW) {
		m_appData.useBGR233 = True;
		m_appData.encodingsString = "copyrect tight zlib hextile corre rre raw";
		m_appData.compressLevel = -1;
		m_appData.qualityLevel = 1;
	}
	else if (q == QUALITY_MEDIUM) {
		m_appData.useBGR233 = False;
		m_appData.encodingsString = "copyrect tight zlib hextile corre rre raw";
		m_appData.compressLevel = -1;
		m_appData.qualityLevel = 4;
	}
	else if ((q == QUALITY_HIGH) || (q == QUALITY_UNKNOWN)) {
		m_appData.useBGR233 = False;
		m_appData.encodingsString = "copyrect hextile corre rre raw";
		m_appData.compressLevel = -1;
		m_appData.qualityLevel = 9;
	}

	m_appData.nColours = 256;
	m_appData.useSharedColours = True;
	m_appData.requestedDepth = 0;
	m_appData.useRemoteCursor = True;

	m_appData.rawDelay = 0;
	m_appData.copyRectDelay = 0;
}

void KRDC::parseHost(QString &s, QString &serverHost, int &serverPort) {
	QString host = s;
	int pos = s.find(':');
	if (pos < 0) {
		s+= ":0";
		host+= ":0";
		pos = s.find(':');
	}

	bool portOk = false;
	QString portS = s.mid(pos+1);
	int port = portS.toInt(&portOk);
	if (portOk) {
		host = s.left(pos);
		if (port < 100)
			serverPort = port + 5900;
		else
			serverPort = port;
	}
	
	serverHost = host;
}

KRDC::~KRDC()
{
}

void KRDC::setSize(int w, int h)
{
	int newW, newH, dw, dh, fx, fy;
	setMaximumSize(w, h);
	QWidget *desktop = QApplication::desktop();
	// todo: switch to fullscreen if desktop is too small
	dw = desktop->width();
	dh = desktop->height();
	newW = (w > dw) ? dw : w;
	newH = (h > dh) ? dh : h;

	QRect g = geometry();
	QRect f = frameGeometry();
	fx = g.x() - f.x();
	fy = g.y() - f.y();

	g.setWidth(newW);
	g.setHeight(newH);
	if (g.right() > dw)
		g.setX(dw - g.width());
	if (g.x() < fx)
		g.setX(fx);
	if (g.bottom() > dh)
		g.setY(dh - g.height());
	if (g.y() < fy)
		g.setY(fy);
	setGeometry(g);
}


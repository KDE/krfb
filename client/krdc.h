/***************************************************************************
                            krdc.h  -  main window
                              -------------------
    begin                : Tue May 13 23:10:42 CET 2002
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

#ifndef KRDC_H
#define KRDC_H

#include <qscrollview.h>
#include "kvncview.h"


class KRDC : public QWidget
{
	Q_OBJECT 
private:
	QScrollView *m_scrollView;
	KVncView *m_view; 
	QString m_host;
	Quality m_quality;
	AppData m_appData;

	void configureApp(Quality q);
	void parseHost(QString &s, QString &serverHost, int &serverPort);

protected:

public:
	KRDC(QWidget *parent = 0, 
	     const QString &host = QString::null, 
	     Quality q = QUALITY_UNKNOWN);
	~KRDC();
	bool start();

private slots:
	void setSize(int w, int h);
	
};

#endif

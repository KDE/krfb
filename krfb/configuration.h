/***************************************************************************
                               configuration.h
                             -------------------
    begin                : Tue Dec 11 2001
    copyright            : (C) 2001-2002 by Tim Jansen
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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "configurationdialog.h"

#include <qobject.h>
#include <qvalidator.h>
#include <qstring.h>

class ConfigurationDialog2 : public ConfigurationDialog {
	Q_OBJECT
public:
	virtual void closeEvent(QCloseEvent *);
signals:
	void closed();
};

/**
 * This class stores the app's configuration
 * @author Tim Jansen
 */
class Configuration : public QObject {
   	Q_OBJECT
public:
	Configuration();
	Configuration(bool oneConnection, bool askOnConnect,
		      bool allowDesktopControl, QString password);
	~Configuration();

	bool preconfigured() const;
	bool oneConnection() const;
	bool askOnConnect() const;
	bool allowDesktopControl() const;
	QString password() const;

        void setOnceConnection(bool oneConnection);
        void setAskOnConnect(bool askOnConnect);
        void setAllowDesktopControl(bool allowDesktopControl);
	void setPassword(QString password);
	void reload();

signals:
  	void passwordChanged();

public slots:
	void showDialog();

private:
        void loadFromKConfig();
        void loadFromDialog();
        void saveToKConfig();
        void saveToDialog();

        ConfigurationDialog2 confDlg;
	QIntValidator *portValidator;

	bool preconfiguredFlag;
	bool askOnConnectFlag;
	bool allowDesktopControlFlag;
	bool oneConnectionFlag;
	QString passwordString;

private slots:
	void okPressed();
	void cancelPressed();
	void applyPressed();
};

#endif

#ifndef __KRFB_IFACE_IMPL_H
#define __KRFB_IFACE_IMPL_H

#include <qobject.h>
#include "configuration.h"
#include "krfbiface.h"

class KRfbIfaceImpl : public QObject, public virtual krfbIface
{
	Q_OBJECT
private:
	Configuration *configuration;
	int portNum;
public:
	KRfbIfaceImpl(Configuration *c);
signals:
	void connectionClosed();
	void exitApp();

public slots:
	void setPort(int p);

public:
	void disconnect();
//	void setWindowID(int);
	void exit();

	bool oneConnection();
	void setOneConnection(bool);
	bool askOnConnect();
	void setAskOnConnect(bool);
	bool allowDesktopControl();
	void setAllowDesktopControl(bool);
	void setPassword(QString);
	int port();
};
#endif

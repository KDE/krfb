#ifndef __KRFB_IFACE_H
#define __KRFB_IFACE_H

#include <dcopobject.h>

class krfbIface : virtual public DCOPObject
{
        K_DCOP
k_dcop:
	virtual void disconnect() = 0;
//	virtual void setWindowID(int) = 0;
	virtual void exit() = 0;

	virtual bool oneConnection() = 0;
	virtual void setOneConnection(bool) = 0;
	virtual bool askOnConnect() = 0;
	virtual void setAskOnConnect(bool) = 0;
	virtual bool allowDesktopControl() = 0;
	virtual void setAllowDesktopControl(bool) = 0;
	virtual void setPassword(QString) = 0;
	virtual int port() = 0;
};
#endif

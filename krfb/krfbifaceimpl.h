#ifndef __KRFB_IFACE_IMPL_H
#define __KRFB_IFACE_IMPL_H

#include <qobject.h>
#include "rfbcontroller.h"
#include "krfbiface.h"

class KRfbIfaceImpl : public QObject, public virtual krfbIface
{
	Q_OBJECT
private:
	RFBController *controller;
public:
	KRfbIfaceImpl(RFBController *c);
signals:
	void exitApp();

public:
	void exit();
	void setAllowDesktopControl(bool);
};
#endif

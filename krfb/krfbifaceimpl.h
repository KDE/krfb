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
public:
	KRfbIfaceImpl(Configuration *c);
signals:
	void exitApp();

public:
	void exit();
	void setAllowDesktopControl(bool);
};
#endif

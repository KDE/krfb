#ifndef __KRFB_IFACE_H
#define __KRFB_IFACE_H

#include <dcopobject.h>

class krfbIface : virtual public DCOPObject
{
        K_DCOP
k_dcop:

	/**
	 * If a client is connected it will be disconnected.
	 */
	virtual void disconnect() = 0;
//	virtual void setWindowID(int) = 0;

	/**
	 * Quites krfb, connected clients will be disconnected.
	 */
	virtual void exit() = 0;

	/**
	 * If true krfb will disconnect after a client disconnected.
	 * @return true if oneConnection feature is turned on
	 */
	virtual bool oneConnection() = 0;

	/**
	 * If set to true krfb will disconnect after a client disconnected.
	 * @param o true to turn oneConnection feature on.
	 */
	virtual void setOneConnection(bool o) = 0;

	/**
	 * If this feature is activated krfb will ask the user before an incomning
	 * connection will be accepted.
	 * @return true if askOnConnect is activated
	 */
	virtual bool askOnConnect() = 0;

	/**
	 * If this feature is activated krfb will ask the user before an incomning
	 * connection will be accepted.
	 * @param a true to turn the askOnConnect feature on.
	 */
	virtual void setAskOnConnect(bool a) = 0;

	/**
	 * If this feature is activated krfb allows the connecting client to
	 * control the desktop (pointer & keyboard).
	 * @return true if desktop control is activated
	 */
	virtual bool allowDesktopControl() = 0;

	/**
	 * If this feature is activated krfb allows the connecting client to
	 * control the desktop (pointer & keyboard).
	 * @return a true to activate desktop control
	 */
	virtual void setAllowDesktopControl(bool a) = 0;
	
	/**
	 * Sets the default password. An empty password can be used to deactivate
	 * the password authentication, but only if krfb is in stand-alone mode.
	 * @param p the password to set
	 */
	virtual void setPassword(QString p) = 0;
	
	/**
	 * Returns the port the server is listening on.
	 * @return the tcp port of the server
	 */
	virtual int port() = 0;
};
#endif

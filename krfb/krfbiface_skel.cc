/****************************************************************************
**
** DCOP Skeleton created by dcopidl2cpp from krfbiface.kidl
**
** WARNING! All changes made in this file will be lost!
**
*****************************************************************************/

#include "./krfbiface.h"

#include <kdatastream.h>


static const char* const krfbIface_ftable[3][3] = {
    { "void", "exit()", "exit()" },
    { "void", "setAllowDesktopControl(bool)", "setAllowDesktopControl(bool a)" },
    { 0, 0, 0 }
};

bool krfbIface::process(const QCString &fun, const QByteArray &data, QCString& replyType, QByteArray &replyData)
{
    if ( fun == krfbIface_ftable[0][1] ) { // void exit()
	replyType = krfbIface_ftable[0][0]; 
	exit( );
    } else if ( fun == krfbIface_ftable[1][1] ) { // void setAllowDesktopControl(bool)
	bool arg0;
	QDataStream arg( data, IO_ReadOnly );
	arg >> arg0;
	replyType = krfbIface_ftable[1][0]; 
	setAllowDesktopControl(arg0 );
    } else {
	return DCOPObject::process( fun, data, replyType, replyData );
    }
    return TRUE;
}

QCStringList krfbIface::interfaces()
{
    QCStringList ifaces = DCOPObject::interfaces();
    ifaces += "krfbIface";
    return ifaces;
}

QCStringList krfbIface::functions()
{
    QCStringList funcs = DCOPObject::functions();
    for ( int i = 0; krfbIface_ftable[i][2]; i++ ) {
	QCString func = krfbIface_ftable[i][0];
	func += ' ';
	func += krfbIface_ftable[i][2];
	funcs << func;
    }
    return funcs;
}



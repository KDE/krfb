/****************************************************************************
**
** DCOP Skeleton created by dcopidl2cpp from krfbiface.kidl
**
** WARNING! All changes made in this file will be lost!
**
*****************************************************************************/

#include "./krfbiface.h"

#include <kdatastream.h>
#include <qasciidict.h>


static const int krfbIface_fhash = 13;
static const char* const krfbIface_ftable[12][3] = {
    { "void", "disconnect()", "disconnect()" },
    { "void", "exit()", "exit()" },
    { "void", "reloadConfig()", "reloadConfig()" },
    { "bool", "oneConnection()", "oneConnection()" },
    { "void", "setOneConnection(bool)", "setOneConnection(bool o)" },
    { "bool", "askOnConnect()", "askOnConnect()" },
    { "void", "setAskOnConnect(bool)", "setAskOnConnect(bool a)" },
    { "bool", "allowDesktopControl()", "allowDesktopControl()" },
    { "void", "setAllowDesktopControl(bool)", "setAllowDesktopControl(bool a)" },
    { "void", "setPassword(QString)", "setPassword(QString p)" },
    { "int", "port()", "port()" },
    { 0, 0, 0 }
};

bool krfbIface::process(const QCString &fun, const QByteArray &data, QCString& replyType, QByteArray &replyData)
{
    static QAsciiDict<int>* fdict = 0;
    if ( !fdict ) {
	fdict = new QAsciiDict<int>( krfbIface_fhash, TRUE, FALSE );
	for ( int i = 0; krfbIface_ftable[i][1]; i++ )
	    fdict->insert( krfbIface_ftable[i][1],  new int( i ) );
    }
    int* fp = fdict->find( fun );
    switch ( fp?*fp:-1) {
    case 0: { // void disconnect()
	replyType = krfbIface_ftable[0][0]; 
	disconnect( );
    } break;
    case 1: { // void exit()
	replyType = krfbIface_ftable[1][0]; 
	exit( );
    } break;
    case 2: { // void reloadConfig()
	replyType = krfbIface_ftable[2][0]; 
	reloadConfig( );
    } break;
    case 3: { // bool oneConnection()
	replyType = krfbIface_ftable[3][0]; 
	QDataStream _replyStream( replyData, IO_WriteOnly );
	_replyStream << oneConnection( );
    } break;
    case 4: { // void setOneConnection(bool)
	bool arg0;
	QDataStream arg( data, IO_ReadOnly );
	arg >> arg0;
	replyType = krfbIface_ftable[4][0]; 
	setOneConnection(arg0 );
    } break;
    case 5: { // bool askOnConnect()
	replyType = krfbIface_ftable[5][0]; 
	QDataStream _replyStream( replyData, IO_WriteOnly );
	_replyStream << askOnConnect( );
    } break;
    case 6: { // void setAskOnConnect(bool)
	bool arg0;
	QDataStream arg( data, IO_ReadOnly );
	arg >> arg0;
	replyType = krfbIface_ftable[6][0]; 
	setAskOnConnect(arg0 );
    } break;
    case 7: { // bool allowDesktopControl()
	replyType = krfbIface_ftable[7][0]; 
	QDataStream _replyStream( replyData, IO_WriteOnly );
	_replyStream << allowDesktopControl( );
    } break;
    case 8: { // void setAllowDesktopControl(bool)
	bool arg0;
	QDataStream arg( data, IO_ReadOnly );
	arg >> arg0;
	replyType = krfbIface_ftable[8][0]; 
	setAllowDesktopControl(arg0 );
    } break;
    case 9: { // void setPassword(QString)
	QString arg0;
	QDataStream arg( data, IO_ReadOnly );
	arg >> arg0;
	replyType = krfbIface_ftable[9][0]; 
	setPassword(arg0 );
    } break;
    case 10: { // int port()
	replyType = krfbIface_ftable[10][0]; 
	QDataStream _replyStream( replyData, IO_WriteOnly );
	_replyStream << port( );
    } break;
    default: 
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



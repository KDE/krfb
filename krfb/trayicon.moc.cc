/****************************************************************************
** TrayIcon meta object code from reading C++ file 'trayicon.h'
**
** Created: Fri Mar 29 17:12:44 2002
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "trayicon.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 19)
#error "This file was generated using the moc from 3.0.2-snapshot-20020126. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *TrayIcon::className() const
{
    return "TrayIcon";
}

QMetaObject *TrayIcon::metaObj = 0;
static QMetaObjectCleanUp cleanUp_TrayIcon;

#ifndef QT_NO_TRANSLATION
QString TrayIcon::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "TrayIcon", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString TrayIcon::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "TrayIcon", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* TrayIcon::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = KSystemTray::staticMetaObject();
    static const QUMethod slot_0 = {"closeConnection", 0, 0 };
    static const QUMethod slot_1 = {"openConnection", 0, 0 };
    static const QUMethod slot_2 = {"showAbout", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "closeConnection()", &slot_0, QMetaData::Public },
	{ "openConnection()", &slot_1, QMetaData::Public },
	{ "showAbout()", &slot_2, QMetaData::Private }
    };
    static const QUMethod signal_0 = {"connectionClosed", 0, 0 };
    static const QUMethod signal_1 = {"showConfigure", 0, 0 };
    static const QUMethod signal_2 = {"showManageInvitations", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "connectionClosed()", &signal_0, QMetaData::Public },
	{ "showConfigure()", &signal_1, QMetaData::Public },
	{ "showManageInvitations()", &signal_2, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"TrayIcon", parentObject,
	slot_tbl, 3,
	signal_tbl, 3,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_TrayIcon.setMetaObject( metaObj );
    return metaObj;
}

void* TrayIcon::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "TrayIcon" ) ) return (TrayIcon*)this;
    return KSystemTray::qt_cast( clname );
}

// SIGNAL connectionClosed
void TrayIcon::connectionClosed()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

// SIGNAL showConfigure
void TrayIcon::showConfigure()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}

// SIGNAL showManageInvitations
void TrayIcon::showManageInvitations()
{
    activate_signal( staticMetaObject()->signalOffset() + 2 );
}

bool TrayIcon::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: closeConnection(); break;
    case 1: openConnection(); break;
    case 2: showAbout(); break;
    default:
	return KSystemTray::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool TrayIcon::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: connectionClosed(); break;
    case 1: showConfigure(); break;
    case 2: showManageInvitations(); break;
    default:
	return KSystemTray::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool TrayIcon::qt_property( int _id, int _f, QVariant* _v)
{
    return KSystemTray::qt_property( _id, _f, _v);
}
#endif // QT_NO_PROPERTIES

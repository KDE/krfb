/****************************************************************************
** KRfbIfaceImpl meta object code from reading C++ file 'krfbifaceimpl.h'
**
** Created: Sun Mar 17 19:35:52 2002
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "krfbifaceimpl.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 19)
#error "This file was generated using the moc from 3.0.2-snapshot-20020126. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *KRfbIfaceImpl::className() const
{
    return "KRfbIfaceImpl";
}

QMetaObject *KRfbIfaceImpl::metaObj = 0;
static QMetaObjectCleanUp cleanUp_KRfbIfaceImpl;

#ifndef QT_NO_TRANSLATION
QString KRfbIfaceImpl::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KRfbIfaceImpl", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString KRfbIfaceImpl::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KRfbIfaceImpl", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* KRfbIfaceImpl::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QObject::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "p", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"setPort", 1, param_slot_0 };
    static const QMetaData slot_tbl[] = {
	{ "setPort(int)", &slot_0, QMetaData::Public }
    };
    static const QUMethod signal_0 = {"connectionClosed", 0, 0 };
    static const QUMethod signal_1 = {"exitApp", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "connectionClosed()", &signal_0, QMetaData::Public },
	{ "exitApp()", &signal_1, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"KRfbIfaceImpl", parentObject,
	slot_tbl, 1,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_KRfbIfaceImpl.setMetaObject( metaObj );
    return metaObj;
}

void* KRfbIfaceImpl::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "KRfbIfaceImpl" ) ) return (KRfbIfaceImpl*)this;
    if ( !qstrcmp( clname, "krfbIface" ) ) return (krfbIface*)this;
    return QObject::qt_cast( clname );
}

// SIGNAL connectionClosed
void KRfbIfaceImpl::connectionClosed()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

// SIGNAL exitApp
void KRfbIfaceImpl::exitApp()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}

bool KRfbIfaceImpl::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setPort(static_QUType_int.get(_o+1)); break;
    default:
	return QObject::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool KRfbIfaceImpl::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: connectionClosed(); break;
    case 1: exitApp(); break;
    default:
	return QObject::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool KRfbIfaceImpl::qt_property( int _id, int _f, QVariant* _v)
{
    return QObject::qt_property( _id, _f, _v);
}
#endif // QT_NO_PROPERTIES

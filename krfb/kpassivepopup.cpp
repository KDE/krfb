/*
 *   copyright            : (C) 2001-2002 by Richard Moore
 *   email                : rich@kde.org
 */

#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qvbox.h>

#include <kdebug.h>
#include <kdialog.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kglobalsettings.h>
#include <netwm.h>

#include "kpassivepopup.h"
#include "kpassivepopup.moc"

static const int DEFAULT_POPUP_TIME = 6*1000;
static const int POPUP_FLAGS = Qt::WStyle_Customize | Qt::WDestructiveClose | Qt::WX11BypassWM
                             | Qt::WStyle_StaysOnTop | Qt::WStyle_Tool | Qt::WStyle_NoBorder;


KPassivePopup::KPassivePopup( QWidget *parent, const char *name, WFlags f )
    : QFrame( 0, name, f | POPUP_FLAGS ),
      window( parent->winId() ), msgView( 0 ), layout( 0 ),
      hideDelay( DEFAULT_POPUP_TIME ), hideTimer( new QTimer( this, "hide_timer" ) ), m_autoDelete( false ), d( 0 )
{
    init();
}

KPassivePopup::KPassivePopup( WId win, const char *name, WFlags f )
    : QFrame( 0, name, f | POPUP_FLAGS ),
      window( win ), msgView( 0 ), layout( 0 ),
      hideDelay( DEFAULT_POPUP_TIME ), hideTimer( new QTimer( this, "hide_timer" ) ), m_autoDelete( false ), d( 0 )
{
    init();
}

void KPassivePopup::init()
{
    setFrameStyle( QFrame::Box| QFrame::Plain );
    setLineWidth( 2 );
    connect( hideTimer, SIGNAL( timeout() ), SLOT( hide() ) );
    connect( this, SIGNAL( clicked() ), SLOT( hide() ) );
}

KPassivePopup::~KPassivePopup()
{
}

void KPassivePopup::setView( QWidget *child )
{
    delete msgView;
    msgView = child;
    
    delete layout;
    layout = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );
    layout->addWidget( msgView );
    layout->activate();
}

void KPassivePopup::setView( const QString &caption, const QString &text,
                             const QPixmap &icon )
{
    // kdDebug() << "KPassivePopup::setView " << caption << ", " << text << endl;
    QVBox *vb = new QVBox( this );
    vb->setSpacing( KDialog::spacingHint() );

    QHBox *hb=0;
    if ( !icon.isNull() ) {
	hb = new QHBox( vb );
	hb->setMargin( KDialog::marginHint() );
	hb->setSpacing( KDialog::spacingHint() );
	ttlIcon = new QLabel( hb, "title_icon" );
	ttlIcon->setPixmap( icon );
    }

    if ( !caption.isNull() ) {
	ttl = new QLabel( caption, hb ? hb : vb, "title_label" );
	QFont fnt = ttl->font();
	fnt.setBold( true );
	ttl->setFont( fnt );
	ttl->setAlignment( Qt::AlignHCenter );
    }

    msg = new QLabel( text, vb, "msg_label" );

    setView( vb );
}

void KPassivePopup::setView( const QString &caption, const QString &text )
{
    setView( caption, text, QPixmap() );
}

void KPassivePopup::setTimeout( int delay )
{
    hideDelay = delay;
	if( hideTimer->isActive() )
		hideTimer->changeInterval( delay );
}

void KPassivePopup::setAutoDelete( bool autoDelete )
{
    m_autoDelete = autoDelete;
}

void KPassivePopup::mouseReleaseEvent( QMouseEvent *e )
{
    emit clicked();
    emit clicked( e->pos() );
}

//
// Main Implementation
//

void KPassivePopup::show()
{
    if ( size() != sizeHint() )
	resize( sizeHint() );

    positionSelf();
    QFrame::show();

    int delay = hideDelay;
    if ( delay < 0 )
	delay = DEFAULT_POPUP_TIME;

    if ( delay > 0 ) {
	hideTimer->start( delay );
    }
}

void KPassivePopup::hideEvent( QHideEvent * )
{
    hideTimer->stop();
    if ( m_autoDelete )
        deleteLater();
}

void KPassivePopup::positionSelf()
{
    NETWinInfo ni( qt_xdisplay(), window, qt_xrootwin(),
		   NET::WMIconGeometry | NET::WMKDESystemTrayWinFor );

    // Figure out where to put the popup. Note that we must handle
    // windows that skip the taskbar cleanly
    QRect target;
    if ( ni.kdeSystemTrayWinFor() ) {
	NETRect frame, win;
	ni.kdeGeometry( frame, win );
	target.setRect( win.pos.x, win.pos.y, win.size.width, win.size.height );
    }
    else if ( ni.state() & NET::SkipTaskbar ) {
	target.setRect( 0, 0, 0, 0 );
    }
    else {
	NETRect r = ni.iconGeometry();
	target.setRect( r.pos.x, r.pos.y, r.size.width, r.size.height );
    }

    moveNear( target );
}

void KPassivePopup::moveNear( QRect target )
{
    QPoint pos = target.topLeft();
    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();

    if ( x < ( qApp->desktop()->width() / 2 ) )
	x = x + target.width();
    else
	x = x - w;

    if ( (y + h) > qApp->desktop()->height() )
	y = y - h + target.height();

#ifdef OLD_BITS
    if ( (x - w) >= 0  )
	x = x - w;
#endif

    move( x, y );
}

//
// Convenience Methods
//

KPassivePopup *KPassivePopup::message( const QString &caption, const QString &text,
				       const QPixmap &icon,
				       QWidget *parent, const char *name, int timeout )
{
    KPassivePopup *pop = new KPassivePopup( parent, name );
    pop->setAutoDelete( true );
    pop->setView( caption, text, icon );
    pop->hideDelay = timeout;
    pop->show();

    return pop;
}

KPassivePopup *KPassivePopup::message( const QString &text, QWidget *parent, const char *name )
{
    return message( QString::null, text, QPixmap(), parent, name );
}

KPassivePopup *KPassivePopup::message( const QString &caption, const QString &text,
				       QWidget *parent, const char *name )
{
    return message( caption, text, QPixmap(), parent, name );
}

KPassivePopup *KPassivePopup::message( const QString &caption, const QString &text,
				       const QPixmap &icon, WId parent, const char *name, int timeout )
{
    KPassivePopup *pop = new KPassivePopup( parent, name );
    pop->setAutoDelete( true );
    pop->setView( caption, text, icon );
    pop->hideDelay = timeout;
    pop->show();

    return pop;
}

// Local Variables:
// c-basic-offset: 4
// End:


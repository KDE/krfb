// -*- c++ -*-

/*
 *   Copyright            : (C) 2001-2002 by Richard Moore
 *   Email                : rich@kde.org
 */

#ifndef KPASSIVEPOPUP_H
#define KPASSIVEPOPUP_H

#include <qframe.h>

class QBoxLayout;
class QTimer;
class QLabel;

/**
 * A dialog-like popup that displays messages without interupting the user.
 * The simplest uses of KPassivePopup are by using the various @ref message() static
 * methods. The position the popup appears at depends on the type of the parent window:
 *
 * @li Normal Windows: The popup is placed adjacent to the icon of the window.
 * @li System Tray Windows: The popup is placed adjact to the system tray window itself.
 * @li Skip Taskbar Windows: The popup is placed adjact to the window
 *     itself if it is visible, and at the edge of the desktop otherwise.
 *
 * The most basic use of KPassivePopup displays a popup containing a piece of text:
 * <pre>
 *    KPassivePopup::message( &quot;This is the message&quot;, this );
 * </pre>
 * We can create popups with titles and icons too, as this example shows:
 * <pre>
 *    QPixmap px;
 *    px.load( &quot;hi32-app-logtracker.png&quot; );
 *    KPassivePopup::message( &quot;Some title&quot;, &quot;This is the main text&quot;, px, this );
 * </pre>
 * For more control over the popup, you can use the @ref setView(QWidget *) method
 * to create a custom popup.
 * <pre>
 *    KPassivePopup *pop = new KPassivePopup( parent );
 *
 *    QVBox *vb = new QVBox( pop );
 *    (void) new QLabel( &quot;&lt;b>Isn't this great?&lt;/b>&quot;, vb );
 *
 *    QHBox *box = new QHBox( vb );
 *    (void) new QPushButton( &quot;Yes&quot;, box );
 *    (void) new QPushButton( &quot;No&quot;, box );
 *
 *    pop->setView( vb );
 *    pop->show();
 * </pre>
 *
 * @version $Id$
 * @author Richard Moore, rich@kde.org
 */
class KPassivePopup : public QFrame
{
    Q_OBJECT

public:
    /**
     * Creates a popup for the specified widget.
     */
    KPassivePopup( QWidget *parent=0, const char *name=0, WFlags f=0 );

    /**
     * Creates a popup for the specified window.
     */
    KPassivePopup( WId parent, const char *name=0, WFlags f=0 );

    /**
     * Cleans up.
     */
    virtual ~KPassivePopup();

    /**
     * Sets the main view to be the specified widget (which must be a child of the popup).
     */
    void setView( QWidget *child );

    /**
     * Creates a standard view then calls @ref #setView(QWidget*) .
     */
    void setView( const QString &caption, const QString &text = QString::null );

    /**
     * Creates a standard view then calls @ref #setView(QWidget*) .
     */
    virtual void setView( const QString &caption, const QString &text, const QPixmap &icon );

    /**
     * Returns the main view.
     */
    QWidget *view() const { return msgView; }

    /**
     * Returns the delay before the popup is removed automatically.
     */
    int timeout() const { return hideDelay; }

    /**
     * Enables / disables auto-deletion of this widget when the @ref timeout
     * occurs.
     * The default is false. If you use the class-methods message(),
     * auto-delection is turned on by default.
     */
    virtual void setAutoDelete( bool autoDelete );

    /**
     * @returns true if the widget auto-deletes itself when the
     * @ref timeout occurs.
     * @see #setAutoDelete
     */
    bool autoDelete() const { return m_autoDelete; }

    /**
     * Convenience method that displays popup with the specified  message  beside the
     * icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see #setAutoDelete
     */
    static KPassivePopup *message( const QString &text, QWidget *parent, const char *name=0 );

    /**
     * Convenience method that displays popup with the specified caption and message
     * beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see #setAutoDelete
     */
    static KPassivePopup *message( const QString &caption, const QString &text,
				   QWidget *parent, const char *name=0 );

    /**
     * Convenience method that displays popup with the specified icon, caption and
     * message beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see #setAutoDelete
     */
    static KPassivePopup *message( const QString &caption, const QString &text,
				   const QPixmap &icon,
				   QWidget *parent, const char *name=0, int timeout = -1 );

    /**
     * Convenience method that displays popup with the specified icon, caption and
     * message beside the icon of the specified window.
     * Note that the returned object is destroyed when it is hidden.
     * @see #setAutoDelete
     */
    static KPassivePopup *message( const QString &caption, const QString &text,
				   const QPixmap &icon,
				   WId parent, const char *name=0, int timeout = -1 );

public slots:
    /**
     * Sets the delay for the popup is removed automatically. Setting the delay to 0
     * disables the timeout, if you're doing this, you may want to connect the
     * @ref clicked() signal to the hide() slot.
	 * Setting the delay to -1 makes it use the default value.
     */
    void setTimeout( int delay );

    /**
     * Reimplemented to reposition the popup.
     */
    virtual void show();

signals:
    /**
     * Emitted when the popup is clicked.
     */
    void clicked();

    /**
     * Emitted when the popup is clicked.
     */
    void clicked( QPoint pos );

protected:
    /**
     * This method positions the popup.
     */
    virtual void positionSelf();

    /**
     * Reimplemented to destroy the object when @ref autoDelete() is
     * enabled.
     */
    virtual void hideEvent( QHideEvent * );

    /**
     * Moves the popup to be adjacent to the icon of the specified rectangle.
     */
    void moveNear( QRect target );

    /**
     * Reimplemented to detect mouse clicks.
     */
    virtual void mouseReleaseEvent( QMouseEvent *e );

private:
    void init();

    WId window;
    QWidget *msgView;
    QBoxLayout *layout;
    int hideDelay;
    QTimer *hideTimer;

    QLabel *ttlIcon;
    QLabel *ttl;
    QLabel *msg;

    bool m_autoDelete;

    /* @internal */
    class Private *d;
};

#endif // KPASSIVEPOPUP_H

// Local Variables:
// c-basic-offset: 4
// End:


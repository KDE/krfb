/* This file is part of the KDE project
   Copyright (C) 2017 Alexey Min <alexey.min@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "xcb_framebuffer.h"
#include "krfb_fb_xcb_debug.h"

#include <xcb/xproto.h>
#include <xcb/damage.h>
#include <xcb/shm.h>
#include <xcb/xcb_image.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QAbstractNativeEventFilter>
#include <qpa/qplatformnativeinterface.h>
#include <QtGui/private/qtx11extras_p.h>

class KrfbXCBEventFilter: public QAbstractNativeEventFilter
{
public:
    KrfbXCBEventFilter(XCBFrameBuffer *owner);

public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

public:
    int xdamageBaseEvent;
    int xdamageBaseError;
    int xshmBaseEvent;
    int xshmBaseError;
    bool xshmAvail;
    XCBFrameBuffer *fb_owner;
};



KrfbXCBEventFilter::KrfbXCBEventFilter(XCBFrameBuffer *owner):
    xdamageBaseEvent(0), xdamageBaseError(0),
    xshmBaseEvent(0), xshmBaseError(0), xshmAvail(false),
    fb_owner(owner)
{
    const xcb_query_extension_reply_t *xdamage_data = xcb_get_extension_data(
                QX11Info::connection(), &xcb_damage_id);
    if (xdamage_data) {
        // also query extension version!
        // ATTENTION: if we don't do that, xcb_damage_create() will always FAIL!
        xcb_damage_query_version_reply_t *xdamage_version = xcb_damage_query_version_reply(
                    QX11Info::connection(),
                    xcb_damage_query_version(
                        QX11Info::connection(),
                        XCB_DAMAGE_MAJOR_VERSION,
                        XCB_DAMAGE_MINOR_VERSION),
                    nullptr);
        if (!xdamage_version) {
            qWarning() << "xcb framebuffer: ERROR: Failed to get XDamage extension version!\n";
            return;
        }

#ifdef _DEBUG
        qCDebug(KRFB_FB_XCB) << "xcb framebuffer: XDamage extension version:" <<
               xdamage_version->major_version << "." << xdamage_version->minor_version;
#endif

        free(xdamage_version);

        xdamageBaseEvent = xdamage_data->first_event;
        xdamageBaseError = xdamage_data->first_error;

        // XShm presence is optional. If it is present, all image getting
        // operations will be faster, without XShm it will only be slower.
        const xcb_query_extension_reply_t *xshm_data = xcb_get_extension_data(
                    QX11Info::connection(), &xcb_shm_id);
        if (xshm_data) {
            xshmAvail = true;
            xshmBaseEvent = xshm_data->first_event;
            xshmBaseError = xshm_data->first_error;
        } else {
            xshmAvail = false;
            qWarning() << "xcb framebuffer: WARNING: XSHM extension not available!";
        }
    } else {
        // if there is no xdamage available, this plugin can be considered useless anyway.
        // you can use just qt framebuffer plugin instead...
        qWarning() << "xcb framebuffer: ERROR: no XDamage extension available. I am useless.";
        qWarning() << "xcb framebuffer:        use qt framebuffer plugin instead.";
    }
}


bool KrfbXCBEventFilter::nativeEventFilter(const QByteArray &eventType,
                                           void *message, long *result) {
    Q_UNUSED(result);  // "result" is only used on windows

    if (xdamageBaseEvent == 0) return false;  // no xdamage extension

    if (eventType == "xcb_generic_event_t") {
        auto ev = static_cast<xcb_generic_event_t *>(message);
        if ((ev->response_type & 0x7F) == (xdamageBaseEvent + XCB_DAMAGE_NOTIFY)) {
            // this is xdamage notification
            this->fb_owner->handleXDamageNotify(ev);
            return true;  // filter out this event, stop its processing
        }
    }

    // continue event processing
    return false;
}


class XCBFrameBuffer::P {
public:
    xcb_damage_damage_t     damage;
    xcb_shm_segment_info_t  shminfo;
    xcb_screen_t           *rootScreen;   // X screen info (all monitors)
    xcb_image_t            *framebufferImage;
    xcb_image_t            *updateTile;

    KrfbXCBEventFilter     *x11EvtFilter;

    bool                    running;

    QRect                   area;  // capture area, primary monitor coordinates
    WId                     win;
};


static xcb_screen_t *get_xcb_screen(xcb_connection_t *conn, int screen_num) {
    xcb_screen_t *screen = nullptr;
    xcb_screen_iterator_t screens_iter = xcb_setup_roots_iterator(xcb_get_setup(conn));
    for (; screens_iter.rem; --screen_num, xcb_screen_next(&screens_iter))
        if (screen_num == 0)
            screen = screens_iter.data;
    return screen;
}



XCBFrameBuffer::XCBFrameBuffer(QObject *parent):
    FrameBuffer(parent), d(new XCBFrameBuffer::P)
{
    d->running = false;
    d->damage = XCB_NONE;
    d->framebufferImage = nullptr;
    d->shminfo.shmaddr = nullptr;
    d->shminfo.shmid = XCB_NONE;
    d->shminfo.shmseg = XCB_NONE;
    d->updateTile = nullptr;
    d->area.setRect(0, 0, 0, 0);
    d->x11EvtFilter = new KrfbXCBEventFilter(this);
    d->rootScreen = get_xcb_screen(QX11Info::connection(), QX11Info::appScreen());

    this->fb = nullptr;

    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        QPlatformNativeInterface* native = qApp->platformNativeInterface();
        d->win = reinterpret_cast<WId>(native->nativeResourceForScreen(QByteArrayLiteral("rootwindow"), primaryScreen));
        qreal scaleFactor = primaryScreen->devicePixelRatio();
        d->area = { primaryScreen->geometry().topLeft() * scaleFactor,
                    primaryScreen->geometry().bottomRight() * scaleFactor };

        qCDebug(KRFB_FB_XCB) << "xcb framebuffer: Primary screen: " << primaryScreen->name()
                 << ", geometry: " << primaryScreen->geometry()
                 << ", device scaling: " << scaleFactor
                 << ", native size: " << d->area
                 << ", depth: " << primaryScreen->depth();
    } else {
        qWarning() << "xcb framebuffer: ERROR: Failed to get application's primary screen info!";
        return;
    }

    d->framebufferImage = xcb_image_get(QX11Info::connection(),
                                        d->win,
                                        d->area.left(),
                                        d->area.top(),
                                        d->area.width(),
                                        d->area.height(),
                                        0xFFFFFFFF, // == Xlib: AllPlanes ((unsigned long)~0L)
                                        XCB_IMAGE_FORMAT_Z_PIXMAP);
    if (d->framebufferImage) {
#ifdef _DEBUG
        qCDebug(KRFB_FB_XCB) << "xcb framebuffer: Got primary screen image. bpp: " << d->framebufferImage->bpp
                 << ", size (" << d->framebufferImage->width << d->framebufferImage->height << ")"
                 << ", depth: " << d->framebufferImage->depth
                 << ", padded width: " << d->framebufferImage->stride;
#endif
        this->fb = (char *)d->framebufferImage->data;
    } else {
        qWarning() << "xcb framebuffer: ERROR: Failed to get primary screen image!";
        return;
    }

    // all XShm operations should take place only if Xshm extension was loaded
    if (d->x11EvtFilter->xshmAvail) {
        // Create xcb_image_t structure, but do not automatically allocate memory
        // for image data storage - it will be allocated as shared memory.
        // "If base == 0 and bytes == ~0 and data == 0, no storage will be auto-allocated."
        // Width and height of the image = size of the capture area.
        d->updateTile = xcb_image_create_native(
                    QX11Info::connection(),
                    d->area.width(),            // width
                    d->area.height(),           // height
                    XCB_IMAGE_FORMAT_Z_PIXMAP,  // image format
                    d->rootScreen->root_depth,  // depth
                    nullptr,                    // base address = 0
                    (uint32_t)~0,               // bytes = 0xffffffff
                    nullptr);                   // data = 0
        if (d->updateTile) {
#ifdef _DEBUG
            qCDebug(KRFB_FB_XCB) << "xcb framebuffer: Successfully created new empty image in native format"
                << "\n    size: " << d->updateTile->width << "x" << d->updateTile->height
                    << "(stride: " << d->updateTile->stride << ")"
                << "\n    bpp, depth:  " << d->updateTile->bpp << d->updateTile->depth // 32, 24
                << "\n    addr of base, data:  " << d->updateTile->base << (void *)d->updateTile->data
                << "\n    size:  " << d->updateTile->size
                << "\n    image byte order = " << d->updateTile->byte_order   // == 0 .._LSB_FIRST
                << "\n    image bit order = " << d->updateTile->bit_order     // == 1 .._MSB_FIRST
                << "\n    image plane_mask = " << d->updateTile->plane_mask;  // == 16777215 == 0x00FFFFFF
#endif

            // allocate shared memory block only once, make its size large enough
            // to fit whole capture area (d->area rect)
            // so, we get as many bytes as needed for image (updateTile->size)
            d->shminfo.shmid = shmget(IPC_PRIVATE, d->updateTile->size, IPC_CREAT | 0777);
            // attached shared memory address is stored both in shminfo structure and in xcb_image_t->data
            d->shminfo.shmaddr = (uint8_t *)shmat(d->shminfo.shmid, nullptr, 0);
            d->updateTile->data = d->shminfo.shmaddr;
            // we keep updateTile->base == NULL here, so xcb_image_destroy() will not attempt
            // to free this block, just in case.

            // attach this shm segment also to X server
            d->shminfo.shmseg = xcb_generate_id(QX11Info::connection());
            xcb_shm_attach(QX11Info::connection(), d->shminfo.shmseg, d->shminfo.shmid, 0);

#ifdef _DEBUG
            qCDebug(KRFB_FB_XCB) << "    shm id: " << d->shminfo.shmseg << ", addr: " << (void *)d->shminfo.shmaddr;
#endif

            // will return 1 on success (yes!)
            int shmget_res = xcb_image_shm_get(
                        QX11Info::connection(),
                        d->win,
                        d->updateTile,
                        d->shminfo,
                        d->area.left(), // x
                        d->area.top(),  // y (size taken from image structure itself)?
                        0xFFFFFFFF);

            if (shmget_res == 0) {
                // error! shared mem not working?
                // will not use shared mem! detach and cleanup
                xcb_shm_detach(QX11Info::connection(), d->shminfo.shmseg);
                shmdt(d->shminfo.shmaddr);
                shmctl(d->shminfo.shmid, IPC_RMID, nullptr); // mark shm segment as removed
                d->x11EvtFilter->xshmAvail = false;
                d->shminfo.shmaddr = nullptr;
                d->shminfo.shmid = XCB_NONE;
                d->shminfo.shmseg = XCB_NONE;
                qWarning() << "xcb framebuffer: ERROR: xcb_image_shm_get() result: " << shmget_res;
            }

            // image is freed, and recreated again for every new damage rectangle
            // data was allocated manually and points to shared mem;
            // tell xcb_image_destroy() do not free image data
            d->updateTile->data = nullptr;
            xcb_image_destroy(d->updateTile);
            d->updateTile = nullptr;
        }
    }

#ifdef _DEBUG
    qCDebug(KRFB_FB_XCB) << "xcb framebuffer: XCBFrameBuffer(), xshm base event = " << d->x11EvtFilter->xshmBaseEvent
             << ", xshm base error = " << d->x11EvtFilter->xdamageBaseError
             << ", xdamage base event = " << d->x11EvtFilter->xdamageBaseEvent
             << ", xdamage base error = " << d->x11EvtFilter->xdamageBaseError;
#endif

    QCoreApplication::instance()->installNativeEventFilter(d->x11EvtFilter);
}


XCBFrameBuffer::~XCBFrameBuffer() {
    // first - uninstall x11 event filter
    QCoreApplication::instance()->removeNativeEventFilter(d->x11EvtFilter);
    //
    if (d->framebufferImage) {
        xcb_image_destroy(d->framebufferImage);
        fb = nullptr;  // image data was already destroyed by above call
    }
    if (d->x11EvtFilter->xshmAvail) {
        // detach shared memory
        if (d->shminfo.shmseg != XCB_NONE)
            xcb_shm_detach(QX11Info::connection(), d->shminfo.shmseg); // detach from X server
        if (d->shminfo.shmaddr)
            shmdt(d->shminfo.shmaddr); // detach addr from our address space
        if (d->shminfo.shmid != XCB_NONE)
            shmctl(d->shminfo.shmid, IPC_RMID, nullptr); // mark shm segment as removed
    }
    // and delete image used for shared mem
    if (d->updateTile) {
        d->updateTile->base = nullptr;
        d->updateTile->data = nullptr;
        xcb_image_destroy(d->updateTile);
    }
    // we don't use d->x11EvtFilter anymore, can delete it now
    if (d->x11EvtFilter) {
        delete d->x11EvtFilter;
    }
    delete d;
}


int XCBFrameBuffer::depth() {
    if (d->framebufferImage) {
        return d->framebufferImage->depth;
    }
    return 0;
}


int XCBFrameBuffer::height() {
    if (d->framebufferImage) {
        return d->framebufferImage->height;
    }
    return 0;
}


int XCBFrameBuffer::width() {
    if (d->framebufferImage) {
        return d->framebufferImage->width;
    }
    return 0;
}

int XCBFrameBuffer::paddedWidth() {
    if (d->framebufferImage) {
        return d->framebufferImage->stride;
    }
    return 0;
}


void XCBFrameBuffer::getServerFormat(rfbPixelFormat &format) {
    if (!d->framebufferImage) return;

    // get information about XCB visual params
    xcb_visualtype_t *root_visualtype = nullptr;  // visual info
    if (d->rootScreen) {
        xcb_visualid_t root_visual = d->rootScreen->root_visual;
        xcb_depth_iterator_t depth_iter;
        // To get the xcb_visualtype_t structure, it's a bit less easy.
        // You have to get the xcb_screen_t structure that you want, get its
        // root_visual member, then iterate over the xcb_depth_t's and the
        // xcb_visualtype_t's, and compare the xcb_visualid_t of these
        // xcb_visualtype_ts: with root_visual
        depth_iter = xcb_screen_allowed_depths_iterator(d->rootScreen);
        for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
            xcb_visualtype_iterator_t visual_iter;
            visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
            for (; visual_iter.rem; xcb_visualtype_next(&visual_iter)) {
                if (root_visual == visual_iter.data->visual_id) {
                    root_visualtype = visual_iter.data;
                    break;
                }
            }
        }
    }


    // fill in format common info
    format.bitsPerPixel = d->framebufferImage->bpp;
    format.depth = d->framebufferImage->depth;
    format.trueColour = true;  // not using color palettes
    format.bigEndian = false;  // always false for ZPIXMAP format!

    // information about pixels layout

    if (root_visualtype) {
#ifdef _DEBUG
        qDebug("xcb framebuffer: Got info about root visual:\n"
               "    bits per rgb value: %d\n"
               "    red   mask: %08x\n"
               "    green mask: %08x\n"
               "    blue  mask: %08x\n",
               (int)root_visualtype->bits_per_rgb_value,
               root_visualtype->red_mask,
               root_visualtype->green_mask,
               root_visualtype->blue_mask);
#endif

        // calculate shifts
        format.redShift = 0;
        if (root_visualtype->red_mask) {
            while (!(root_visualtype->red_mask & (1 << format.redShift))) {
                format.redShift++;
            }
        }
        format.greenShift = 0;
        if (root_visualtype->green_mask) {
            while (!(root_visualtype->green_mask & (1 << format.greenShift))) {
                format.greenShift++;
            }
        }
        format.blueShift = 0;
        if (root_visualtype->blue_mask) {
            while (!(root_visualtype->blue_mask & (1 << format.blueShift))) {
                format.blueShift++;
            }
        }

        // calculate pixel max value.
        // NOTE: bits_per_rgb_value is unreliable, thus should be avoided.
        format.redMax   = root_visualtype->red_mask >> format.redShift;
        format.greenMax = root_visualtype->green_mask >> format.greenShift;
        format.blueMax  = root_visualtype->blue_mask >> format.blueShift;

#ifdef _DEBUG
        qCDebug(KRFB_FB_XCB,
            "    Calculated redShift   = %d\n"
            "    Calculated greenShift = %d\n"
            "    Calculated blueShift  = %d\n"
            "    Calculated max values: R%d G%d B%d",
               format.redShift, format.greenShift, format.blueShift
               format.redMax, format.greenMax, format.blueMax);
#endif
    } else {
        // some kind of fallback (unlikely code execution will go this way)
        // idea taken from qt framefuffer sources
        if (format.bitsPerPixel == 8) {
            format.redShift   = 0;
            format.greenShift = 3;
            format.blueShift  = 6;
            format.redMax   = 7;
            format.greenMax = 7;
            format.blueMax  = 3;
        } else if (format.bitsPerPixel == 16) {
            // TODO: 16 bits per pixel format ??
            // what format of pixels does X server use for 16-bpp?
        } else if (format.bitsPerPixel == 32) {
            format.redMax   = 0xff;
            format.greenMax = 0xff;
            format.blueMax  = 0xff;
            if (format.bigEndian) {
                format.redShift   = 0;
                format.greenShift = 8;
                format.blueShift  = 16;
            } else {
                format.redShift   = 16;
                format.greenShift = 8;
                format.blueShift  = 0;
            }
        }
    }
}


/**
 * This function contents was taken from X11 framebuffer source code.
 * It simply several intersecting rectangles into one bigger rect.
 * Non-intersecting rects are treated as different rects and exist
 * separately in this->tiles QList.
 */
void XCBFrameBuffer::cleanupRects() {
    QList<QRect> cpy = tiles;
    bool inserted = false;
    tiles.clear();

    QListIterator<QRect> iter(cpy);
    while (iter.hasNext()) {
        const QRect &r = iter.next();
        // skip rects not intersecting with primary monitor
        if (!r.intersects(d->area)) continue;
        // only take intersection of this rect with primary monitor rect
        QRect ri = r.intersected(d->area);

        if (tiles.size() > 0) {
            for (auto &tile : tiles) {
                // if current rect has intersection with tile, unite them
                if (ri.intersects(tile)) {
                    tile |= ri;
                    inserted = true;
                    break;
                }
            }

            if (!inserted) {
                // else, append to list as different rect
                tiles.append(ri);
            }
        } else {
            // tiles list is empty, append first item
            tiles.append(ri);
        }
    }

    // increase all rectangles size by 30 pixels each side.
    // limit coordinates to primary monitor boundaries.
    for (auto &tile : tiles) {
        tile.adjust(-30, -30, 30, 30);
        if (tile.top() < d->area.top()) {
            tile.setTop(d->area.top());
        }
        if (tile.bottom() > d->area.bottom()) {
            tile.setBottom(d->area.bottom());
        }
        //
        if (tile.left() < d->area.left()) {
            tile.setLeft(d->area.left());
        }
        if (tile.right() > d->area.right()) {
            tile.setRight(d->area.right());
        }
        // move update rects so that they are positioned relative to
        //   framebuffer image, not whole screen
        tile.moveTo(tile.left() - d->area.left(),
                    tile.top() - d->area.top());
    }
}


/**
 * This function is called by RfbServerManager::updateScreens()
 * approximately every 50ms (!!), driven by QTimer to get all
 * modified rectangles on the screen
 */
QList<QRect> XCBFrameBuffer::modifiedTiles() {
    QList<QRect> ret;
    if (!d->running) {
        return ret;
    }

    cleanupRects();

    if (tiles.size() > 0) {
        if (d->x11EvtFilter->xshmAvail) {

            // loop over all damage rectangles gathered up to this time
            QListIterator<QRect> iter(tiles);
            //foreach(const QRect &r, tiles) {
            while (iter.hasNext()) {
                const QRect &r = iter.next();

                // get image data into shared memory segment
                // now rects are positioned relative to framebufferImage,
                // but we need to get image from the whole screen, so
                // translate whe coordinates
                xcb_shm_get_image_cookie_t sgi_cookie = xcb_shm_get_image(
                            QX11Info::connection(),
                            d->win,
                            d->area.left() + r.left(),
                            d->area.top()  + r.top(),
                            r.width(),
                            r.height(),
                            0xFFFFFFFF,
                            XCB_IMAGE_FORMAT_Z_PIXMAP,
                            d->shminfo.shmseg,
                            0);

                xcb_shm_get_image_reply_t *sgi_reply = xcb_shm_get_image_reply(
                            QX11Info::connection(), sgi_cookie, nullptr);
                if (sgi_reply) {
                    // create temporary image to get update rect contents into
                    d->updateTile = xcb_image_create_native(
                                QX11Info::connection(),
                                r.width(),
                                r.height(),
                                XCB_IMAGE_FORMAT_Z_PIXMAP,
                                d->rootScreen->root_depth,
                                nullptr,      // base == 0
                                (uint32_t)~0, // bytes == ~0
                                nullptr);

                    if (d->updateTile) {
                        d->updateTile->data = d->shminfo.shmaddr;

                        // copy pixels from this damage rectangle image
                        // to our total framebuffer image
                        int pxsize =  d->framebufferImage->bpp / 8;
                        char *dest = fb + ((d->framebufferImage->stride * r.top()) + (r.left() * pxsize));
                        char *src = (char *)d->updateTile->data;
                        for (int i = 0; i < d->updateTile->height; i++) {
                            memcpy(dest, src, d->updateTile->stride);  // copy whole row of pixels
                            dest += d->framebufferImage->stride;
                            src += d->updateTile->stride;
                        }

                        // delete temporary image
                        d->updateTile->data = nullptr;
                        xcb_image_destroy(d->updateTile);
                        d->updateTile = nullptr;
                    }

                    free(sgi_reply);
                }
            } // foreach
        } else {
            // not using shared memory
            // will use just xcb_image_get() and copy pixels
            for (const QRect& r : std::as_const(tiles)) {
                // I did not find XGetSubImage() analog in XCB!!
                // need function that copies pixels from one image to another
                xcb_image_t *damagedImage = xcb_image_get(
                            QX11Info::connection(),
                            d->win,
                            r.left(),
                            r.top(),
                            r.width(),
                            r.height(),
                            0xFFFFFFFF, // AllPlanes
                            XCB_IMAGE_FORMAT_Z_PIXMAP);
                // manually copy pixels
                int pxsize =  d->framebufferImage->bpp / 8;
                char *dest = fb + ((d->framebufferImage->stride * r.top()) + (r.left() * pxsize));
                char *src = (char *)damagedImage->data;
                // loop every row in damaged image
                for (int i = 0; i < damagedImage->height; i++) {
                    // copy whole row of pixels from src image to dest
                    memcpy(dest, src, damagedImage->stride);
                    dest += d->framebufferImage->stride;  // move 1 row down in dest
                    src += damagedImage->stride;          // move 1 row down in src
                }
                //
                xcb_image_destroy(damagedImage);
            }
        }
    } // if (tiles.size() > 0)

    ret = tiles;
    tiles.clear();
    // ^^ If we clear here all our known "damage areas", then we can also clear
    //    damaged area for xdamage? No, we don't need to in our case
    //    (XCB_DAMAGE_REPORT_LEVEL_RAW_RECTANGLES report mode)
    //xcb_damage_subtract(QX11Info::connection(), d->damage, XCB_NONE, XCB_NONE);

    return ret;
}


void XCBFrameBuffer::startMonitor() {
    if (d->running) return;

    d->running = true;
    d->damage = xcb_generate_id(QX11Info::connection());
    xcb_damage_create(QX11Info::connection(), d->damage, d->win,
            XCB_DAMAGE_REPORT_LEVEL_RAW_RECTANGLES);

    // (currently) we do not call xcb_damage_subtract() EVER, because
    // RAW rectangles are reported. every time some area of the screen
    // was changed, we get only that rectangle
    //xcb_damage_subtract(QX11Info::connection(), d->damage, XCB_NONE, XCB_NONE);
}


void XCBFrameBuffer::stopMonitor() {
    if (!d->running) return;
    d->running = false;
    xcb_damage_destroy(QX11Info::connection(), d->damage);
}


// void XCBFrameBuffer::acquireEvents() {} // this function was totally unused
// in X11 framebuffer, but it was the only function where XDamageSubtract() was called?
// Also it had a blocking event loop like:
//
//     XEvent ev;
//     while (XCheckTypedEvent(QX11Info::display(), d->xdamageBaseEvent + XDamageNotify, &ev)) {
//         handleXDamage(&ev);
//     }
//     XDamageSubtract(QX11Info::display(), d->damage, None, None);
//
// This loop takes all available Xdamage events from queue, and ends if there are no
// more such events in input queue.


void XCBFrameBuffer::handleXDamageNotify(xcb_generic_event_t *xevent) {
    auto xdevt = (xcb_damage_notify_event_t *)xevent;

    QRect r((int)xdevt->area.x,     (int)xdevt->area.y,
            (int)xdevt->area.width, (int)xdevt->area.height);
    this->tiles.append(r);
}


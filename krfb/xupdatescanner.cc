/*
 *  Copyright (C) 2000 heXoNet Support GmbH, D-66424 Homburg.
 *  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */
/*
 * December 15th 2001: removed coments, mouse pointer options and some
 * other stuff
 * January 10th 2002: improved hint creation (join adjacent hints)
 * February 20th: use only partial tiles
 * January 21st 2003: remember last modified scanlines, and scan them and
 *                    in every cycle, reduce scanlines to every 35th
 * January 21st 2003: scan lines around the cursor in every cycle
 *
 *                   Tim Jansen <tim@tjansen.de>
 */

#include <kdebug.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include <string.h>
#include <assert.h>

#include "xupdatescanner.h"

/* ../../krfb/libvncserver/rfb.h */
#ifdef Bool
#undef Bool
#endif
#define Bool int


#define SCANLINES 35
unsigned int scanlines[SCANLINES] = {  0, 16,  8, 24,
                                33, 4, 20, 12, 28,
			       10, 26, 18,  34, 2,
			       22,  6, 30, 14,
			        1, 17,  32, 9, 25,
			        7, 23, 15, 31,
			       19,  3, 27, 11,
			       29, 13,  5, 21 };
#define MAX_ADJ_TOLERANCE 8

#define MAX_RECENT_HITS 12
unsigned int recentHitScanlines[MAX_RECENT_HITS];

#define CURSOR_SCANLINES 5
int cursorScanlines[CURSOR_SCANLINES] = {
		-10, -4, 0, 4, 10
};




XUpdateScanner::XUpdateScanner(Display *_dpy,
			       Window _window,
			       unsigned char *_fb,
			       int _width,
			       int _height,
			       int _bitsPerPixel,
			       int _bytesPerLine,
			       bool useXShm) :
	dpy(_dpy),
	window(_window),
	fb(_fb),
	width(_width),
	height(_height),
	bitsPerPixel(_bitsPerPixel),
	bytesPerLine(_bytesPerLine),
	tileWidth(32),
	tileHeight(32),
	count (0),
	scanline(NULL),
	tile(NULL)
{
	useShm = useXShm && XShmQueryExtension(dpy);
	if (useShm) {
		int major, minor;
		Bool pixmaps;
		if ((!XShmQueryVersion(dpy, &major, &minor, &pixmaps)) || !pixmaps)
			useShm = false;
	}

	if (useShm) {
		tile = XShmCreateImage(dpy,
				       DefaultVisual( dpy, 0 ),
				       bitsPerPixel,
				       ZPixmap,
				       NULL,
				       &shminfo_tile,
				       tileWidth,
				       tileHeight);

		shminfo_tile.shmid = shmget(IPC_PRIVATE,
					    tile->bytes_per_line * tile->height,
					    IPC_CREAT | 0777);
		shminfo_tile.shmaddr = tile->data = (char *)
			shmat(shminfo_tile.shmid, 0, 0);
		shminfo_tile.readOnly = False;

		XShmAttach(dpy, &shminfo_tile);
	}
	else {
		int tlen = tileWidth*(bitsPerPixel/8);
		void *data = malloc(tlen*tileHeight);

		tile = XCreateImage(dpy,
				    DefaultVisual(dpy, 0),
				    bitsPerPixel,
				    ZPixmap,
				    0,
				    (char*)data,
				    tileWidth,
				    tileHeight,
				    8,
				    tlen);
	}

	tilesX = (width + tileWidth - 1) / tileWidth;
	tilesY = (height + tileHeight - 1) / tileHeight;
	tileMap = new bool[tilesX * tilesY];
	tileRegionMap = new struct TileChangeRegion[tilesX * tilesY];

	unsigned int i;
	for (i = 0; i < tilesX * tilesY; i++)
		tileMap[i] = false;

	if (useShm) {
		scanline = XShmCreateImage(dpy,
					   DefaultVisual(dpy, 0),
					   bitsPerPixel,
					   ZPixmap,
					   NULL,
					   &shminfo_scanline,
					   width,
					   1);

		shminfo_scanline.shmid = shmget(IPC_PRIVATE,
						scanline->bytes_per_line,
						IPC_CREAT | 0777);
		shminfo_scanline.shmaddr = scanline->data = (char *)
			shmat( shminfo_scanline.shmid, 0, 0 );
		shminfo_scanline.readOnly = False;

		XShmAttach(dpy, &shminfo_scanline);
	}
	else {
		int slen = width*(bitsPerPixel/8);
		void *data = malloc(slen);
		scanline = XCreateImage(dpy,
					DefaultVisual(dpy, 0),
					bitsPerPixel,
					ZPixmap,
					0,
					(char*)data,
					width,
					1,
					8,
					slen);
	}

	for (int i = 0; i < MAX_RECENT_HITS; i++)
		recentHitScanlines[i] = i;
}


XUpdateScanner::~XUpdateScanner()
{
	if (useShm) {
		XShmDetach(dpy, &shminfo_scanline);
		XDestroyImage(scanline);
		shmdt(shminfo_scanline.shmaddr);
		shmctl(shminfo_scanline.shmid, IPC_RMID, 0);
		XShmDetach(dpy, &shminfo_tile);
		XDestroyImage(tile);
		shmdt(shminfo_tile.shmaddr);
		shmctl(shminfo_tile.shmid, IPC_RMID, 0);
	}
	else {
		free(tile->data);
		free(scanline->data);
		XDestroyImage(scanline);
		XDestroyImage(tile);
	}
	delete [] tileMap;
	delete [] tileRegionMap;
}


// returns true if last line changed. this is used to re-scan the tile under
// this one because it is likely to be modified but missed by the probe
bool XUpdateScanner::copyTile(int x, int y, int tx, int ty)
{
	unsigned int maxWidth = width - x;
	unsigned int maxHeight = height - y;
	if (maxWidth > tileWidth)
		maxWidth = tileWidth;
	if (maxHeight > tileHeight)
		maxHeight = tileHeight;

	if (useShm) {
		if ((maxWidth == tileWidth) && (maxHeight == tileHeight)) {
			XShmGetImage(dpy, window, tile, x, y, AllPlanes);
		} else {
			XGetSubImage(dpy, window, x, y, maxWidth, maxHeight,
				     AllPlanes, ZPixmap, tile, 0, 0);
		}
	}
	else
		XGetSubImage(dpy, window, x, y, maxWidth, maxHeight,
			     AllPlanes, ZPixmap, tile, 0, 0);

	unsigned int line;
	int pixelsize = bitsPerPixel >> 3;
	unsigned char *src = (unsigned char*) tile->data;
	unsigned char *dest = fb + y * bytesPerLine + x * pixelsize;

        unsigned char *ssrc = src;
        unsigned char *sdest = dest;
        int firstLine = maxHeight;

	for (line = 0; line < maxHeight; line++) {
		if (memcmp(sdest, ssrc, maxWidth * pixelsize)) {
			firstLine = line;
			break;
		}
		ssrc += tile->bytes_per_line;
		sdest += bytesPerLine;
        }

        if (firstLine == maxHeight) {
		tileMap[tx + ty * tilesX] = false;
		return false;
        }

        unsigned char *msrc = src + (tile->bytes_per_line * maxHeight);
        unsigned char *mdest = dest + (bytesPerLine * maxHeight);
        int lastLine = firstLine;

        for (line = maxHeight-1; line > firstLine; line--) {
		msrc -= tile->bytes_per_line;
		mdest -= bytesPerLine;
		if (memcmp(mdest, msrc, maxWidth * pixelsize)) {
			lastLine = line;
			break;
		}
        }

        for (line = firstLine; line <= lastLine; line++) {
                memcpy(sdest, ssrc, maxWidth * pixelsize );
                ssrc += tile->bytes_per_line;
		sdest += bytesPerLine;
	}

        struct TileChangeRegion *r = &tileRegionMap[tx + (ty * tilesX)];
        r->firstLine = firstLine;
        r->lastLine = lastLine;

        return lastLine == (maxHeight-1);
}

void XUpdateScanner::copyAllTiles()
{
	for (unsigned int y = 0; y < tilesY; y++) {
		for (unsigned int x = 0; x < tilesX; x++) {
                        if (tileMap[x + y * tilesX])
                                if (copyTile(x*tileWidth, y*tileHeight, x, y) &&
                                    ((y+1) < tilesY))
					tileMap[x + (y+1) * tilesX] = true;
		}
	}

}

void XUpdateScanner::createHintFromTile(int x, int y, int th, Hint &hint)
{
	unsigned int w = width - x;
	unsigned int h = height - y;
	if (w > tileWidth)
		w = tileWidth;
	if (h > th)
		h = th;

	hint.x = x;
	hint.y = y;
	hint.w = w;
	hint.h = h;
}

void XUpdateScanner::addTileToHint(int x, int y, int th, Hint &hint)
{
	unsigned int w = width - x;
	unsigned int h = height - y;
	if (w > tileWidth)
		w = tileWidth;
	if (h > th)
		h = th;

	if (hint.x > x) {
		hint.w += hint.x - x;
		hint.x = x;
	}

	if (hint.y > y) {
		hint.h += hint.y - y;
		hint.y = y;
	}

	if ((hint.x+hint.w) < (x+w)) {
		hint.w = (x+w) - hint.x;
	}

	if ((hint.y+hint.h) < (y+h)) {
		hint.h = (y+h) - hint.y;
	}
}

static void printStatistics(Hint &hint) {
	static int snum = 0;
	static float ssum = 0.0;

	int oX0 = hint.x & 0xffffffe0;
	int oY0 = hint.y & 0xffffffe0;
	int oX2 = (hint.x+hint.w) & 0x1f;
	int oY2 = (hint.y+hint.h) & 0x1f;
	int oX3 = (((hint.x+hint.w) | 0x1f) + ((oX2 == 0) ? 0 : 1)) & 0xffffffe0;
	int oY3 = (((hint.y+hint.h) | 0x1f) + ((oY2 == 0) ? 0 : 1)) & 0xffffffe0;
	float s0 = hint.w*hint.h;
	float s1 = (oX3-oX0)*(oY3-oY0);
	float p = (100*s0/s1);
	ssum += p;
	snum++;
	float avg = ssum / snum;
	kdDebug() << "avg size: "<< avg <<"%"<<endl;
}

void XUpdateScanner::flushHint(int x, int y, int &x0,
			       Hint &hint, QPtrList<Hint> &hintList)
{
	if (x0 < 0)
		return;

	x0 = -1;

	assert (hint.w > 0);
	assert (hint.h > 0);

	//printStatistics(hint);

	hintList.append(new Hint(hint));
}

void XUpdateScanner::createHints(QPtrList<Hint> &hintList)
{
	Hint hint;
	int x0 = -1;

	for (int y = 0; y < tilesY; y++) {
		int x;
		for (x = 0; x < tilesX; x++) {
			int idx = x + y * tilesX;
			if (tileMap[idx]) {
				int ty = tileRegionMap[idx].firstLine;
				int th = tileRegionMap[idx].lastLine - ty +1;
				if (x0 < 0) {
					createHintFromTile(x * tileWidth,
							   (y * tileHeight) + ty,
							   th,
							   hint);
					x0 = x;

				} else {
					addTileToHint(x * tileWidth,
						      (y * tileHeight) + ty,
						      th,
						      hint);
				}
			}
			else
				flushHint(x, y, x0, hint, hintList);
		}
		flushHint(x, y, x0, hint, hintList);
	}
}

void XUpdateScanner::testScanline(int y, bool rememberHits) {
	if (y < 0)
		return;
	if (y >= (int)height)
		return;

	int x = 0;
	bool hit = false;
	if (useShm)
		XShmGetImage(dpy, window, scanline, 0, y, AllPlanes);
	else
		XGetSubImage(dpy, window, 0, y, width, 1,
			     AllPlanes, ZPixmap, scanline, 0, 0);

	while (x < width) {
		int pixelsize = bitsPerPixel >> 3;
		unsigned char *src = (unsigned char*) scanline->data +
			x * pixelsize;
		unsigned char *dest = fb +
			y * bytesPerLine + x * pixelsize;
		int w = (x + 32) > width ? (width-x) : 32;
		if (memcmp(dest, src, w * pixelsize)) {
			hit = true;
			tileMap[(x / tileWidth) +
				(y / tileHeight) * tilesX] = true;
		}
		x += 32;
	}

	if (!rememberHits)
		return;

	for (int i = 1; i < MAX_RECENT_HITS; i++)
		recentHitScanlines[i-1] = recentHitScanlines[i];
	recentHitScanlines[MAX_RECENT_HITS-1] = y;
}

void XUpdateScanner::searchUpdates(QPtrList<Hint> &hintList, int ptrY)
{
	count++;
	count %= SCANLINES;

	unsigned int i;
	unsigned int y;

	for (i = 0; i < (tilesX * tilesY); i++) {
		tileMap[i] = false;
	}

	// test last scanlines with hits
	for (i = 0; i < MAX_RECENT_HITS; i++)
		testScanline(recentHitScanlines[i], true);

	// test scanlines around the cursor
	for (i = 0; i < CURSOR_SCANLINES; i++)
		testScanline(ptrY+cursorScanlines[i], false);
	// test last/first line of the tiles around the cursor
	// (assumes tileHeight = 32)
	testScanline((ptrY&0xffe0)-1, false);
	testScanline((ptrY|0x1f)+1, false);

	// test every SCANLINESth scanline
	y = scanlines[count];
	while (y < (int)height) {
		testScanline(y, true);
		y += SCANLINES;
	}

	copyAllTiles();

	createHints(hintList);
}




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
 * 
 *                   Tim Jansen <tim@tjansen.de>
 */   

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include <string.h>

#include "XUpdateScanner.h"

unsigned int scanlines[32] = {  0, 16,  8, 24,
                                4, 20, 12, 28,
			       10, 26, 18,  2,
			       22,  6, 30, 14,
			        1, 17,  9, 25,
			        7, 23, 15, 31,
			       19,  3, 27, 11,
			       29, 13,  5, 21 };

XUpdateScanner::XUpdateScanner(Display *_dpy,
			       Window _window,
			       unsigned char *_fb,
			       int _width,
			       int _height,
			       int _bitsPerPixel,
			       int _bytesPerLine,
			       unsigned int _tileWidth,
			       unsigned int _tileHeight) : 
	dpy(_dpy),
	window(_window),
	fb(_fb),
	width(_width),
	height(_height),
	bitsPerPixel(_bitsPerPixel),
	bytesPerLine(_bytesPerLine),
	tileWidth(_tileWidth),
	tileHeight(_tileHeight),
	count (0),
	scanline(NULL),
	tile(NULL)
{
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

	tilesX = (width + tileWidth - 1) / tileWidth;
	tilesY = (height + tileHeight - 1) / tileHeight;
	tileMap = new bool[tilesX * tilesY];
        tileRegionMap = new struct TileChangeRegion[tilesX * tilesY];

	unsigned int i;
	for (i = 0; i < tilesX * tilesY; i++) 
		tileMap[i] = false;

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
};


XUpdateScanner::~XUpdateScanner()
{
	XShmDetach(dpy, &shminfo_scanline);
	XDestroyImage(scanline);
	shmdt(shminfo_scanline.shmaddr);
	shmctl(shminfo_scanline.shmid, IPC_RMID, 0);
	delete tileMap;
        delete tileRegionMap;
	XShmDetach(dpy, &shminfo_tile);
	XDestroyImage(tile);
	shmdt(shminfo_tile.shmaddr);
	shmctl(shminfo_tile.shmid, IPC_RMID, 0);
}


bool XUpdateScanner::copyTile(int x, int y, int tx, int ty)
{
	unsigned int maxWidth = width - x;
	unsigned int maxHeight = height - y;
	if (maxWidth > tileWidth) 
		maxWidth = tileWidth;
	if (maxHeight > tileHeight) 
		maxHeight = tileHeight;

	if ((maxWidth == tileWidth) && (maxHeight == tileHeight)) {
		XShmGetImage(dpy, window, tile, x, y, AllPlanes);
	} else {
		XGetSubImage(dpy, window, x, y, maxWidth, maxHeight, 
			     AllPlanes, ZPixmap, tile, 0, 0);
	}
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
	
        return lastLine >= (maxHeight-1);
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

void XUpdateScanner::createHintFromTile(int x, int y, Hint &hint)
{
	unsigned int w = width - x;
	unsigned int h = height - y;
	if (w > tileWidth) 
		w = tileWidth;
	if (h > tileHeight) 
		h = tileHeight;
	
	hint.x = x;
	hint.y = y;
	hint.w = w;
	hint.h = h;
}

void XUpdateScanner::addTileToHint(int x, int y, Hint &hint)
{
	unsigned int w = width - x;
	unsigned int h = height - y;
	if (w > tileWidth) 
		w = tileWidth;
	if (h > tileHeight) 
		h = tileHeight;

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

void XUpdateScanner::flushHint(int x, int y, int &x0, 
			       Hint &hint, QPtrList<Hint> &hintList)
{
	if (x0 < 0)
		return;

	int h = 1;
	for (int i = y+1; i < tilesY; i++) {
		bool lk = true;
		for (int j = x0; j < x; j++) {
			if (!tileMap[j + i * tilesX]) {
				lk = false;
				break;
			}
		}
		if (!lk)
			break;
		
		for (int j = x0; j < x; j++) 
			tileMap[j + i * tilesX] = false;
		h++;
	}
	
	hint.h = h * tileHeight;
	if ((hint.y + hint.h) > height)
		h = height - hint.y;

	x0 = -1;

	hintList.append(new Hint(hint));
}

void XUpdateScanner::createHints(QPtrList<Hint> &hintList)
{
	Hint hint;
	int x0 = -1;

	for (int y = 0; y < tilesY; y++) {
		int x;
		for (x = 0; x < tilesX; x++) {
			if (tileMap[x + y * tilesX]) {
				if (x0 < 0) {
					createHintFromTile(x * tileWidth, 
							   y * tileHeight, 
							   hint);
					x0 = x;
				}
				else
					addTileToHint(x * tileWidth, 
						      y * tileHeight, 
						      hint);
			}
			else
				flushHint(x, y, x0, hint, hintList);
		}
		flushHint(x, y, x0, hint, hintList);
	}
}

void XUpdateScanner::searchUpdates(QPtrList<Hint> &hintList)
{
	count++;
	count %= 32;

	unsigned int i;
	unsigned int x, y;

	for (i = 0; i < (tilesX * tilesY); i++) {
		tileMap[i] = false;
	}

	y = scanlines[count];
	while (y < height) {
		XShmGetImage(dpy, window, scanline, 0, y, AllPlanes);
		x = 0;
		while (x < width) {
			int pixelsize = bitsPerPixel >> 3;
			unsigned char *src = (unsigned char*) scanline->data + 
				x * pixelsize;
			unsigned char *dest = fb + 
				y * bytesPerLine + x * pixelsize;
			int w = (x + 32) > width ? (width-x) : 32;
			if (memcmp(dest, src, w * pixelsize)) 
				tileMap[(x / tileWidth) + 
					(y / tileHeight) * tilesX] = true;
			x += 32;
		}
		y += 32;
	}

	copyAllTiles();

	createHints(hintList);
}




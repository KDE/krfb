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
 *                   Tim Jansen <tim@tjansen.de>
 */   

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include "XUpdateScanner.h"

namespace rfb {

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
			       Framebuffer *_fb,
			       unsigned int _tileWidth,
			       unsigned int _tileHeight,
			       unsigned int _blockWidth,
			       unsigned int _blockHeight) : 
	dpy(_dpy), 
	window(_window),
	fb(_fb), 
	tileWidth(32), 
	tileHeight(32), 
	blockWidth(_blockWidth), 
	blockHeight(_blockHeight), 
	count (0), 
	scanline(NULL), 
	tile(NULL)
{
	tile = XShmCreateImage(dpy,
			       DefaultVisual( dpy, 0 ),
			       fb->pixelFormat.bits_per_pixel,
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

	tilesX = (fb->width + tileWidth - 1) / tileWidth;
	tilesY = (fb->height + tileHeight - 1) / tileHeight;
	tileMap = (char*) malloc(tilesX * tilesY);
	
	unsigned int i;
	for (i = 0; i < tilesX * tilesY; i++) 
		tileMap[i] = 0;

	scanline = XShmCreateImage(dpy,
				   DefaultVisual(dpy, 0),
				   fb->pixelFormat.bits_per_pixel,
				   ZPixmap,
				   NULL,
				   &shminfo_scanline,
				   fb->width,
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
	free(tileMap);
	XShmDetach(dpy, &shminfo_tile);
	XDestroyImage(tile);
	shmdt(shminfo_tile.shmaddr);
	shmctl(shminfo_tile.shmid, IPC_RMID, 0);
}


void XUpdateScanner::checkTile(int x, int y, list<Hint> &hintList)
{
	unsigned int maxWidth = fb->width - x;
	unsigned int maxHeight = fb->height - y;
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
	bool changed = false;
	int pixelsize = fb->pixelFormat.bits_per_pixel >> 3;
	unsigned char *src = (unsigned char*) tile->data;
	unsigned char *dest = fb->data + y * fb->bytesPerLine + x * pixelsize;
	for (line = 0; line < maxHeight; line++) {
		if ( changed || memcmp(dest, src, maxWidth * pixelsize) ) {
			changed = true;
			memcpy(dest, src, maxWidth * pixelsize );
		}
		src += tile->bytes_per_line;
		dest += fb->bytesPerLine;
	}

	if (changed) {
		Hint hint;
		hint.type = hintRefresh;
		hint.hint.refresh.x = x;
		hint.hint.refresh.y = y;
		hint.hint.refresh.width = maxWidth;
		hint.hint.refresh.height = maxHeight;
		hintList.push_back(hint);
	}
}

void XUpdateScanner::searchUpdates(list<Hint> &hintList, 
				   bool showMousePointer)
{
	count %= 32;
	unsigned int i;
	unsigned int x, y;

	for (i = 0; i < (tilesX * tilesY); i++) {
		tileMap[i] = 0;
	}

	y = scanlines[count];
	while (y < fb->height) {
		XShmGetImage(dpy, window, scanline, 0, y, AllPlanes);
		x = 0;
		while (x < fb->width) {
			int pixelsize = fb->pixelFormat.bits_per_pixel >> 3;
			unsigned char *src = (unsigned char*) scanline->data + 
				x * pixelsize;
			unsigned char *dest = fb->data + 
				y * fb->bytesPerLine + x * pixelsize;
			if (memcmp(dest, src, 32 * pixelsize)) {
				tileMap[(x / tileWidth) + 
					(y / tileHeight) * tilesX] = 1;
			}
			x += 32;
		}
		y += 32;
	}

	for (y = 0; y < tilesY; y++) {
		for (x = 0; x < tilesX; x++) {
			if (tileMap[x + y * tilesX] > 0)
				checkTile(x*tileWidth, y*tileHeight, 
					  hintList);
		}
	}

	count++;
}

} // namespace rfb



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

#ifndef _hexonet_rfb_XUpdateScanner_h_
#define _hexonet_rfb_XUpdateScanner_h_

#include <qptrlist.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>


class Hint {
 public:
	int x, y, w, h;

	Hint() :
		x(0),
		y(0),
		w(0),
		h(0)
	{}
	Hint(Hint &h) :
		x(h.x),
		y(h.y),
		w(h.w),
		h(h.h)
	{
	}
	int left() {
		return x;
	}
	int right() {
		return x+w;
	}
	int top() {
		return y;
	}
	int bottom() {
		return y+h;
	}
};

struct TileChangeRegion {
  short firstLine, lastLine;
};


class XUpdateScanner
{
 public:
	XUpdateScanner( Display *_dpy,
			Window _window,
			unsigned char *_fb,
			int _width, int _height,
			int _bitsPerPixel, int _bytesPerLine,
			bool useXShm);

	~XUpdateScanner();

	// hitList: returns list of changes
	// ptrY: ptrY: position of the cursor
	void searchUpdates( QPtrList<Hint> &hintList, int ptrY);

 private:
	void testScanline(int y, bool rememberHits);
	bool copyTile(int x, int y, int tx, int ty);
	void copyAllTiles();
	void flushHint(int x, int y, int &x0, Hint &hint,
		       QPtrList<Hint> &hintList);
	void createHints(QPtrList<Hint> &hintList);
	void addTileToHint(int x, int y, int th, Hint &hint);
	void createHintFromTile(int x, int y, int th, Hint &hint);

	Display *dpy;
	Window window;
	unsigned char *fb;
	int width, height;
	int bitsPerPixel, bytesPerLine;
	unsigned int tileWidth, tileHeight;
	unsigned int count;
	bool useShm;

	XImage *scanline;
	XShmSegmentInfo shminfo_scanline;

	XImage *tile;
	XShmSegmentInfo shminfo_tile;

	unsigned int tilesX, tilesY;
	bool *tileMap;
	struct TileChangeRegion *tileRegionMap;
};


#endif // _hexonet_rfb_XUpdateScanner_h_

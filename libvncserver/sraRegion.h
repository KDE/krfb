#ifndef SRAREGION_H
#define SRAREGION_H

/* -=- SRA - Simple Region Algorithm
 * A simple rectangular region implementation.
 * Copyright (c) 2001 James "Wez" Weatherall, Johannes E. Schindelin
 */

/* -=- sraRect */

typedef struct _rect {
	int x1;
	int y1;
	int x2;
	int y2;
} sraRect;

typedef struct sraRegion sraRegion;

/* -=- Region manipulation functions */

extern sraRegion *sraRgnCreate();
extern sraRegion *sraRgnCreateRect(int x1, int y1, int x2, int y2);
extern sraRegion *sraRgnCreateRgn(const sraRegion *src);

extern void sraRgnDestroy(sraRegion *rgn);
extern void sraRgnMakeEmpty(sraRegion *rgn);
extern Bool sraRgnAnd(sraRegion *dst, const sraRegion *src);
extern void sraRgnOr(sraRegion *dst, const sraRegion *src);
extern Bool sraRgnSubtract(sraRegion *dst, const sraRegion *src);

extern void sraRgnOffset(sraRegion *dst, int dx, int dy);

extern Bool sraRgnPopRect(sraRegion *region, sraRect *rect,
			  unsigned long flags);

extern unsigned long sraRgnCountRects(const sraRegion *rgn);
extern Bool sraRgnEmpty(const sraRegion *rgn);

/* -=- rectangle iterator */

typedef struct sraRectangleIterator {
  Bool reverseX,reverseY;
  int ptrSize,ptrPos;
  struct sraSpan** sPtrs;
} sraRectangleIterator;

extern sraRectangleIterator *sraRgnGetIterator(sraRegion *s);
extern sraRectangleIterator *sraRgnGetReverseIterator(sraRegion *s,Bool reverseX,Bool reverseY);
extern Bool sraRgnIteratorNext(sraRectangleIterator *i,sraRect *r);
extern void sraRgnReleaseIterator(sraRectangleIterator *i);

void sraRgnPrint(const sraRegion *s);

/* -=- Rectangle clipper (for speed) */

extern Bool sraClipRect(int *x, int *y, int *w, int *h,
			int cx, int cy, int cw, int ch);

#endif

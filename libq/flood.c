
#include "libq.h"
#include <stdio.h>
#include <stdlib.h>
#include "flood.h"

#include "flood.h"

typedef struct _QFlood8 QFlood8;

struct _QFlood8 {
	uint8_t *area;
	int width;
	int height;
	QRectangle clip;
	int style;
	int from;
	int to;
	QPoint *stack;
	int stack_size;
	int pointer;
	int left;
	int top;
	int right;
	int bottom;
};

static int flood_pop8(QFlood8 *f,QPoint *p);
static void flood_push8(QFlood8 *f,int x,int y);
static int flood_vseek8(QFlood8 *f,int x,int y);


int q_flood_fill8(uint8_t *area,int w,int h,int x,int y,int t,QRectangle *b,int st) {
	QFlood8 f = {
		area: area,
		width: w,
		height: h,
		clip: { x: 0, y: 0, w: w, h: h },
		style: st
	};
	QPoint p;
	int x1 = 0,y1 = 0,n = 0;
	if(x<0 || y<0 || x>=w || y>=h) return 0;
	if(b!=NULL) {
		if(b->x>=w || b->y>=h || b->w<=0 || b->h<=0 || b->x+b->w<=0 || b->y+b->h<=0) return 0;
		f.clip = q_rect_intersection(&f.clip,b);
		if(f.clip.w<=0 || f.clip.h<=0) return 0;
		if((f.style&Q_FLOOD_HWRAP) && (f.clip.x>0 || f.clip.w<w)) f.style &= ~Q_FLOOD_HWRAP;
		if((f.style&Q_FLOOD_VWRAP) && (f.clip.y>0 || f.clip.h<h)) f.style &= ~Q_FLOOD_VWRAP;
//fprintf(stderr,"q_flood_fill8(cl.x: %d, cl.y: %d, cl.w: %d, cl.h: %d)\n",f.clip.x,f.clip.y,f.clip.w,f.clip.h);
	}
	f.from = (int)f.area[x+y*f.width];
	f.to = t;
	if(f.to==f.from) return 0;
	f.stack_size = f.height*4;
	f.stack = (QPoint *)q_malloc(f.stack_size*sizeof(QPoint));
	f.pointer = 0;
	f.left = f.right = x;
	f.top = f.bottom = y;
	flood_push8(&f,x,y);
	while(flood_pop8(&f,&p)) {
		if((int)f.area[p.x+p.y*f.width]!=f.from) continue;
		x1 = p.x;
		y1 = p.y;
		while(1) {
			f.area[x1+y1*f.width] = (uint8_t)f.to;
			n++;
			if(x1<f.left) f.left = x1;
			else if(x1>f.right) f.right = x1;
			if(y1<f.top) f.top = y1;
			else if(y1>f.bottom) f.bottom = y1;

			flood_push8(&f,x1-1,y1);
			flood_push8(&f,x1+1,y1);
			if((f.style&Q_FLOOD_8DIRS)) {
				flood_push8(&f,x1-1,y1-1);
				flood_push8(&f,x1+1,y1-1);
				flood_push8(&f,x1-1,y1+1);
				flood_push8(&f,x1+1,y1+1);
			}

			y1++;
			if((f.style&Q_FLOOD_VWRAP)) {
				if(y1>=f.height) y1 -= f.height;
			} else {
				if(y1>=f.clip.y+f.clip.h) break;
			}
			if((int)f.area[x1+y1*f.width]!=f.from) break;
		}
	}
	f.left = f.left<0? f.left+f.width : (f.left>=f.width? f.left-f.width : f.left);
	f.top = f.top<0? f.top+f.height : (f.top>=f.height? f.top-f.height : f.top);
	f.right = f.right<0? f.right+f.width : (f.right>=f.width? f.right-f.width : f.right);
	f.bottom = f.bottom<0? f.bottom+f.height : (f.bottom>=f.height? f.bottom-f.height : f.bottom);
	if(b!=NULL) {
		if(f.left<f.right) b->x = f.left,b->width = f.right-f.left+1;
		else b->x = f.right,b->width = f.left-f.right+1;
		if(f.top<f.bottom) b->y = f.top,b->height = f.bottom-f.top+1;
		else b->y = f.bottom,b->height = f.top-f.bottom+1;
	}
	if(f.stack!=NULL) q_free(f.stack);
	return n;
}

static int flood_pop8(QFlood8 *f,QPoint *p) {
	if(f->pointer==0) return 0;
	*p = f->stack[--f->pointer];
	return 1;
}

static void flood_push8(QFlood8 *f,int x,int y) {
	if((f->style&Q_FLOOD_HWRAP)) {
		if(x<0) x += f->width;
		else if(x>=f->width) x -= f->width;
	}
	if(x>=f->clip.x && y>=f->clip.y && x<f->clip.x+f->clip.w && y<f->clip.y+f->clip.h) {
		if((int)f->area[x+y*f->width]==f->from) {
			y = flood_vseek8(f,x,y);
			if(f->pointer==f->stack_size) {
				f->stack_size += f->stack_size;
fprintf(stderr,"flood_push8(stack_size: %d)\n",f->stack_size);
				f->stack = (QPoint *)q_realloc(f->stack,f->stack_size*sizeof(QPoint));
			}
			f->stack[f->pointer++] = (QPoint){x,y};
		}
	}
}

static int flood_vseek8(QFlood8 *f,int x,int y) {
	if((f->style&Q_FLOOD_VWRAP)) {
		int y1 = y;
		while((int)f->area[x+(y>0? y-1 : f->height-1)*f->width]==f->from) {
			--y;
			if(y<0) y = f->height-1;
			if(y==y1) break;
		}
	} else {
		while(y>f->clip.y && (int)f->area[x+(y-1)*f->width]==f->from) --y;
	}
	return y;
}



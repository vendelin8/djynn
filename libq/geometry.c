
#include <math.h>
#include <stdlib.h>
#include "geometry.h"


int q_point_equals(const QPoint *pt1,const QPoint *pt2) {
	return pt1!=NULL && pt2!=NULL && 
		pt1->x==pt2->x && pt1->y==pt2->y;
}

int q_point_inside(const QPoint *pt,int x,int y,int w,int h) {
	return pt!=NULL &&
		pt->x>=x && pt->y>=y && pt->x<x+w && pt->y<y+h;
}

int q_point_in_rect(const QPoint *pt,const QRectangle *r) {
	return pt!=NULL && r!=NULL && 
		pt->x>=r->x && pt->y>=r->y && pt->x<r->x+r->w && pt->y<r->y+r->h;
}


int q_rect_equals(const QRectangle *r1,const QRectangle *r2) {
	return r1!=NULL && r2!=NULL &&
		r1->x==r2->x && r1->y==r2->y && r1->w==r2->w && r1->h==r2->h;
}

QRectangle q_rect_ltrb_to_xywh(int l,int t,int r,int b) {
	return (QRectangle){
		x: (l<r?l:r),
		y: (t<b?t:b),
		w: abs(r-l),
		h: abs(b-t)
	};
}

QRectangle q_rect_xywh_to_ltrb(int x,int y,int w,int h) {
	return (QRectangle){
		l: (w<0?x+w:x),
		t: (h<0?y+h:y),
		r: (w<0?x:x+w),
		b: (h<0?y:y+h)
	};
}

QRectangle q_rect_xywh_from_points(const QPoint *p1,const QPoint *p2) {
	return q_rect_ltrb_to_xywh(p1->x,p1->y,p2->x,p2->y);
}

QRectangle q_rect_union(const QRectangle *r1,const QRectangle *r2) {
	if(r1!=NULL && r2!=NULL) {
		if(r1->w<0 || r1->h<0 || r2->w<0 || r2->h<0) {
			QRectangle r3 = *r1;
			QRectangle r4 = *r2;
			if(r3.w<0) r3.x += r3.w,r3.w = -r3.w;
			if(r3.h<0) r3.y += r3.h,r3.h = -r3.h;
			if(r4.w<0) r4.x += r4.w,r4.w = -r4.w;
			if(r4.h<0) r4.y += r4.h,r4.h = -r4.h;
			return q_rect_union(&r3,&r4);
		} else {
			QRectangle r = *r1;
			if(r2->x<r1->x) r.x = r2->x,r.w += (r1->x-r2->x);
			if(r2->x+r2->w>r1->x+r1->w) r.w += (r2->x+r2->w)-(r1->x+r1->w);
			if(r2->y<r1->y) r.y = r2->y,r.h += (r1->y-r2->y);
			if(r2->y+r2->h>r1->y+r1->h) r.h += (r2->y+r2->h)-(r1->y+r1->h);
			return r;
		}
	}
	else if(r1!=NULL) return *r1;
	else if(r2!=NULL) return *r2;
	else return (QRectangle){ x: 0, y: 0, w: 0, h: 0 };
}

QRectangle q_rect_intersection(const QRectangle *r1,const QRectangle *r2) {
	if(r1!=NULL && r2!=NULL) {
		if(r1->w<0 || r1->h<0 || r2->w<0 || r2->h<0) {
			QRectangle r3 = *r1;
			QRectangle r4 = *r2;
			if(r3.w<0) r3.x += r3.w,r3.w = -r3.w;
			if(r3.h<0) r3.y += r3.h,r3.h = -r3.h;
			if(r4.w<0) r4.x += r4.w,r4.w = -r4.w;
			if(r4.h<0) r4.y += r4.h,r4.h = -r4.h;
			return q_rect_intersection(&r3,&r4);
		} else {
			return (QRectangle){
				x: (r1->x>r2->x? r1->x : r2->x),
				y: (r1->y>r2->y? r1->y : r2->y),
				w: (r1->x+r1->w>r2->x+r2->w? r2->x+r2->w : r1->x+r1->w)-(r1->x>r2->x? r1->x : r2->x),
				h: (r1->y+r1->h>r2->y+r2->h? r2->y+r2->h : r1->y+r1->h)-(r1->y>r2->y? r1->y : r2->y)
			};
		}
	}
	else if(r1!=NULL) return *r1;
	else if(r2!=NULL) return *r2;
	else return (QRectangle){ x: 0, y: 0, w: 0, h: 0 };
}

void q_rect_cut(QRectangle *r,int w,int h) {
	if(r->x<0) r->w += r->x,r->x = 0;
	if(r->y<0) r->h += r->y,r->y = 0;
	if(r->x+r->w>w) r->w = w-r->x;
	if(r->y+r->h>h) r->h = h-r->y;
}

void q_rect_grow(QRectangle *r,int n) {
	if(r!=NULL) {
		r->x -= n;
		r->y -= n;
		r->w += n+n;
		r->h += n+n;
	}
}

void q_rect_shrink(QRectangle *r,int n) {
	if(r!=NULL) {
		r->x += n;
		r->y += n;
		r->w -= n+n;
		r->h -= n+n;
	}
}

int q_rect_contains(const QRectangle *r,int x,int y) {
	return r!=NULL &&
		r->x<=x && r->x+r->w>=x && r->y<=y && r->y+r->h>=y;
}

int q_rect_contains_point(const QRectangle *r,const QPoint *pt) {
	return r!=NULL && pt!=NULL &&
		r->x<=pt->x && r->x+r->w>=pt->x && r->y<=pt->y && r->y+r->h>=pt->y;
}

int q_rect_contains_rect(const QRectangle *r1,const QRectangle *r2) {
	return r1!=NULL && r2!=NULL &&
		r1->x<=r2->x && r1->x+r1->w>=r2->x+r2->w && r1->y<=r2->y && r1->y+r1->h>=r2->y+r2->h;
}

int q_rect_intersects(const QRectangle *r,int x,int y,int w,int h) {
	return r!=NULL &&
		r->x<x+w && r->x+r->w>x && r->y<y+h && r->y+r->h>y;
}

int q_rect_intersects_rect(const QRectangle *r1,const QRectangle *r2) {
	return r1!=NULL && r2!=NULL &&
		r1->x<r2->x+r2->w && r1->x+r1->w>r2->x && r1->y<r2->y+r2->h && r1->y+r1->h>r2->y;
}



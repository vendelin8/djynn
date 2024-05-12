#ifndef _LIBQ_GEOMETRY_H_
#define _LIBQ_GEOMETRY_H_

/**
 * @file libq/geometry.h
 * @author Per Löwgren
 * @date Modified: 2015-07-17
 * @date Created: 2015-06-16
 */

typedef struct _QPoint QPoint;
typedef struct _QDimension QDimension;
typedef struct _QRectangle QRectangle;

struct _QPoint {
	int x;
	int y;
};

struct _QDimension {
	union {
		int w;
		int width;
	};
	union {
		int h;
		int height;
	};
};

struct _QRectangle {
	union {
		int x;
		int l;
		int left;
	};
	union {
		int y;
		int t;
		int top;
	};
	union {
		int w;
		int width;
		int r;
		int right;
	};
	union {
		int h;
		int height;
		int b;
		int bottom;
	};
};

int q_point_equals(const QPoint *pt1,const QPoint *pt2);
int q_point_inside(const QPoint *pt,int x,int y,int w,int h);
int q_point_in_rect(const QPoint *pt,const QRectangle *r);

int q_rect_equals(const QRectangle *r1,const QRectangle *r2);
QRectangle q_rect_ltrb_to_xywh(int l,int t,int r,int b);
QRectangle q_rect_xywh_to_ltrb(int x,int y,int w,int h);
QRectangle q_rect_xywh_from_points(const QPoint *p1,const QPoint *p2);
QRectangle q_rect_union(const QRectangle *r1,const QRectangle *r2);
QRectangle q_rect_intersection(const QRectangle *r1,const QRectangle *r2);
void q_rect_cut(QRectangle *r,int w,int h);
void q_rect_grow(QRectangle *r,int n);
void q_rect_shrink(QRectangle *r,int n);
int q_rect_contains(const QRectangle *r,int x,int y);
int q_rect_contains_point(const QRectangle *r,const QPoint *pt);
int q_rect_contains_rect(const QRectangle *r1,const QRectangle *r2);
int q_rect_intersects(const QRectangle *r,int x,int y,int w,int h);
int q_rect_intersects_rect(const QRectangle *r1,const QRectangle *r2);


#endif /* _LIBQ_GEOMETRY_H_ */

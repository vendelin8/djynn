
#include "cairo.h"

void q_cairo_set_rgb(cairo_t *cr,guint32 rgb) {
	cairo_set_source_rgb(cr,
		(double)((rgb>>16)&0xff)/255.0,
		(double)((rgb>> 8)&0xff)/255.0,
		(double)( rgb     &0xff)/255.0);
}

void q_cairo_set_argb(cairo_t *cr,guint32 argb) {
	cairo_set_source_rgba(cr,
		(double)((argb>>16)&0xff)/255.0,
		(double)((argb>> 8)&0xff)/255.0,
		(double)( argb     &0xff)/255.0,
		(double)((argb>>24)&0xff)/255.0);
}

void q_cairo_line(cairo_t *cr,int x1,int y1,int x2,int y2) {
	cairo_move_to(cr,x1,y1);
	cairo_line_to(cr,x2,y2);
}


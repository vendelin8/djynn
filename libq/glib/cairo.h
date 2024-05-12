#ifndef _LIBQ_GLIB_CAIRO_H_
#define _LIBQ_GLIB_CAIRO_H_

/**
 * @file libq/glib/cairo.h
 * @author Per Löwgren
 * @date Modified: 2015-07-17
 * @date Created: 2015-07-16
 */

#include <cairo.h>
#include <glib.h>

void q_cairo_set_rgb(cairo_t *cr,guint32 rgb);
void q_cairo_set_argb(cairo_t *cr,guint32 argb);
void q_cairo_line(cairo_t *cr,int x1,int y1,int x2,int y2);
//void q_cairo_draw_rectangle(cairo_t *cr,int x,int y,int w,int h);
//void q_cairo_fill_rectangle(cairo_t *cr,int x,int y,int w,int h);

#endif /* _LIBQ_GLIB_CAIRO_H_ */

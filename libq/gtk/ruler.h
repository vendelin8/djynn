#ifndef _LIBQ_GTK_RULER_H_
#define _LIBQ_GTK_RULER_H_

/**
 * @file ligq/gtk/ruler.h
 * @author Per Löwgren
 * @date Modified: 2015-07-17
 * @date Created: 2015-07-14
 */

#include <gtk/gtk.h>
#include "widget.h"


G_BEGIN_DECLS

#define Q_TYPE_RULER           (q_ruler_get_type())
#define Q_RULER(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj),Q_TYPE_RULER,QRuler))
#define Q_RULER_CLASS(cl)      (G_TYPE_CHECK_CLASS_CAST((cl),Q_TYPE_RULER,QRulerClass))
#define Q_IS_RULER(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj),Q_TYPE_RULER))
#define Q_IS_RULER_CLASS(cl)   (G_TYPE_CHECK_CLASS_TYPE((cl),Q_TYPE_RULER))
#define Q_RULER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj),Q_TYPE_RULER,QRulerClass))


typedef struct _QRuler QRuler;
typedef struct _QRulerClass QRulerClass;
typedef struct _QRulerPrivate QRulerPrivate;
typedef struct _QRulerMetric QRulerMetric;

/* All distances below are in 1/72nd's of an inch. (According to
 * Adobe that's a point, but points are really 1/72.27 in.) */
struct _QRuler {
	QWidget widget;

	QRulerPrivate *_priv;

	GdkPixmap *pixmap;
	QRulerMetric *metric;
	gint xsrc;
	gint ysrc;
	gint slider_size;
	gdouble lower; /* The upper limit of the ruler (in points) */
	gdouble upper; /* The lower limit of the ruler */
	gdouble start; /* The start value of the ruler */
	gdouble end; /* The end value of the ruler */
	gdouble position; /* The position of the mark on the ruler */
	gdouble max_size; /* The maximum size of the ruler */
};

struct _QRulerClass {
	QWidgetClass parent_class;

	void (* draw_ticks)(QRuler *ruler);
	void (* draw_pos)(QRuler *ruler);
};

struct _QRulerMetric {
	gchar *metric_name;
	gchar *abbrev;
	/* This should be points_per_unit. This is the size of the unit
	* in 1/72nd's of an inch and has nothing to do with screen pixels */
	gdouble pixels_per_unit;
	gdouble ruler_scale[10];
	gint subdivide[5]; /* five possible modes of subdivision */
};


GType q_ruler_get_type(void) G_GNUC_CONST;
GtkWidget *q_ruler_new(GtkOrientation orientation);
void q_ruler_set_metric(QRuler *ruler,GtkMetricType metric);
GtkMetricType q_ruler_get_metric(QRuler *ruler);
void q_ruler_set_range(QRuler *ruler,gdouble lower,gdouble upper,gdouble start,gdouble end);
void q_ruler_get_range(QRuler *ruler,gdouble *lower,gdouble *upper,gdouble *start,gdouble *end);
void q_ruler_set_pos(QRuler *ruler,gdouble position);
gdouble q_ruler_get_pos(QRuler *ruler);
void q_ruler_set_max_size(QRuler *ruler,gdouble max_size);
gdouble q_ruler_get_max_size(QRuler *ruler);
void q_ruler_draw_ticks(QRuler *ruler);
void q_ruler_draw_pos(QRuler *ruler);

G_END_DECLS

#endif /* _LIBQ_GTK_RULER_H_ */


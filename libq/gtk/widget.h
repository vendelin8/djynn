#ifndef _LIBQ_GTK_WIDGET_H_
#define _LIBQ_GTK_WIDGET_H_

/**
 * @file libq/gtk/widget.h
 * @author Per Löwgren
 * @date Modified: 2015-07-17
 * @date Created: 2015-06-16
 */

#include <gtk/gtk.h>
#include "../geometry.h"


G_BEGIN_DECLS

#define Q_TYPE_WIDGET           (q_widget_get_type())
#define Q_WIDGET(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj),Q_TYPE_WIDGET,QWidget))
#define Q_WIDGET_CLASS(cl)      (G_TYPE_CHECK_CLASS_CAST((cl),Q_TYPE_WIDGET,QWidgetClass))
#define Q_IS_WIDGET(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj),Q_TYPE_WIDGET))
#define Q_IS_WIDGET_CLASS(cl)   (G_TYPE_CHECK_CLASS_TYPE((cl),Q_TYPE_WIDGET))
#define Q_WIDGET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj),Q_TYPE_WIDGET,QWidgetClass))


typedef struct _QWidget QWidget;
typedef struct _QWidgetClass QWidgetClass;
typedef struct _QWidgetPrivate QWidgetPrivate;

enum {
	Q_WIDGET_RUNNING   = 0x00000001,
	Q_WIDGET_ACTIVE    = 0x00000002,
	Q_WIDGET_FOCUS     = 0x00000004,
	Q_WIDGET_LBUTTON   = 0x00000010,
	Q_WIDGET_MBUTTON   = 0x00000020,
	Q_WIDGET_RBUTTON   = 0x00000040,
};

struct _QWidget {
	GtkWidget widget;

	QWidgetPrivate *_priv;

	guint status;
	gdouble fps;
};

struct _QWidgetClass {
	GtkWidgetClass parent_class;

	gboolean (*timeout)(QWidget *widget,guint64 tm);
	gboolean (*draw)(QWidget *widget,cairo_t *cr,QRectangle *rect);
	gboolean (*configure)(QWidget *widget,GdkEventConfigure *event);
	gboolean (*clicked)(QWidget *widget,GdkEventButton *event);
	gboolean (*motion)(QWidget *widget,GdkEventMotion *event);
	gboolean (*crossing)(QWidget *widget,GdkEventCrossing *event);
	gboolean (*key)(QWidget *widget,GdkEventKey *event);
};

GType q_widget_get_type(void) G_GNUC_CONST;
GtkWidget *q_widget_new();

gboolean q_widget_has_key_focus(GtkWidget *widget);
gboolean q_widget_key_event(GdkEventKey *event);

void q_widget_start(QWidget *widget,double fps);
void q_widget_stop(QWidget *widget);
void q_widget_activate(QWidget *widget);
gboolean q_widget_is_active(QWidget *widget);

G_END_DECLS

#endif /* _LIBQ_GTK_WIDGET_H_ */

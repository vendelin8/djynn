
#ifdef GSEAL_ENABLE
#undef GSEAL_ENABLE
#endif

#include <math.h>
#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtkprivate.h>
#include <libq/geometry.h>
#include "ruler.h"


#define RULER_WIDTH           14
#define MINIMUM_INCR          5
#define MAXIMUM_SUBDIVIDE     5
#define MAXIMUM_SCALES        10

#define ROUND(x) ((int)((x)+0.5))

enum {
	PROP_0,
	PROP_ORIENTATION,
	PROP_LOWER,
	PROP_UPPER,
	PROP_START,
	PROP_END,
	PROP_POSITION,
	PROP_MAX_SIZE,
	PROP_METRIC,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST] = { NULL };

struct _QRulerPrivate {
	GtkOrientation orientation;
};

#define S_RULER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj),Q_TYPE_RULER,QRulerPrivate))


static void q_ruler_set_property(GObject *object,guint prop_id,const GValue *value,GParamSpec *pspec);
static void q_ruler_get_property(GObject *object,guint prop_id,GValue *value,GParamSpec *pspec);
static void q_ruler_realize(GtkWidget *widget);
static void q_ruler_unrealize(GtkWidget *widget);
static void q_ruler_size_request(GtkWidget *widget,GtkRequisition *requisition);
static void q_ruler_size_allocate(GtkWidget *widget,GtkAllocation *allocation);
static void q_ruler_make_pixmap(QRuler *ruler);

static gboolean q_ruler_timeout(QWidget *widget,guint64 tm);
static gboolean q_ruler_draw(QWidget *widget,cairo_t *cr,QRectangle *rect);
static gboolean q_ruler_motion(QWidget *widget,GdkEventMotion *event);

static void q_ruler_real_draw_ticks(QRuler *ruler);
static void q_ruler_real_draw_pos(QRuler *ruler);

static const QRulerMetric q_ruler_metrics[] = {
  { "Pixels",      "pi",  1.0,  { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  { "Inches",      "in", 72.0,  { 1, 2, 4,  8, 16, 32,  64, 128, 256,  512 }, { 1, 2,  4,  8,  16 }},
  { "Centimeters", "cm", 28.35, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
};

G_DEFINE_TYPE_WITH_CODE(QRuler,q_ruler,Q_TYPE_WIDGET,G_IMPLEMENT_INTERFACE(GTK_TYPE_ORIENTABLE,NULL))


GtkWidget *q_ruler_new(GtkOrientation orientation) {
	QRuler *ruler = g_object_new(Q_TYPE_RULER,NULL);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(ruler),orientation);
	return (GtkWidget *)ruler;
}


static void q_ruler_class_init(QRulerClass *cl) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(cl);
	GtkWidgetClass *widget_class  = GTK_WIDGET_CLASS(cl);
	QWidgetClass *swidget_class  = Q_WIDGET_CLASS(cl);
	int i;

	gobject_class->set_property = q_ruler_set_property;
	gobject_class->get_property = q_ruler_get_property;

	widget_class->realize = q_ruler_realize;
	widget_class->unrealize = q_ruler_unrealize;
	widget_class->size_request = q_ruler_size_request;
	widget_class->size_allocate = q_ruler_size_allocate;

	swidget_class->timeout = q_ruler_timeout;
	swidget_class->draw = q_ruler_draw;
	swidget_class->motion = q_ruler_motion;

	cl->draw_ticks = q_ruler_real_draw_ticks;
	cl->draw_pos = q_ruler_real_draw_pos;

	g_object_class_override_property(gobject_class,PROP_ORIENTATION,"orientation");

	properties[PROP_LOWER] = g_param_spec_double("lower",_("Lower"),_("Lower limit of ruler"),-G_MAXDOUBLE,G_MAXDOUBLE,0.0,GTK_PARAM_READWRITE);
	properties[PROP_UPPER] = g_param_spec_double("upper",_("Upper"),_("Upper limit of ruler"),-G_MAXDOUBLE,G_MAXDOUBLE,0.0,GTK_PARAM_READWRITE);
	properties[PROP_START] = g_param_spec_double("start",_("Start"),_("Start value of ruler"),-G_MAXDOUBLE,G_MAXDOUBLE,0.0,GTK_PARAM_READWRITE);
	properties[PROP_END] = g_param_spec_double("end",_("End"),_("End value of ruler"),-G_MAXDOUBLE,G_MAXDOUBLE,0.0,GTK_PARAM_READWRITE);
	properties[PROP_POSITION] = g_param_spec_double("position",_("Position"),_("Position of mark on the ruler"),-G_MAXDOUBLE,G_MAXDOUBLE,0.0,GTK_PARAM_READWRITE);
	properties[PROP_MAX_SIZE] = g_param_spec_double("max-size",_("Max Size"),_("Maximum size of the ruler"),-G_MAXDOUBLE,G_MAXDOUBLE,0.0,GTK_PARAM_READWRITE);
	properties[PROP_METRIC] = g_param_spec_enum("metric",_("Metric"),_("The metric used for the ruler"),GTK_TYPE_METRIC_TYPE,GTK_PIXELS,GTK_PARAM_READWRITE);

	for(i=PROP_0; i<PROP_LAST; ++i)
		if(properties[i]!=NULL)
			g_object_class_install_property(gobject_class,i,properties[i]);

	g_type_class_add_private(gobject_class,sizeof(QRulerPrivate));
}

static void q_ruler_init(QRuler *ruler) {
	GtkWidget *widget = (GtkWidget *)ruler;
	QRulerPrivate *p = S_RULER_GET_PRIVATE(ruler);

	widget->requisition.width  = widget->style->xthickness*2+1;
	widget->requisition.height = widget->style->ythickness*2+RULER_WIDTH;

	ruler->_priv = p;
	p->orientation = GTK_ORIENTATION_HORIZONTAL;

	ruler->pixmap = NULL;
	ruler->xsrc = 0;
	ruler->ysrc = 0;
	ruler->slider_size = 0;
	ruler->lower = 0;
	ruler->upper = 0;
	ruler->start = 0;
	ruler->end = 0;
	ruler->position = 0;
	ruler->max_size = 0;

	q_ruler_set_metric(ruler,GTK_PIXELS);
}

static void q_ruler_set_property(GObject *object,guint prop_id,const GValue *value,GParamSpec *pspec) {
	QRuler *ruler = Q_RULER(object);
	switch(prop_id) {
		case PROP_ORIENTATION:
			ruler->_priv->orientation = g_value_get_enum(value);
			gtk_widget_queue_resize((GtkWidget *)ruler);
			break;
		case PROP_LOWER:
			q_ruler_set_range(ruler,g_value_get_double(value),ruler->upper,ruler->start,ruler->end);
			break;
		case PROP_UPPER:
			q_ruler_set_range(ruler,ruler->lower,g_value_get_double(value),ruler->start,ruler->end);
			break;
		case PROP_START:
			q_ruler_set_range(ruler,ruler->lower,ruler->upper,g_value_get_double(value),ruler->end);
			break;
		case PROP_END:
			q_ruler_set_range(ruler,ruler->lower,ruler->upper,ruler->start,g_value_get_double(value));
			break;
		case PROP_POSITION:
			q_ruler_set_pos(ruler,g_value_get_double(value));
			break;
		case PROP_MAX_SIZE:
			q_ruler_set_max_size(ruler,g_value_get_double(value));
			break;
		case PROP_METRIC:
			q_ruler_set_metric(ruler,g_value_get_enum(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
			break;
	}
}

static void q_ruler_get_property(GObject *object,guint prop_id,GValue *value,GParamSpec *pspec) {
	QRuler *ruler = Q_RULER(object);
	switch(prop_id) {
		case PROP_ORIENTATION:
			g_value_set_enum(value,ruler->_priv->orientation);
			break;
		case PROP_LOWER:
			g_value_set_double(value,ruler->lower);
			break;
		case PROP_UPPER:
			g_value_set_double(value,ruler->upper);
			break;
		case PROP_START:
			g_value_set_double(value,ruler->start);
			break;
		case PROP_END:
			g_value_set_double(value,ruler->end);
			break;
		case PROP_POSITION:
			g_value_set_double(value,ruler->position);
			break;
		case PROP_MAX_SIZE:
			g_value_set_double(value,ruler->max_size);
			break;
		case PROP_METRIC:
			g_value_set_enum(value,q_ruler_get_metric(ruler));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
			break;
	}
}

void q_ruler_set_metric(QRuler *ruler,GtkMetricType  metric) {
	g_return_if_fail(Q_IS_RULER(ruler));
	ruler->metric = (QRulerMetric *)&q_ruler_metrics[metric];
	if(gtk_widget_is_drawable((GtkWidget *)ruler))
		gtk_widget_queue_draw((GtkWidget *)ruler);
	g_object_notify_by_pspec((GObject *)ruler,properties[PROP_METRIC]);
}

GtkMetricType q_ruler_get_metric(QRuler *ruler) {
	gint i;
	g_return_val_if_fail(Q_IS_RULER(ruler),0);
	for(i=0; i<G_N_ELEMENTS(q_ruler_metrics); ++i)
		if(ruler->metric==&q_ruler_metrics[i])
			return i;
	g_assert_not_reached();
	return 0;
}

void q_ruler_set_range(QRuler *ruler,gdouble lower,gdouble upper,gdouble start,gdouble end) {
	g_return_if_fail(Q_IS_RULER(ruler));
	g_object_freeze_notify((GObject *)ruler);
	if(ruler->lower!=lower) {
		ruler->lower = lower;
		g_object_notify_by_pspec((GObject *)ruler,properties[PROP_LOWER]);
	}
	if(ruler->upper!=upper) {
		ruler->upper = upper;
		g_object_notify_by_pspec((GObject *)ruler,properties[PROP_UPPER]);
	}
	if(ruler->start!=start) {
		ruler->start = start;
		g_object_notify_by_pspec((GObject *)ruler,properties[PROP_START]);
	}
	if(ruler->end!=end) {
		ruler->end = end;
		g_object_notify_by_pspec((GObject *)ruler,properties[PROP_END]);
	}
	g_object_thaw_notify((GObject *)ruler);
	if(gtk_widget_is_drawable((GtkWidget *)ruler))
		gtk_widget_queue_draw((GtkWidget *)ruler);
}

void q_ruler_get_range(QRuler *ruler,gdouble *lower,gdouble *upper,gdouble *start,gdouble *end) {
	g_return_if_fail(Q_IS_RULER(ruler));
	if(lower) *lower = ruler->lower;
	if(upper) *upper = ruler->upper;
	if(start) *start = ruler->start;
	if(end) *end = ruler->end;
}

void q_ruler_set_pos(QRuler *ruler,gdouble position) {
	g_return_if_fail(Q_IS_RULER(ruler));
	if(ruler->position != position) {
		ruler->position = position;

//fprintf(stdout,"q_ruler_set_pos(%p, pos: %d)\n",ruler,(int)position);

		g_object_notify_by_pspec((GObject *)ruler,properties[PROP_POSITION]);
	}
	if(gtk_widget_is_drawable((GtkWidget *)ruler))
		q_ruler_draw_pos(ruler);
}

gdouble q_ruler_get_pos(QRuler *ruler) {
	if(Q_IS_RULER(ruler)) return ruler->position;
	return 0;
}

void q_ruler_set_max_size(QRuler *ruler,gdouble max_size) {
	g_return_if_fail(Q_IS_RULER(ruler));
	if(ruler->max_size != max_size) {
		ruler->max_size = max_size;
		g_object_notify_by_pspec((GObject *)ruler,properties[PROP_MAX_SIZE]);
	}
	if(gtk_widget_is_drawable(GTK_WIDGET(ruler)))
		gtk_widget_queue_draw(GTK_WIDGET(ruler));
}

gdouble q_ruler_get_max_size(QRuler *ruler) {
	if(Q_IS_RULER(ruler)) return ruler->max_size;
	return 0;
}

void q_ruler_draw_ticks(QRuler *ruler) {
	if(Q_IS_RULER(ruler)) {
		QRulerClass *cl = Q_RULER_GET_CLASS(ruler);
		if(cl->draw_ticks!=NULL) cl->draw_ticks(ruler);
	}
}

void q_ruler_draw_pos(QRuler *ruler) {
	if(Q_IS_RULER(ruler)) {
		QRulerClass *cl = Q_RULER_GET_CLASS(ruler);
		if(cl->draw_pos) cl->draw_pos(ruler);
	}
}

static void q_ruler_realize(GtkWidget *widget) {
	if(Q_IS_RULER(widget)) {
		GTK_WIDGET_CLASS(q_ruler_parent_class)->realize(widget);
		q_ruler_make_pixmap((QRuler *)widget);
		q_widget_start((QWidget *)widget,16);
	}
}

static void q_ruler_unrealize(GtkWidget *widget) {
	if(Q_IS_RULER(widget)) {
		QRuler *ruler = (QRuler *)widget;
		q_widget_stop((QWidget *)widget);
		if(ruler->pixmap!=NULL) {
			g_object_unref(ruler->pixmap);
			ruler->pixmap = NULL;
		}
		GTK_WIDGET_CLASS(q_ruler_parent_class)->unrealize(widget);
	}
}

static void q_ruler_size_request(GtkWidget *widget,GtkRequisition *requisition) {
	if(Q_IS_RULER(widget)) {
		QRuler *ruler = (QRuler *)widget;
		if(ruler->_priv->orientation==GTK_ORIENTATION_HORIZONTAL) {
			requisition->width = widget->style->xthickness*2+1;
			requisition->height = widget->style->ythickness*2+RULER_WIDTH;
		} else {
			requisition->width = widget->style->xthickness*2+RULER_WIDTH;
			requisition->height = widget->style->ythickness*2+1;
		}
	}
}

static void q_ruler_size_allocate(GtkWidget *widget,GtkAllocation *allocation) {
	if(Q_IS_RULER(widget)) {
		widget->allocation = *allocation;
		if(gtk_widget_get_realized(widget)) {
			gdk_window_move_resize(widget->window,allocation->x,allocation->y,allocation->width,allocation->height);
			q_ruler_make_pixmap((QRuler *)widget);
		}
	}
}

static void q_ruler_make_pixmap(QRuler *ruler) {
	GtkWidget *widget = (GtkWidget *)ruler;
	if(ruler->pixmap!=NULL) {
		gint width,height;
		gdk_pixmap_get_size(ruler->pixmap,&width,&height);
		if((width>=widget->allocation.width) && (height>=widget->allocation.height)) return;
		g_object_unref(ruler->pixmap);
	}
	ruler->pixmap = gdk_pixmap_new(widget->window,widget->allocation.width,widget->allocation.height,-1);
	ruler->xsrc = 0;
	ruler->ysrc = 0;
}

static gboolean q_ruler_timeout(QWidget *widget,guint64 tm) {
	if(Q_IS_RULER(widget) && q_widget_is_active(widget)) {
		QRuler *ruler = (QRuler *)widget;
		if(ruler->pixmap!=NULL) q_ruler_draw_pos(ruler);
	}
	return TRUE;
}

static gboolean q_ruler_draw(QWidget *widget,cairo_t *cr,QRectangle *rect) {
	if(Q_IS_RULER(widget)) {
		QRuler *ruler = (QRuler *)widget;
		q_ruler_draw_ticks(ruler);
		gdk_cairo_set_source_pixmap(cr,ruler->pixmap,0,0);
		gdk_cairo_rectangle(cr,(GdkRectangle *)rect);
		cairo_fill(cr);
		q_ruler_draw_pos(ruler);
	}
	return FALSE;
}

static gboolean q_ruler_motion(QWidget *widget,GdkEventMotion *event) {
	if(Q_IS_RULER(widget)) {
		GtkWidget *w = (GtkWidget *)widget;
		QRuler *ruler = (QRuler *)widget;
		gint pos = ruler->_priv->orientation==GTK_ORIENTATION_HORIZONTAL? (gint)event->x : (gint)event->y;
		gint len = ruler->_priv->orientation==GTK_ORIENTATION_HORIZONTAL? w->allocation.width : w->allocation.height;

		ruler->position = ruler->lower + ((ruler->upper - ruler->lower) * pos) / len;
		g_object_notify_by_pspec((GObject *)ruler,properties[PROP_POSITION]);

		q_widget_activate(widget);
	}
	return FALSE;
}

static void q_ruler_real_draw_ticks(QRuler *ruler) {
	static const double subdiv_lengths[] = { 1.0, 0.4, 0.3, 0.2, 0.1 };
	GtkWidget *widget = (GtkWidget *)ruler;
	cairo_t *cr;
	gint i,j;
	gint width,height;
	gint xthickness;
	gint ythickness;
	gint length,ideal_length;
	gdouble lower,upper; /* Upper and lower limits, in ruler units */
	gdouble start,end; /* Start and end values, in ruler units */
	gdouble increment; /* Number of pixels per unit */
	gint scale; /* Number of units per major unit */
	gdouble subd_incr;
	gdouble s,e,cur;
	gchar unit_str[32];
	gint digit_height;
	gint digit_offset;
	gint text_width;
	gint text_height;
	gint pos;
	GdkColor *color_1,*color_2,*color,*color_set;
	PangoLayout *layout;
	PangoRectangle logical_rect,ink_rect;

	if(!gtk_widget_is_drawable(widget)) return;

	xthickness = widget->style->xthickness;
	ythickness = widget->style->ythickness;

	layout = gtk_widget_create_pango_layout(widget,"012456789");
	pango_layout_get_extents(layout,&ink_rect,&logical_rect);

	digit_height = PANGO_PIXELS(ink_rect.height)+2;
	digit_offset = ink_rect.y;

	if(ruler->_priv->orientation==GTK_ORIENTATION_HORIZONTAL) {
		width = widget->allocation.width;
		height = widget->allocation.height-ythickness*2;
	} else {
		width = widget->allocation.height;
		height = widget->allocation.width-ythickness*2;
	}

	gtk_paint_box(widget->style,ruler->pixmap,GTK_STATE_NORMAL,GTK_SHADOW_OUT,NULL,widget,
		ruler->_priv->orientation==GTK_ORIENTATION_HORIZONTAL? "hruler" : "vruler",
		0,0,widget->allocation.width,widget->allocation.height);

	color_1 = &widget->style->fg[widget->state];
	color_2 = &widget->style->fg[GTK_STATE_INSENSITIVE];
	color = color_set = color_1;

	cr = gdk_cairo_create(ruler->pixmap);
	gdk_cairo_set_source_color(cr,color_set);

	if(ruler->_priv->orientation==GTK_ORIENTATION_HORIZONTAL) {
		cairo_rectangle(cr,xthickness,height+ythickness,widget->allocation.width-2*xthickness,1);
	} else {
		cairo_rectangle(cr,height+xthickness,ythickness,1,widget->allocation.height-2*ythickness);
	}

	upper = ruler->upper / ruler->metric->pixels_per_unit;
	lower = ruler->lower / ruler->metric->pixels_per_unit;
	if((upper-lower)!=0) {
		start = ruler->start / ruler->metric->pixels_per_unit;
		end = ruler->end / ruler->metric->pixels_per_unit;
		increment = (gdouble) width / (upper - lower);

		/* determine the scale H
		 *  We calculate the text size as for the vruler instead of using
		 *  text_width = gdk_string_width(font, unit_str), so that the result
		 *  for the scale looks consistent with an accompanying vruler */
		/* determine the scale V
		 *   use the maximum extents of the ruler to determine the largest
		 *   possible number to be displayed.  Calculate the height in pixels
		 *   of this displayed text. Use this height to find a scale which
		 *   leaves sufficient room for drawing the ruler. */
		scale = ceil(ruler->max_size/ruler->metric->pixels_per_unit);
		g_snprintf(unit_str,sizeof(unit_str),"%d",scale);

		if(ruler->_priv->orientation==GTK_ORIENTATION_HORIZONTAL) {
			text_width = strlen(unit_str)*digit_height+1;
			for(scale=0; scale<MAXIMUM_SCALES; ++scale)
				if(ruler->metric->ruler_scale[scale]*fabs(increment)>2*text_width) break;
		} else {
			text_height = strlen(unit_str)*digit_height+1;
			for(scale=0; scale<MAXIMUM_SCALES; ++scale)
				if(ruler->metric->ruler_scale[scale]*fabs(increment)>2*text_height) break;
		}

		if(scale==MAXIMUM_SCALES) scale = MAXIMUM_SCALES-1;

		/* drawing starts here */
		length = 0;
		for(i=MAXIMUM_SUBDIVIDE-1; i>=0; --i) {
			subd_incr = (gdouble)ruler->metric->ruler_scale[scale]/(gdouble)ruler->metric->subdivide[i];
			if(subd_incr*fabs(increment)<=MINIMUM_INCR) continue;
			/* Calculate the length of the tickmarks. Make sure that
			 * this length increases for each set of ticks */
			ideal_length = ROUND((double)height*subdiv_lengths[i]);
			if(ideal_length>++length) length = ideal_length;
			if(lower<upper) {
				s = floor(lower/subd_incr)*subd_incr;
				e = ceil(upper/subd_incr)*subd_incr;
			} else {
				double n = start;
				s = floor(upper/subd_incr)*subd_incr;
				e = ceil(lower/subd_incr)*subd_incr;
				start = end;
				end = n;
			}
			for(cur=s; cur<=e; cur+=subd_incr) {
				pos = ROUND((cur-lower)*increment);

				color = (cur<start || cur>end)? color_2 : color_1;
				if(color!=color_set) {
					color_set = color;
					cairo_fill(cr);
					gdk_cairo_set_source_color(cr,color_set);
				}

				if(ruler->_priv->orientation==GTK_ORIENTATION_HORIZONTAL) {
					cairo_rectangle(cr,pos,height+ythickness-length,1,length);
				} else {
					cairo_rectangle(cr,height+xthickness-length,pos,length,1);
				}
				/* draw label */
				if(i==0 && cur>=start && cur<=end) {
					g_snprintf(unit_str,sizeof(unit_str),"%d",(int) cur);
					if(ruler->_priv->orientation==GTK_ORIENTATION_HORIZONTAL) {
						pango_layout_set_text(layout,unit_str,-1);
						pango_layout_get_extents(layout,&logical_rect,NULL);
						gtk_paint_layout(widget->style,ruler->pixmap,gtk_widget_get_state(widget),FALSE,NULL,widget,"hruler",
							pos+2,ythickness+PANGO_PIXELS(logical_rect.y-digit_offset),layout);
					} else {
						for(j=0; j<(int)strlen(unit_str); ++j) {
							pango_layout_set_text(layout,unit_str+j,1);
							pango_layout_get_extents(layout,NULL,&logical_rect);
							gtk_paint_layout(widget->style,ruler->pixmap,gtk_widget_get_state(widget),FALSE,NULL,widget,"vruler",
								xthickness+1,pos+digit_height*j+2+PANGO_PIXELS(logical_rect.y-digit_offset),layout);
						}
					}
				}
			}
		}
		cairo_fill(cr);
	}
	cairo_destroy(cr);
	g_object_unref(layout);
}

static void q_ruler_real_draw_pos(QRuler *ruler) {
	GtkWidget *widget = (GtkWidget *)ruler;
	gint x,y;
	gint width,height;
	gint bs_width,bs_height;
	gint xthickness;
	gint ythickness;
	gdouble increment;

	if(!gtk_widget_is_drawable(widget)) return;

	xthickness = widget->style->xthickness;
	ythickness = widget->style->ythickness;
	width = widget->allocation.width;
	height = widget->allocation.height;
	if(ruler->_priv->orientation==GTK_ORIENTATION_HORIZONTAL) {
		height -= ythickness*2;
		bs_width = height/2+2;
		bs_width |= 1;  /* make sure it's odd */
		bs_height = bs_width/2+1;
	} else {
		width -= xthickness*2;
		bs_height = width/2+2;
		bs_height |= 1;  /* make sure it's odd */
		bs_width = bs_height/2+1;
	}
	if((bs_width>0) && (bs_height>0)) {
		cairo_t *cr = gdk_cairo_create(widget->window);
		/* If a backing store exists, restore the ruler */
		if(ruler->pixmap) {
			gdk_cairo_set_source_pixmap(cr,ruler->pixmap,0,0);
			cairo_rectangle(cr,ruler->xsrc,ruler->ysrc,bs_width,bs_height);
			cairo_fill(cr);
		}
		if(ruler->_priv->orientation==GTK_ORIENTATION_HORIZONTAL) {
			increment = (gdouble)width/(ruler->upper-ruler->lower);
			x = ROUND((ruler->position-ruler->lower)*increment)+(xthickness-bs_width)/2-1;
			y = (height+bs_height)/2+ythickness;
		} else {
			increment = (gdouble)height/(ruler->upper-ruler->lower);
			x = (width+bs_width)/2+xthickness;
			y = ROUND((ruler->position-ruler->lower)*increment)+(ythickness-bs_height)/2-1;
		}
		gdk_cairo_set_source_color(cr,&widget->style->fg[widget->state]);
		cairo_move_to(cr,x,y);
		if(ruler->_priv->orientation==GTK_ORIENTATION_HORIZONTAL) {
			cairo_line_to(cr,x+bs_width/2.0,y+bs_height);
			cairo_line_to(cr,x+bs_width,y);
		} else {
			cairo_line_to(cr,x+bs_width,y+bs_height/2.0);
			cairo_line_to(cr,x,y+bs_height);
		}
		cairo_fill(cr);
		cairo_destroy(cr);
		ruler->xsrc = x;
		ruler->ysrc = y;
	}
}


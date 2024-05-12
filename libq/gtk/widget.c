
#ifdef GSEAL_ENABLE
#undef GSEAL_ENABLE
#endif

#include <math.h>
#include <cairo.h>
#include <glib/gi18n.h>
#include <gtk/gtkprivate.h>
#include "widget.h"

enum {
	PROP_0,
	PROP_STATUS,
	PROP_FPS,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST] = { NULL };

struct _QWidgetPrivate {
	guint timeout_id;
};

#define Q_WIDGET_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj),Q_TYPE_WIDGET,QWidgetPrivate))


static void q_widget_set_property(GObject *object,guint prop_id,const GValue *value,GParamSpec *pspec);
static void q_widget_get_property(GObject *object,guint prop_id,GValue *value,GParamSpec *pspec);
static void q_widget_realize(GtkWidget *widget);
static gboolean q_widget_expose(GtkWidget *widget,GdkEventExpose *event);
//static gboolean q_widget_configure(GtkWidget *widget,GdkEventConfigure *event);
static gboolean q_widget_clicked(GtkWidget *widget,GdkEventButton *event);
static gboolean q_widget_motion(GtkWidget *widget,GdkEventMotion *event);
static gint q_widget_key(GtkWidget *widget,GdkEventKey *event);
static gboolean q_widget_crossing(GtkWidget *widget,GdkEventCrossing *event);
static gboolean q_widget_timeout(gpointer data);


G_DEFINE_TYPE(QWidget,q_widget,GTK_TYPE_WIDGET)


static QWidget *key_focus;


GtkWidget *q_widget_new() {
	return g_object_new(Q_TYPE_WIDGET,NULL);
}


static void q_widget_class_init(QWidgetClass *cl) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(cl);
	GtkWidgetClass *widget_class  = GTK_WIDGET_CLASS(cl);

//fprintf(stdout,"q_widget_class_init(%p)\n",cl);

	gobject_class->set_property = q_widget_set_property;
	gobject_class->get_property = q_widget_get_property;

	widget_class->realize               = q_widget_realize;
	widget_class->expose_event          = q_widget_expose;
//	widget_class->configure_event       = q_widget_configure;
	widget_class->button_press_event    = q_widget_clicked;
	widget_class->button_release_event  = q_widget_clicked;
	widget_class->motion_notify_event   = q_widget_motion;
	widget_class->key_press_event       = q_widget_key;
	widget_class->key_release_event     = q_widget_key;
	widget_class->enter_notify_event    = q_widget_crossing;
	widget_class->leave_notify_event    = q_widget_crossing;

	cl->timeout = NULL;
	cl->draw = NULL;
	cl->configure = NULL;
	cl->clicked = NULL;
	cl->motion = NULL;
	cl->crossing = NULL;
	cl->key = NULL;

	properties[PROP_STATUS] = g_param_spec_uint("status",_("Status"),_("Status of widget"),0,G_MAXUINT,0,GTK_PARAM_READABLE);
	properties[PROP_FPS] = g_param_spec_double("fps",_("FPS"),_("FPS for activity updates"),0.0,256.0,24.0,GTK_PARAM_READWRITE);

	g_object_class_install_properties(gobject_class,PROP_LAST,properties);

	g_type_class_add_private(gobject_class,sizeof(QWidgetPrivate));
}

static void q_widget_init(QWidget *widget) {
	QWidgetPrivate *p = Q_WIDGET_GET_PRIVATE(widget);

//fprintf(stdout,"q_widget_init(%p)\n",widget);

	widget->_priv = p;
	p->timeout_id = 0;

	widget->status = 0;
	widget->fps = 0.0;
}

static void q_widget_set_property(GObject *object,guint prop_id,const GValue *value,GParamSpec *pspec) {
	QWidget *widget = Q_WIDGET(object);

//fprintf(stdout,"q_widget_set_property(%p)\n",object);

	switch(prop_id) {
		case PROP_FPS:
			q_widget_start(widget,g_value_get_double(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
			break;
	}
}

static void q_widget_get_property(GObject *object,guint prop_id,GValue *value,GParamSpec *pspec) {
	QWidget *widget = Q_WIDGET(object);

//fprintf(stdout,"q_widget_get_property(%p)\n",object);

	switch(prop_id) {
		case PROP_STATUS:
			g_value_set_uint(value,widget->status);
			break;
		case PROP_FPS:
			g_value_set_double(value,widget->fps);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
			break;
	}
}

static void q_widget_realize(GtkWidget *widget) {
	if(Q_IS_WIDGET(widget)) {
		QWidget *w = (QWidget *)widget;
		QWidgetClass *cl = Q_WIDGET_GET_CLASS(widget);
		GdkWindowAttr attributes;
		gint attributes_mask;

//fprintf(stdout,"q_widget_realize(%p)\n",widget);

		gtk_widget_set_realized(widget,TRUE);

		attributes.window_type = GDK_WINDOW_CHILD;
		attributes.x = widget->allocation.x;
		attributes.y = widget->allocation.y;
		attributes.width = widget->allocation.width;
		attributes.height = widget->allocation.height;
		attributes.wclass = GDK_INPUT_OUTPUT;
		attributes.visual = gtk_widget_get_visual(widget);
		attributes.colormap = gtk_widget_get_colormap(widget);
		attributes.event_mask = gtk_widget_get_events(widget);
		if(cl->draw!=NULL)
			attributes.event_mask |= GDK_EXPOSURE_MASK;
		if(cl->clicked!=NULL)
			attributes.event_mask |= (GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK);
		if(cl->motion!=NULL)
			attributes.event_mask |= GDK_POINTER_MOTION_MASK;
		if(cl->crossing!=NULL || cl->clicked!=NULL || cl->motion!=NULL || cl->key!=NULL)
			attributes.event_mask |= (GDK_ENTER_NOTIFY_MASK|GDK_LEAVE_NOTIFY_MASK);
		if(cl->key!=NULL)
			attributes.event_mask |= (GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK);

		attributes_mask = GDK_WA_X|GDK_WA_Y|GDK_WA_VISUAL|GDK_WA_COLORMAP;

		widget->window = gdk_window_new(gtk_widget_get_parent_window(widget),&attributes,attributes_mask);
		gdk_window_set_user_data(widget->window,w);

		widget->style = gtk_style_attach(widget->style,widget->window);
		gtk_style_set_background(widget->style,widget->window,GTK_STATE_NORMAL);
	}
}


static gboolean q_widget_expose(GtkWidget *widget,GdkEventExpose *event) {

//fprintf(stdout,"q_widget_expose(%p)\n",widget);

	if(Q_IS_WIDGET(widget) && gtk_widget_is_drawable(widget)) {
		QWidgetClass *cl = Q_WIDGET_GET_CLASS(widget);
		if(cl->draw!=NULL) {
			QWidget *w = (QWidget *)widget;
			cairo_t *cr = gdk_cairo_create(widget->window);
			cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE);
			cairo_set_line_width(cr,1.0);

			cl->draw(w,cr,(QRectangle *)&event->area);

			cairo_destroy(cr);
		}
	}
	return FALSE;
}

/*static gboolean q_widget_configure(GtkWidget *widget,GdkEventConfigure *event) {

fprintf(stdout,"q_widget_configure(%p)\n",widget);

	if(Q_IS_WIDGET(widget)) {
		QWidgetClass *cl = Q_WIDGET_GET_CLASS(widget);
		if(cl->configure!=NULL)
			cl->configure((QWidget *)widget,event);
	}
	return TRUE;
}*/

static gboolean q_widget_clicked(GtkWidget *widget,GdkEventButton *event) {
	if(Q_IS_WIDGET(widget)) {
		QWidget *w = (QWidget *)widget;
		QWidgetClass *cl = Q_WIDGET_GET_CLASS(widget);
//		if((event->state&GDK_SHIFT_MASK) && (event->button==1 || event->button==3))
//			event->button = event->button==1? 3 : 1;
		if(event->type==GDK_BUTTON_PRESS) {
			if(event->button==1) w->status |= Q_WIDGET_LBUTTON;
			else if(event->button==2) w->status |= Q_WIDGET_MBUTTON;
			else if(event->button==3) w->status |= Q_WIDGET_RBUTTON;
		} else if(event->type==GDK_BUTTON_RELEASE) {
			if(event->button==1) w->status &= ~Q_WIDGET_LBUTTON;
			else if(event->button==2) w->status &= ~Q_WIDGET_MBUTTON;
			else if(event->button==3) w->status &= ~Q_WIDGET_RBUTTON;
		}
		if(cl->clicked!=NULL)
			return cl->clicked(w,event);
	}
	return FALSE;
}

static gboolean q_widget_motion(GtkWidget *widget,GdkEventMotion *event) {
	if(Q_IS_WIDGET(widget)) {
		QWidgetClass *cl = Q_WIDGET_GET_CLASS(widget);
		if(cl->motion!=NULL)
			cl->motion((QWidget *)widget,event);
	}
	return FALSE;
}

static gboolean q_widget_crossing(GtkWidget *widget,GdkEventCrossing *event) {
	if(Q_IS_WIDGET(widget)) {
		QWidget *w = (QWidget *)widget;
		QWidgetClass *cl = Q_WIDGET_GET_CLASS(widget);
		if(event->type==GDK_ENTER_NOTIFY) {
			gtk_grab_add(widget);
			w->status |= Q_WIDGET_FOCUS;
			key_focus = w;
		} else if(event->type==GDK_LEAVE_NOTIFY) {
			gtk_grab_remove(widget);
			w->status &= ~Q_WIDGET_FOCUS;
			if(key_focus==w) key_focus = NULL;
		}
		if(cl->crossing!=NULL)
			cl->crossing(w,event);
	}
	return FALSE;
}

gboolean q_widget_has_key_focus(GtkWidget *widget) {
	return widget!=NULL && Q_IS_WIDGET(widget) && key_focus==(QWidget *)widget? TRUE : FALSE;
}

gboolean q_widget_key_event(GdkEventKey *event) {
	if(key_focus!=NULL) {
		QWidgetClass *cl = Q_WIDGET_GET_CLASS(key_focus);
//fprintf(stdout,"q_widget_key_event(state: %u, keyval: %x, hardware_keycode: %x, group: %d)\n",
//					event->state,event->keyval,(int)event->hardware_keycode,(int)event->group);
		if(cl->key!=NULL)
			return cl->key(key_focus,event);
	}
	return FALSE;
}

gboolean q_widget_key(GtkWidget *widget,GdkEventKey *event) {
	if(Q_IS_WIDGET(widget)) {
		QWidgetClass *cl = Q_WIDGET_GET_CLASS(widget);
//fprintf(stdout,"q_widget_key_event(state: %u, keyval: %x, hardware_keycode: %x, group: %d)\n",
//					event->state,event->keyval,(int)event->hardware_keycode,(int)event->group);
		if(cl->key!=NULL)
			return cl->key((QWidget *)widget,event);
	}
	return FALSE;
}


void q_widget_start(QWidget *widget,double fps) {
	if(widget->_priv->timeout_id!=0) {
		if(fps==widget->fps) return;
		q_widget_stop(widget);
	}
	if(fps>0.0) {
		int millis = round(1000.0/fps);
		widget->status |= Q_WIDGET_RUNNING;
		widget->fps = fps;
		g_object_notify_by_pspec((GObject *)widget,properties[PROP_FPS]);
		widget->_priv->timeout_id = g_timeout_add(millis,q_widget_timeout,(gpointer)widget);
	}
}

void q_widget_stop(QWidget *widget) {
	if(widget->_priv->timeout_id!=0)
		g_source_remove(widget->_priv->timeout_id);
	widget->status &= ~Q_WIDGET_RUNNING;
	widget->fps = 0.0;
	widget->_priv->timeout_id = 0;
}

void q_widget_activate(QWidget *widget) {
	widget->status |= Q_WIDGET_ACTIVE;
}

gboolean q_widget_is_active(QWidget *widget) {
	return widget->status&Q_WIDGET_ACTIVE? TRUE : FALSE;
}

static gboolean q_widget_timeout(gpointer data) {
	static guint64 tm = 0;
	QWidget *widget = (QWidget *)data;
	QWidgetClass *cl = Q_WIDGET_GET_CLASS(widget);
	if(cl->timeout!=NULL) {
//fprintf(stdout,"q_widget_timeout(%lu)\n",t);
		if(cl->timeout(widget,tm))
			widget->status &= ~Q_WIDGET_ACTIVE;
	}
	++tm;
	if(widget->_priv->timeout_id==0 || !(widget->status&Q_WIDGET_RUNNING)) {
		widget->status &= ~Q_WIDGET_RUNNING;
		widget->_priv->timeout_id = 0;
		return G_SOURCE_REMOVE;
	}
	return G_SOURCE_CONTINUE;
}


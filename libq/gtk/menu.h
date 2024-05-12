#ifndef _LIBQ_GTK_MENU_H_
#define _LIBQ_GTK_MENU_H_

/**
 * @file libq/gtk/menu.h
 * @author Per Löwgren
 * @date Modified: 2015-08-27
 * @date Created: 2015-06-16
 */

#include <gtk/gtk.h>

typedef enum {
	Q_MENU_UNKNOWN,
	Q_MENU_SEPARATOR,
	Q_MENU_LABEL,
	Q_MENU_CHECKBOX,
	Q_MENU_RADIO
} QMenuItemType;

typedef enum {
	Q_MENU_APPEND,
	Q_MENU_PREPEND,
	Q_MENU_INSERT_BEFORE,
	Q_MENU_INSERT_AFTER
} QMenuInsert;

typedef struct _QMenuItem QMenuItem;

typedef void (*QMenuCallback)(QMenuItem *);

struct _QMenuItem {
	guint type;
	guint id;
	gchar *label;
	gchar *tooltip;
	gchar *icon;
	QMenuItem *submenu;
	QMenuCallback activate;
	guint index;
	QMenuItem *parent;
	GtkMenuShell *menu;
	GtkWidget *widget;
};

GtkWidget *q_menu_item_create(guint type,const gchar *label,const gchar *icon,const gchar *tooltip,GSList **group,GCallback activate,gpointer user_data);
void q_menu_create(GtkMenuShell *menu,QMenuItem items[],QMenuInsert insert,gint pos);
void q_menu_callback(QMenuCallback activate);
//void q_menu_activate(GtkMenuItem *menuitem,gpointer gdata);
gint q_menu_item_pos(GtkMenuShell *menu,GtkMenuItem *menuitem);
QMenuItem *q_menu_get(guint id);
QMenuItem *q_menu_get_previous(QMenuItem *menu_item);
QMenuItem *q_menu_get_next(QMenuItem *menu_item);
gboolean q_menu_is_checked(guint id);
#define q_menu_check(id) q_menu_set_checked(id,TRUE)
#define q_menu_uncheck(id) q_menu_set_checked(id,FALSE)
void q_menu_set_checked(guint id,gboolean check);
void q_menu_toggle_checked(guint id);
GtkWidget *q_menu_get_widget(guint id);
void q_menu_state(QMenuItem items[],guint64 disable_mask,guint64 hide_mask);

#endif /* _LIBQ_GTK_MENU_H_ */

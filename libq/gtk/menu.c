
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libq/config.h>
#include "../array.h"
#include "menu.h"

static QArray q_menu_index = NULL;
static QMenuCallback q_menu_default_callback = NULL;

static void q_menu_delete_index();

GtkWidget *q_menu_item_create(guint type,const gchar *label,const gchar *tooltip,const gchar *icon,GSList **group,GCallback activate,gpointer user_data) {
	GtkWidget *widget = NULL;
	if(type==Q_MENU_SEPARATOR) {
		widget = gtk_separator_menu_item_new();
		if(group!=NULL) *group = NULL;
	} else if(label!=NULL) {
		label = _(label);
		if(icon!=NULL) {
			GtkWidget *image = gtk_image_new_from_icon_name(icon,GTK_ICON_SIZE_MENU);
			widget = gtk_image_menu_item_new_with_label(label);
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(widget),image);
		} else if(type==Q_MENU_CHECKBOX) widget = gtk_check_menu_item_new_with_label(label);
		else if(type==Q_MENU_RADIO) widget = gtk_radio_menu_item_new_with_label(group!=NULL? *group : NULL,label);
		else widget = gtk_menu_item_new_with_label(label);
		if(type==Q_MENU_RADIO && group!=NULL) *group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(widget));
		if(strchr(label,'_')!=NULL) gtk_menu_item_set_use_underline(GTK_MENU_ITEM(widget),TRUE);
		if(tooltip!=NULL) {
			tooltip = _(tooltip);
			if(strchr(tooltip,'<')!=NULL) gtk_widget_set_tooltip_markup(widget,tooltip);
			else gtk_widget_set_tooltip_text(widget,tooltip);
		}
		if(widget!=NULL && activate!=NULL)
			g_signal_connect(widget,"activate",activate,user_data);
	}
	gtk_widget_show(widget);
	return widget;
}

void q_menu_create(GtkMenuShell *menu,QMenuItem items[],QMenuInsert insert,gint pos) {
	gint i,n;
	QMenuItem *m;
	GSList *group = NULL;

	if(items==NULL) return;

	if(q_menu_index==NULL) {
		q_menu_index = q_array_new(0,0);
		atexit(q_menu_delete_index);
	}

	if(insert==Q_MENU_INSERT_BEFORE && pos>=0) {
		--pos;
		insert = Q_MENU_INSERT_AFTER;
	}
	if(pos<-1) pos = -1;

	for(i=0; 1; ++i) {
		m = &items[i];
		m->index = i;
		m->parent = NULL;
		m->menu = menu;
		m->widget = NULL;
		if(m->type==0) break;
//debug_output("q_menu_create(%d: %s)\n",m->id,m->label);
		m->widget = q_menu_item_create(m->type,m->label,m->tooltip,m->icon,&group,NULL,NULL);
		if(m->widget!=NULL) {
			if(m->type!=Q_MENU_SEPARATOR) {
				q_array_set_pointer(q_menu_index,m->id,m);
				if(m->submenu!=NULL) {
					GtkWidget *submenu = gtk_menu_new();
					q_menu_create(GTK_MENU_SHELL(submenu),m->submenu,Q_MENU_APPEND,0);
					for(n=0; m->submenu[n].type!=0; ++n)
						m->submenu[n].parent = m;
					gtk_menu_item_set_submenu(GTK_MENU_ITEM(m->widget),submenu);
					gtk_widget_show(submenu);
				} else {
					if(m->activate==NULL) m->activate = q_menu_default_callback;
					if(m->activate!=NULL)
						g_signal_connect_swapped(m->widget,"activate",G_CALLBACK(m->activate),(gpointer)m);
				}
//debug_output("q_menu_create(%d: %s (%p))\n",m->id,m->label,q_menu_get(m->id));
			}
			if(insert==Q_MENU_APPEND)
				gtk_menu_shell_append(GTK_MENU_SHELL(menu),m->widget);
			else if(insert==Q_MENU_PREPEND)
				gtk_menu_shell_prepend(GTK_MENU_SHELL(menu),m->widget);
			else if(insert==Q_MENU_INSERT_AFTER)
				gtk_menu_shell_insert(GTK_MENU_SHELL(menu),m->widget,++pos);
		}
	}
}

static void q_menu_delete_index() {
	if(q_menu_index!=NULL) {
		q_array_free(q_menu_index);
		q_menu_index = NULL;
	}
}

void q_menu_callback(QMenuCallback activate) {
	q_menu_default_callback = activate;
}

gint q_menu_item_pos(GtkMenuShell *menu,GtkMenuItem *menuitem) {
	gint pos = -1;
	if(menu!=NULL && menuitem!=NULL) {
		GList *list = gtk_container_get_children(GTK_CONTAINER(menu));
		pos = g_list_index(list,menuitem);
		g_list_free(list);
	}
	return pos;
}

QMenuItem *q_menu_get(guint id) {
	if(q_menu_index!=NULL)
		return (QMenuItem *)q_array_index(q_menu_index,id).v;
	return NULL;
}

QMenuItem *q_menu_get_previous(QMenuItem *menu_item) {
	if(menu_item!=NULL && menu_item->id!=0 && menu_item->index>0 && menu_item->parent!=NULL) {
		QMenuItem *menu = menu_item->parent->submenu;
		QMenuItem *prev = &menu[menu_item->index-1];
		if(prev->id!=0) return prev;
	}
	return NULL;
}

QMenuItem *q_menu_get_next(QMenuItem *menu_item) {
	if(menu_item!=NULL && menu_item->id!=0 && menu_item->parent!=NULL) {
		QMenuItem *menu = menu_item->parent->submenu;
		QMenuItem *next = &menu[menu_item->index+1];
		if(next->id!=0) return next;
	}
	return NULL;
}

gboolean q_menu_is_checked(guint id) {
	QMenuItem *m = q_menu_get(id);
	if(m!=NULL)
		return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(m->widget));
	return FALSE;
}

void q_menu_toggle_checked(guint id) {
	gboolean checked = q_menu_is_checked(id);
	q_menu_set_checked(id,!checked);
}

void q_menu_set_checked(guint id,gboolean check) {
	QMenuItem *m = q_menu_get(id);
	if(m!=NULL) {
//debug_output("q_menu_set_checked(%d: %s => %d)\n",m->id,m->label,check);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(m->widget),check);
	}
}

GtkWidget *q_menu_get_widget(guint id) {
	QMenuItem *m = q_menu_get(id);
	return m!=NULL? m->widget : NULL;
}

void q_menu_state(QMenuItem items[],guint64 disable_mask,guint64 hide_mask) {
	QMenuItem *m;
	for(m=items; m->type!=0; ++m,disable_mask>>=1,hide_mask>>=1) {
		gtk_widget_set_sensitive(m->widget,(disable_mask&1)? FALSE : TRUE);
		gtk_widget_set_visible(m->widget,(hide_mask&1)? FALSE : TRUE);
	}
}


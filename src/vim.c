

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libq/array.h>
#include <libq/glib/config.h>
#include <libq/gtk/menu.h>
#include "djynn.h"
#include "vim.h"

static void menu_activate(QMenuItem *menuitem) {
	if(menuitem!=NULL) {
		gboolean check = FALSE;
		if(menuitem->type==Q_MENU_CHECKBOX || menuitem->type==Q_MENU_RADIO)
			check = q_menu_is_checked(menuitem->id);
		djynn_vim_action(menuitem->id,check);
	}
}

enum {
	VIM_KEYBOARD = 0x6000,
};

static QMenuItem vim_menu_items[] = {
/*   type          id                label & tooltip                icon   submenu  activate */
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_CHECKBOX, VIM_KEYBOARD,  N_("Use _Vim Keyboard"),NULL,  NULL,  NULL,    menu_activate },
	{ 0,0 }
};

static DjynnKeybind vim_keybindings[] = {
	{ VIM_KEYBOARD,  "vim_keyboard",  -1, N_("Toggle Vim Keyboard") },
	{ DJYNN_KEYBIND_END }
};


/*static void document_new_cb(GObject *obj,gpointer user_data) {
debug_output("document_new_cb()\n");
	djynn_vim_connect();
}

static void document_open_cb(GObject *obj,gpointer user_data) {
debug_output("document_open_cb()\n");
	djynn_vim_connect();
}

static void document_activate_cb(GObject *obj,gpointer user_data) {
debug_output("document_activate_cb()\n");
	djynn_vim_connect();
}

static gboolean editor_notify_cb(GObject *obj,GeanyEditor *ed,SCNotification *nt,gpointer ud) {
	gint code = nt->nmhdr.code;
	if(code==SCN_MODIFIED || code==SCN_CHARADDED) {
		debug_output("editor_notify_cb(code: %d, message: %d, modificationType: %x, ch: '%c', modifiers: %d, text: %s)\n",nt->nmhdr.code,nt->message,nt->modificationType,nt->ch,nt->modifiers,nt->text);
		nt->nmhdr.code = 0;
		return TRUE;
	}
	return FALSE;
}*/


void djynn_vim_init(GeanyData *data) {
	djynn_vim_create();
	if(!djynn_cfg->vim)
		djynn_vim_disable();
}

void djynn_vim_create() {
	if(djynn_widget->geany.send_selection_to_terminal!=NULL && djynn_widget->vim_menu_item==NULL) {
		q_menu_create(GTK_MENU_SHELL(djynn_widget->geany.commands_menu),vim_menu_items,Q_MENU_INSERT_AFTER,
			q_menu_item_pos(GTK_MENU_SHELL(djynn_widget->geany.commands_menu),GTK_MENU_ITEM(djynn_widget->geany.send_selection_to_terminal)));
		djynn_widget->vim_menu_item = vim_menu_items[1].widget;
	}

	if(djynn_widget->vim_status==NULL) {
		GtkWidget *label = gtk_label_new("Vim:");
		gtk_widget_set_size_request(label,120,-1);
		gtk_box_pack_start(GTK_BOX(djynn_widget->geany.statusbar_hbox),label,FALSE,TRUE,2);
		gtk_box_reorder_child(GTK_BOX(djynn_widget->geany.statusbar_hbox),label,0);
		gtk_misc_set_alignment(GTK_MISC(label),0.0f,0.5f);
		djynn_widget->vim_status = label;
	}

	djynn_vim_keyboard();
/*
	plugin_signal_connect(geany_plugin,NULL,"document-new",TRUE,(GCallback)&document_new_cb,NULL);
	plugin_signal_connect(geany_plugin,NULL,"document-open",TRUE,(GCallback)&document_open_cb,NULL);
	plugin_signal_connect(geany_plugin,NULL,"document-activate",TRUE,(GCallback)&document_activate_cb,NULL);
	plugin_signal_connect(geany_plugin,NULL,"editor-notify",TRUE,(GCallback)&editor_notify_cb,NULL);
*/
	djynn_keybind(vim_keybindings);
}


void djynn_vim_cleanup() {
	if(djynn_widget->vim_menu_item!=NULL) {
		gint i;
		for(i=0; vim_menu_items[i].type!=0; ++i)
			if(vim_menu_items[i].widget!=NULL) {
				gtk_widget_destroy(vim_menu_items[i].widget);
				vim_menu_items[i].widget = NULL;
			}
		djynn_widget->vim_menu_item = NULL;
	}
	if(djynn_widget->vim_status!=NULL) {
		gtk_widget_destroy(djynn_widget->vim_status);
		djynn_widget->vim_status = NULL;
	}
}

void djynn_vim_action(gint id,gboolean check) {
	if(!djynn_cfg->line) return;
	switch(id) {
		case VIM_KEYBOARD:
			djynn_vim_keyboard();
			break;
	}
}

gint djynn_vim_keybindings() {
	return djynn_count_keybindings(vim_keybindings);
}

void djynn_vim_enable() {
	if(djynn_widget->vim_menu_item!=NULL) {
		gint i;
		for(i=0; vim_menu_items[i].type!=0; ++i)
			if(vim_menu_items[i].widget!=NULL)
				gtk_widget_show(vim_menu_items[i].widget);
	}

	djynn_vim_keyboard();
}

void djynn_vim_disable() {
	if(djynn_widget->vim_menu_item!=NULL) {
		gint i;
		for(i=0; vim_menu_items[i].type!=0; ++i)
			if(vim_menu_items[i].widget!=NULL)
				gtk_widget_hide(vim_menu_items[i].widget);
	}
	if(djynn_widget->vim_status!=NULL)
		gtk_widget_hide(djynn_widget->vim_status);
}


void djynn_vim_keyboard() {
	gboolean use = q_menu_is_checked(VIM_KEYBOARD);
	if(use==djynn_cfg->vim_use_keyboard) return;
	djynn_cfg->vim_use_keyboard = use;
	if(use) {
		gtk_widget_show(djynn_widget->vim_status);
	} else {
		gtk_widget_hide(djynn_widget->vim_status);
	}
	q_config_open(djynn->config_filename);
	q_config_set_int(djynn_key->djynn,djynn_key->vim_use_keyboard,use);
	q_config_close();

	djynn_vim_connect();
}

void djynn_vim_connect() {
	if(djynn_cfg->vim_use_keyboard) {
debug_output("djynn_vim_connect(use)\n");
	} else {
debug_output("djynn_vim_connect(not use)\n");
	}
}

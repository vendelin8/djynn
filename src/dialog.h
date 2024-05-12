#ifndef _DJYNN_DIALOG_H_
#define _DJYNN_DIALOG_H_

/**
 * @file dialog.h
 * @author Per Löwgren
 * @date Modified: 2015-08-31
 * @date Created: 2014-05-22
 */ 

#include "geanyplugin.h"
#include "djynn.h"
#include "project.h"


typedef struct _DjynnDialog DjynnDialog;

struct _DjynnDialog {
	const gchar *title;
	gint width;
	gint height;
	GtkWidget *dialog;
	GtkWidget *ok;
};

gint djynn_msgbox_ask(const gchar *title,const gchar *msg,const gchar *item);
void djynn_msgbox_warn(const gchar *title,const gchar *msg,const gchar *item);

void djynn_workspace_dialog(gboolean create);
void djynn_session_dialog(gboolean create);
void djynn_project_dialog(DjynnProject *project,gboolean create);
void djynn_project_folder_dialog(DjynnProjectFile *file,gboolean create);
void djynn_project_file_dialog(DjynnProjectFile *file);
void djynn_project_file_properties_dialog(DjynnProjectFile *file);

void djynn_dialog(DjynnDialog *dialog,GtkWidget *vbox,guint buttons);
void djynn_dialog_buttons(GtkDialog *dialog,guint buttons,GtkWidget *list[]);
GtkWidget *djynn_frame(GtkWidget *vbox,
                       const gchar *label_text,GtkWidget **label_checkbox,gboolean label_checked,GCallback label_toggled,const gchar *label_tooltip,
                       GtkWidget *content);
GtkWidget *djynn_label(GtkWidget *vbox,GtkWidget **label_widget,
                       const gchar *label_text,gint label_width,gfloat label_align,gboolean label_bold,gint label_padding,
                       const gchar *value_text,gfloat value_align,gboolean value_bold);
GtkWidget *djynn_entry(GtkWidget *vbox,GtkWidget **label_widget,
                       const gchar *label_text,gint label_width,
                       const gchar *entry_text,const gchar *entry_tooltip);
GtkWidget *djynn_combo_box(GtkWidget *vbox,GtkWidget **label_widget,
                           const gchar *label_text,gint label_width,
                           const gchar *combo_box_text[],gint combo_box_active,const gchar *combo_box_tooltip);
GtkWidget *djynn_check_button(GtkWidget *vbox,const gchar *text,gboolean checked,GCallback toggled,const gchar *tooltip);
GtkWidget *djynn_file_browser_entry(GtkWidget *vbox,GtkWidget **label_widget,
                       const gchar *label_text,gint label_width,
                       const gchar *entry_text,const gchar *entry_tooltip);


#endif /* _DJYNN_DIALOG_H_ */



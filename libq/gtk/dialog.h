#ifndef _LIBQ_GTK_DIALOG_H_
#define _LIBQ_GTK_DIALOG_H_

/**
 * @file libq/gtk/dialog.h  
 * @author Per Löwgren
 * @date Modified: 2015-09-18
 * @date Created: 2015-06-16
 */ 

#include <gtk/gtk.h>


typedef enum {
  Q_BUTTONS_NONE                 = 0,
  Q_BUTTONS_YES                  = 0x0001,
  Q_BUTTONS_OK                   = 0x0002,
  Q_BUTTONS_CANCEL               = 0x0003,
  Q_BUTTONS_CLOSE                = 0x0004,
  Q_BUTTONS_HELP                 = 0x0005,
  Q_BUTTONS_YES_NO               = 0x0011,
  Q_BUTTONS_OK_CANCEL            = 0x0012,
  Q_BUTTONS_OPEN_CANCEL          = 0x0013,
  Q_BUTTONS_SAVE_CANCEL          = 0x0014,
  Q_BUTTONS_YES_NO_CANCEL        = 0x0111,
  Q_BUTTONS_OK_CANCEL_HELP       = 0x0112,
  Q_BUTTONS_OK_CANCEL_APPLY_HELP = 0x1111,
} QButtonsType;


GtkResponseType q_message_dialog(GtkWidget *parent,GtkMessageType type,QButtonsType buttons,char *fmt, ...);
gchar *q_prompt_dialog(GtkWidget *parent,const gchar *title,const gchar *prompt,const gchar *value);

GtkWidget *q_dialog(GtkWidget *parent,const gchar *title,
                    gint width,gint height,gboolean modal,gboolean resizable,gint padding,
                    QButtonsType buttons,GtkWidget *list[]);
void q_dialog_buttons(GtkDialog *dialog,QButtonsType buttons,GtkWidget *list[],gint default_response);
GtkWidget *q_dialog_vbox(GtkWidget *dialog,gint padding,gint spacing);

GtkWidget *q_frame(GtkWidget *vbox,
                   const gchar *label_text,GtkWidget **label_checkbox,gboolean label_checked,GCallback label_toggled,const gchar *label_tooltip,
                   GtkWidget *content);
GtkWidget *q_label(GtkWidget *vbox,GtkWidget **label_widget,
                   const gchar *label_text,gint label_width,gfloat label_align,gboolean label_bold,gint label_padding,
                   const gchar *value_text,gfloat value_align,gboolean value_bold);
GtkWidget *q_entry(GtkWidget *vbox,GtkWidget **label_widget,
                   const gchar *label_text,gint label_width,
                   const gchar *entry_text,const gchar *entry_tooltip);
GtkWidget *q_combo_box(GtkWidget *vbox,GtkWidget **label_widget,
                       const gchar *label_text,gint label_width,
                       const gchar *combo_box_text[],gint combo_box_active,const gchar *combo_box_tooltip);
GtkWidget *q_check_button(GtkWidget *vbox,const gchar *text,gboolean checked,GCallback toggled,const gchar *tooltip);
GtkWidget *q_file_browser_entry(GtkWidget *vbox,GtkWidget **label_widget,
                                const gchar *label_text,gint label_width,
                                const gchar *entry_text,const gchar *entry_tooltip,
                                GCallback browse);
GtkWidget *q_spin_button(GtkWidget *vbox,GtkWidget **label_widget,
                         const gchar *label_text,gint label_width,
                         gint spin_button_width,const gchar *spin_button_tooltip,
                         gdouble climb_rate,guint digits,gdouble value,gdouble lower,gdouble upper,
                         gdouble step_increment,gdouble page_increment,gdouble page_size);
GtkWidget *q_scroll(GtkWidget *widget,GtkPolicyType horiz,GtkPolicyType vert);
GtkWidget *q_notebook(GtkPositionType pos,gboolean show_border,gboolean scrollable,guint tab_border);
GtkWidget *q_notebook_page(GtkWidget *notebook,const gchar *label,gint border,gint spacing);

void q_dialog_spawn(gchar *argv[]);


#endif /* _LIBQ_GTK_DIALOG_H_ */


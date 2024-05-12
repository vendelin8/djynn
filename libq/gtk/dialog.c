
#include <libq/config.h>
#include "dialog.h"

static GtkWidget *create_label_widget(const gchar *text,gfloat align,gfloat valign,gboolean bold);
static GtkWidget *vbox_label_widget(GtkWidget *vbox,const gchar *text,gint width,gfloat align,gboolean bold,gint padding,
                                    GtkWidget *widget,gint widget_width);


GtkResponseType q_message_dialog(GtkWidget *parent,GtkMessageType type,QButtonsType buttons,char *fmt, ...) {
	GtkResponseType response = GTK_RESPONSE_CANCEL;
	GtkWidget *dialog;
	va_list list,copy;
	int l;
	va_start(list,fmt);
	va_copy(copy,list);
	l = vsnprintf(NULL,0,fmt,copy);
	if(l>0) {
		gchar str[l+1];
		vsprintf(str,fmt,list);
		dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
			GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,type,GTK_BUTTONS_NONE,"%s",str);
		q_dialog_buttons(GTK_DIALOG(dialog),buttons,NULL,0);
		gtk_window_set_position(GTK_WINDOW(dialog),GTK_WIN_POS_CENTER_ON_PARENT);
		response = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	va_end(copy);
	va_end(list);
	return response;
}

gchar *q_prompt_dialog(GtkWidget *parent,const gchar *title,const gchar *prompt,const gchar *value) {
	gchar *ret = NULL;
	GtkWidget *dialog,*vbox,*entry,*buttons[2];
	if(value==NULL) value = "";
	dialog = q_dialog(parent,title,-1,-1,TRUE,FALSE,0,Q_BUTTONS_OK_CANCEL,buttons);
	vbox = q_dialog_vbox(dialog,5,0);
	entry = q_entry(vbox,NULL,prompt,0,value,NULL);
	gtk_widget_show_all(dialog);
	if(gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_ACCEPT) {
		ret = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
	}
	gtk_widget_destroy(dialog);
	return ret;
}
GtkWidget *q_dialog(GtkWidget *parent,const gchar *title,
                    gint width,gint height,gboolean modal,gboolean resizable,gint padding,
                    QButtonsType buttons,GtkWidget *list[]) {
	GtkWidget *dialog = gtk_dialog_new_with_buttons(title,GTK_WINDOW(parent),GTK_DIALOG_DESTROY_WITH_PARENT,NULL);
	gtk_window_set_modal(GTK_WINDOW(dialog),modal);
	gtk_window_set_position(GTK_WINDOW(dialog),GTK_WIN_POS_CENTER_ON_PARENT);
	if(width>0 && height>0)
		gtk_window_set_default_size(GTK_WINDOW(dialog),width,height);
	if(padding>0)
		gtk_container_set_border_width(GTK_CONTAINER(dialog),padding);
	gtk_window_set_resizable(GTK_WINDOW(dialog),resizable);
	q_dialog_buttons(GTK_DIALOG(dialog),buttons,list,0);
	return dialog;
}

void q_dialog_buttons(GtkDialog *dialog,QButtonsType buttons,GtkWidget *list[],gint default_response) {
	GtkWidget *button;
	const gchar *stock;
	gint i,res;
	if(default_response==0)
		default_response = GTK_RESPONSE_ACCEPT;
	for(i=3; i>=0; --i) {
		stock = NULL;
		res = GTK_RESPONSE_NONE;
		if(i==0 && (buttons&0x000f)) {
			switch(buttons) {
				case Q_BUTTONS_YES:
				case Q_BUTTONS_YES_NO:
				case Q_BUTTONS_YES_NO_CANCEL:stock = GTK_STOCK_YES,res = GTK_RESPONSE_ACCEPT;break;
				case Q_BUTTONS_OK:
				case Q_BUTTONS_OK_CANCEL:
				case Q_BUTTONS_OK_CANCEL_HELP:
				case Q_BUTTONS_OK_CANCEL_APPLY_HELP:stock = GTK_STOCK_OK,res = GTK_RESPONSE_ACCEPT;break;
				case Q_BUTTONS_CANCEL:stock = GTK_STOCK_CANCEL,res = GTK_RESPONSE_CANCEL;break;
				case Q_BUTTONS_CLOSE:stock = GTK_STOCK_CLOSE,res = GTK_RESPONSE_CLOSE;break;
				case Q_BUTTONS_HELP:stock = GTK_STOCK_HELP,res = GTK_RESPONSE_HELP;break;
				case Q_BUTTONS_OPEN_CANCEL:stock = GTK_STOCK_OPEN,res = GTK_RESPONSE_ACCEPT;break;
				case Q_BUTTONS_SAVE_CANCEL:stock = GTK_STOCK_SAVE,res = GTK_RESPONSE_ACCEPT;break;
				default:break;
			}
		} else if(i==1 && (buttons&0x00f0)) {
			switch(buttons) {
				case Q_BUTTONS_YES_NO:
				case Q_BUTTONS_YES_NO_CANCEL:stock = GTK_STOCK_NO,res = GTK_RESPONSE_NO;break;
				case Q_BUTTONS_OK_CANCEL:
				case Q_BUTTONS_OPEN_CANCEL:
				case Q_BUTTONS_SAVE_CANCEL:
				case Q_BUTTONS_OK_CANCEL_HELP:
				case Q_BUTTONS_OK_CANCEL_APPLY_HELP:stock = GTK_STOCK_CANCEL,res = GTK_RESPONSE_CANCEL;break;
				default:break;
			}
		} else if(i==2 && (buttons&0x0f00)) {
			switch(buttons) {
				case Q_BUTTONS_YES_NO_CANCEL:stock = GTK_STOCK_CANCEL,res = GTK_RESPONSE_CANCEL;break;
				case Q_BUTTONS_OK_CANCEL_HELP:stock = GTK_STOCK_HELP,res = GTK_RESPONSE_HELP;break;
				case Q_BUTTONS_OK_CANCEL_APPLY_HELP:stock = GTK_STOCK_APPLY,res = GTK_RESPONSE_APPLY;break;
				default:break;
			}
		} else if(i==3 && (buttons&0xf000)) {
			switch(buttons) {
				case Q_BUTTONS_OK_CANCEL_APPLY_HELP:stock = GTK_STOCK_HELP,res = GTK_RESPONSE_HELP;break;
				default:break;
			}
		}
		if(stock!=NULL) {
			button = gtk_dialog_add_button(dialog,stock,res);
			if(list!=NULL) list[i] = button;
			if(default_response!=GTK_RESPONSE_NONE && default_response==res) {
				gtk_widget_set_can_default(button,TRUE);
				gtk_widget_grab_default(button);
			}
		}
	}
	if(default_response!=GTK_RESPONSE_NONE)
		gtk_dialog_set_default_response(dialog,default_response);
}

GtkWidget *q_dialog_vbox(GtkWidget *dialog,gint padding,gint spacing) {
	GtkWidget *area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget *vbox = gtk_vbox_new(FALSE,spacing);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),padding);
	gtk_box_pack_start(GTK_BOX(area),vbox,FALSE,FALSE,0);
	return vbox;
}

GtkWidget *q_frame(GtkWidget *vbox,
                   const gchar *label_text,GtkWidget **label_checkbox,gboolean label_checked,GCallback label_toggled,const gchar *label_tooltip,
                   GtkWidget *content) {
	GtkWidget *label;
	GtkWidget *frame;
	if(label_checkbox!=NULL) {
		label = gtk_check_button_new_with_label(label_text);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(label),label_checked);
		frame = gtk_frame_new(NULL);
		gtk_frame_set_label_widget(GTK_FRAME(frame),label);
		*label_checkbox = label;
	} else {
		frame = gtk_frame_new(label_text);
		label = gtk_frame_get_label_widget(GTK_FRAME(frame));
	}
	if(label_toggled!=NULL)
		g_signal_connect(label,"toggled",label_toggled,NULL);
	if(label_tooltip!=NULL && *label_tooltip!='\0')
		gtk_widget_set_tooltip_markup(label,label_tooltip);
	if(content!=NULL) {
		GtkWidget *box = gtk_vbox_new(FALSE,0);
		gtk_container_set_border_width(GTK_CONTAINER(box),6);
		gtk_box_pack_start(GTK_BOX(box),content,TRUE,TRUE,0);
		gtk_container_add(GTK_CONTAINER(frame),box);
	}
	if(vbox!=NULL)
		gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,3);
	return frame;
}

GtkWidget *q_label(GtkWidget *vbox,GtkWidget **label_widget,
                       const gchar *label_text,gint label_width,gfloat label_align,gboolean label_bold,gint label_padding,
                       const gchar *value_text,gfloat value_align,gboolean value_bold) {
	GtkWidget *label;
	GtkWidget *value_label = NULL;
	value_label = create_label_widget(value_text,value_align,0.5,value_bold);
	label = vbox_label_widget(vbox,label_text,label_width,label_align,label_bold,label_padding,value_label,0);
	if(label_widget!=NULL) *label_widget = label;
	return value_label;	
}

GtkWidget *q_entry(GtkWidget *vbox,GtkWidget **label_widget,
                       const gchar *label_text,gint label_width,
                       const gchar *entry_text,const gchar *entry_tooltip) {
	GtkWidget *label;
	GtkWidget *entry = gtk_entry_new();
	if(entry_text!=NULL && *entry_text!='\0')
		gtk_entry_set_text(GTK_ENTRY(entry),entry_text);
	gtk_entry_set_activates_default(GTK_ENTRY(entry),TRUE);
	if(entry_tooltip!=NULL && *entry_tooltip!='\0')
		gtk_widget_set_tooltip_markup(entry,entry_tooltip);
	label = vbox_label_widget(vbox,label_text,label_width,0.0,FALSE,0,entry,0);
	if(label_widget!=NULL) *label_widget = label;
	return entry;
}

GtkWidget *q_combo_box(GtkWidget *vbox,GtkWidget **label_widget,
                           const gchar *label_text,gint label_width,
                           const gchar *combo_box_text[],gint combo_box_active,const gchar *combo_box_tooltip) {
	GtkWidget *label;
	GtkWidget *combo_box = gtk_combo_box_text_new();
	gint i;
	if(combo_box_text!=NULL)
		for(i=0; combo_box_text[i]!=NULL; ++i)
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box),combo_box_text[i]);
	if(combo_box_tooltip!=NULL && *combo_box_tooltip!='\0')
		gtk_widget_set_tooltip_markup(combo_box,combo_box_tooltip);
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box),combo_box_active);
	label = vbox_label_widget(vbox,label_text,label_width,0.0,FALSE,0,combo_box,0);
	if(label_widget!=NULL) *label_widget = label;
	return combo_box;
}

GtkWidget *q_check_button(GtkWidget *vbox,const gchar *text,gboolean checked,GCallback toggled,const gchar *tooltip) {
	GtkWidget *check_button = gtk_check_button_new_with_label(text);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button),checked);
	if(toggled!=NULL)
		g_signal_connect(check_button,"toggled",toggled,NULL);
	if(tooltip!=NULL && *tooltip!='\0')
		gtk_widget_set_tooltip_markup(check_button,tooltip);
	if(vbox!=NULL)
		gtk_box_pack_start(GTK_BOX(vbox),check_button,FALSE,FALSE,0);
	return check_button;
}

GtkWidget *q_file_browser_entry(GtkWidget *vbox,GtkWidget **label_widget,
                       const gchar *label_text,gint label_width,
                       const gchar *entry_text,const gchar *entry_tooltip,
                       GCallback browse) {
	GtkWidget *label;
	GtkWidget *hbox = gtk_hbox_new(FALSE,5);
	GtkWidget *entry = gtk_entry_new();
	GtkWidget *button = gtk_button_new_with_label(_("Browse..."));
	gtk_entry_set_activates_default(GTK_ENTRY(entry),TRUE);
	if(entry_text!=NULL && *entry_text!='\0')
		gtk_entry_set_text(GTK_ENTRY(entry),entry_text);
	if(entry_tooltip!=NULL && *entry_tooltip!='\0')
		gtk_widget_set_tooltip_markup(entry,entry_tooltip);
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,0);
	g_signal_connect(button,"clicked",browse,entry);
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
	label = vbox_label_widget(vbox,label_text,label_width,0.0,FALSE,0,hbox,0);
	if(label_widget!=NULL) *label_widget = label;
	return entry;
}

GtkWidget *q_spin_button(GtkWidget *vbox,GtkWidget **label_widget,
                         const gchar *label_text,gint label_width,
                         gint spin_button_width,const gchar *spin_button_tooltip,
                         gdouble climb_rate,guint digits,gdouble value,gdouble lower,gdouble upper,
                         gdouble step_increment,gdouble page_increment,gdouble page_size) {
	GtkWidget *label;
	GtkAdjustment *adj = (GtkAdjustment *)gtk_adjustment_new(value,lower,upper,step_increment,page_increment,page_size);
	GtkWidget *spin_button = gtk_spin_button_new(adj,climb_rate,digits);
	gtk_entry_set_activates_default(GTK_ENTRY(spin_button),TRUE);
	if(spin_button_width>0)
		gtk_widget_set_size_request(spin_button,spin_button_width,-1);
	if(spin_button_tooltip!=NULL && *spin_button_tooltip!='\0')
		gtk_widget_set_tooltip_markup(spin_button,spin_button_tooltip);
	label = vbox_label_widget(vbox,label_text,label_width,0.0,FALSE,0,spin_button,spin_button_width);
	if(label_widget!=NULL) *label_widget = label;
	return spin_button;
}

GtkWidget *q_scroll(GtkWidget *widget,GtkPolicyType horiz,GtkPolicyType vert) {
	GtkWidget *scroll = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),horiz,vert);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(scroll),widget);
	return scroll;
}

GtkWidget *q_notebook(GtkPositionType pos,gboolean show_border,gboolean scrollable,guint tab_border) {
	GtkWidget *notebook = gtk_notebook_new();
	g_object_set((GObject *)notebook,
		"tab-pos",pos,
		"show-border",show_border,
		"scrollable",scrollable,
		"tab-border",tab_border,
		NULL);
	return notebook;
}

GtkWidget *q_notebook_page(GtkWidget *notebook,const gchar *label,gint border,gint spacing) {
	GtkWidget *page = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(page),border);
	gtk_box_set_spacing(GTK_BOX(page),spacing);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),page,gtk_label_new(label));
	return page;
}


void q_dialog_spawn(gchar *argv[]) {
	GError *error = NULL;
	if(!g_spawn_async(NULL,argv,NULL,G_SPAWN_SEARCH_PATH,NULL,NULL,NULL,&error)) {
		g_printerr("Error spawning process: #%d %s\n",error->code,error->message);
		g_error_free(error);
	}
}


static GtkWidget *create_label_widget(const gchar *text,gfloat align,gfloat valign,gboolean bold) {
	GtkWidget *label;
	if(bold) {
		char str[strlen(text)+17];
		label = gtk_label_new(NULL);
		sprintf(str,"<b>%s</b>",text);
		gtk_label_set_markup(GTK_LABEL(label),str);
	} else {
		label = gtk_label_new(text);
	}
	gtk_misc_set_alignment(GTK_MISC(label),align,valign);
debug_output("create_label_widget(%s %f)\n",text,align);
	return label;	
}

static GtkWidget *vbox_label_widget(GtkWidget *vbox,const gchar *text,gint width,gfloat align,gboolean bold,gint padding,
                                    GtkWidget *widget,gint widget_width) {
	GtkWidget *label = NULL;
	if(vbox!=NULL) {
		if(text!=NULL && *text!='\0') {
			label = create_label_widget(text,align,0.5,bold);
			if(width>0) {
				GtkWidget *hbox = gtk_hbox_new(FALSE,0);
				gtk_widget_set_size_request(label,width,-1);
				gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,padding);
				if(widget_width>0) {
					gtk_widget_set_size_request(widget,widget_width,-1);
					gtk_box_pack_start(GTK_BOX(hbox),widget,FALSE,TRUE,0);
				} else {
					gtk_box_pack_start(GTK_BOX(hbox),widget,TRUE,TRUE,0);
				}
				gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
			} else {
				GtkWidget *vbox2 = gtk_vbox_new(FALSE,2);
				gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,FALSE,padding);
				gtk_box_pack_start(GTK_BOX(vbox2),widget,FALSE,FALSE,0);
				gtk_box_pack_start(GTK_BOX(vbox),vbox2,FALSE,FALSE,5);
			}
		} else {
			gtk_box_pack_start(GTK_BOX(vbox),widget,FALSE,FALSE,2);
		}
	}
	return label;
}


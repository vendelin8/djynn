

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libq/base58.h>
#include <libq/base64.h>
#include <libq/string.h>
#include <libq/gtk/menu.h>
#include "djynn.h"
#include "encode.h"

static void menu_activate(QMenuItem *menuitem) {
	if(menuitem!=NULL) {
		gboolean check = FALSE;
		if(menuitem->type==Q_MENU_CHECKBOX || menuitem->type==Q_MENU_RADIO)
			check = q_menu_is_checked(menuitem->id);
		djynn_encode_action(menuitem->id,check);
	}
}

enum {
	ENCODE_DECODE = 0x4000,
	   HEXADECIMAL_ENCODE,
	   HEXADECIMAL_DECODE,
	   BASE58_ENCODE,
	   BASE58_DECODE,
	   BASE64_ENCODE,
	   BASE64_DECODE,
	   C_STRING_ESCAPE,
	   C_STRING_UNESCAPE,
	   HTML_ENCODE,
	   HTML_DECODE,
	   URL_ENCODE,
	   URL_DECODE,
};

static QMenuItem encode_submenu[] = {
/*   type          id                   label & tooltip                    icon  submenu   activate */
	{ Q_MENU_LABEL, HEXADECIMAL_ENCODE,  N_("Encode Hexadecimal"),NULL,     NULL, NULL,     menu_activate },
	{ Q_MENU_LABEL, HEXADECIMAL_DECODE,  N_("Decode Hexadecimal"),NULL,     NULL, NULL,     menu_activate },
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, BASE58_ENCODE,       N_("Encode Base58"),NULL,          NULL, NULL,     menu_activate },
	{ Q_MENU_LABEL, BASE58_DECODE,       N_("Decode Base58"),NULL,          NULL, NULL,     menu_activate },
	{ Q_MENU_LABEL, BASE64_ENCODE,       N_("Encode Base64"),NULL,          NULL, NULL,     menu_activate },
	{ Q_MENU_LABEL, BASE64_DECODE,       N_("Decode Base64"),NULL,          NULL, NULL,     menu_activate },
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, C_STRING_ESCAPE,     N_("C String Escape"),NULL,        NULL, NULL,     menu_activate },
	{ Q_MENU_LABEL, C_STRING_UNESCAPE,   N_("C String Unescape"),NULL,      NULL, NULL,     menu_activate },
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, HTML_ENCODE,         N_("HTML Encode"),NULL,            NULL, NULL,     menu_activate },
	{ Q_MENU_LABEL, HTML_DECODE,         N_("HTML Decode"),NULL,            NULL, NULL,     menu_activate },
	{ Q_MENU_LABEL, URL_ENCODE,          N_("URL Encode"),NULL,             NULL, NULL,     menu_activate },
	{ Q_MENU_LABEL, URL_DECODE,          N_("URL Decode"),NULL,             NULL, NULL,     menu_activate },
	{ 0,0 }
};

static QMenuItem encode_menu[] = {
/*   type          id                   label & tooltip                    icon  submenu         activate */
	{ Q_MENU_LABEL, ENCODE_DECODE,       N_("Encode & Decode"),NULL,        NULL, encode_submenu, NULL },
	{ 0,0 }
};

static DjynnKeybind encode_keybindings[] = {
	{ BASE64_ENCODE,          "b64encode",    -1, NULL },
	{ BASE64_DECODE,          "b64decode",    -1, NULL },
	{ C_STRING_ESCAPE,        "cstrescape",   -1, NULL },
	{ C_STRING_UNESCAPE,      "cstrunescape", -1, NULL },
	{ HTML_ENCODE,            "htmlencode",   -1, NULL },
	{ HTML_DECODE,            "htmldecode",   -1, NULL },
	{ DJYNN_KEYBIND_END }
};

static void encode_text(gint id) {
	DjynnTextSelection selection;
	djynn_get_text_selection(&selection,djynn_cfg->encode_doc_on_no_selection,FALSE);
	if(selection.text==NULL) return;
	if(selection.length>0) {
		gint i,n;
		gchar *s = NULL,*p,c;
		glong l;
		QString string = NULL;
		switch(id) {

			case HEXADECIMAL_ENCODE:
				s = (gchar *)malloc((selection.length*2)*sizeof(gchar)+1);
				for(i=0,p=s; i<selection.length; ++i) {
					n = (int)selection.text[i];
					*p++ = q_tox((n>>4)&0xf);
					*p++ = q_tox(n&0xf);
				}
				*p = '\0';
				break;

			case HEXADECIMAL_DECODE:
				s = (gchar *)malloc((selection.length/2+1)*sizeof(gchar)+1);
				for(i=0,p=s; i<selection.length; i+=2) {
					while((n=q_x(selection.text[i]))==0x20 && i<selection.length) ++i;
					if(i==selection.length || n==0x20) break;
					c = (n<<4)&0xf0;
					while((n=q_x(selection.text[i+1]))==0x20 && i<selection.length) ++i;
					if(i==selection.length || n==0x20) break;
					*p++ = c|(n&0xf);
				}
				*p = '\0';
				break;

			case BASE58_ENCODE:
				s = (gchar *)q_base58_encode((const void *)selection.text,selection.length,&l);
				break;

			case BASE58_DECODE:
				s = (gchar *)q_base58_decode((const char *)selection.text,selection.length,&l);
				break;

			case BASE64_ENCODE:
				s = (gchar *)q_base64_encode((const void *)selection.text,selection.length,&l);
				break;

			case BASE64_DECODE:
				s = (gchar *)q_base64_decode((const char *)selection.text,selection.length,&l);
				break;

			case C_STRING_ESCAPE:
			case C_STRING_UNESCAPE:
			case HTML_ENCODE:
			case HTML_DECODE:
			case URL_ENCODE:
			case URL_DECODE:
				string = q_string_new();
				q_string_insert(string,0,selection.text);
				switch(id) {
					case C_STRING_ESCAPE:q_string_escape(string,0,0,NULL,ESCAPE);break;
					case C_STRING_UNESCAPE:q_string_unescape(string,0,0,UNESCAPE);break;
					case HTML_ENCODE:q_string_encode_html(string,0,0,HTML_ALL);break;
					case HTML_DECODE:q_string_decode_html(string,0,0);break;
					case URL_ENCODE:q_string_encode_url(string,0,0);break;
					case URL_DECODE:q_string_decode_url(string,0,0);break;
				}
				s = (gchar *)q_string_chars(string);
				break;
		}
		if(s!=NULL) {
			if(selection.all) sci_set_text(selection.scintilla,(const gchar *)s);
			else sci_replace_sel(selection.scintilla,(const gchar *)s);
			if(string==NULL) g_free(s);
		}
		if(string!=NULL) q_string_free(string);
	}
	g_free(selection.text);
}

void djynn_encode_init(GeanyData *data) {
	djynn_encode_create();
	if(!djynn_cfg->encode)
		djynn_encode_disable();
}

void djynn_encode_create() {
	if(djynn_widget->encode_menu_item==NULL) {
		q_menu_create(GTK_MENU_SHELL(djynn_widget->geany.format_menu),encode_menu,Q_MENU_INSERT_AFTER,
			q_menu_item_pos(GTK_MENU_SHELL(djynn_widget->geany.format_menu),GTK_MENU_ITEM(djynn_widget->comment_menu_item)));
		djynn_widget->encode_menu_item = encode_menu[0].widget;
		djynn_keybind(encode_keybindings);
	}
}


void djynn_encode_cleanup() {
	if(djynn_widget->encode_menu_item!=NULL) {
		gtk_widget_destroy(djynn_widget->encode_menu_item);
		djynn_widget->encode_menu_item = NULL;
	}
}

void djynn_encode_action(gint id,gboolean check) {
	if(!djynn_cfg->encode) return;
	encode_text(id);
}

gint djynn_encode_keybindings() {
	return djynn_count_keybindings(encode_keybindings);
}

void djynn_encode_enable() {
	gtk_widget_show(djynn_widget->encode_menu_item);
}

void djynn_encode_disable() {
	gtk_widget_hide(djynn_widget->encode_menu_item);
}

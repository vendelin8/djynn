

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libq/array.h>
#include <libq/gtk/menu.h>
#include "djynn.h"
#include "line.h"

static void menu_activate(QMenuItem *menuitem) {
	if(menuitem!=NULL) {
		gboolean check = FALSE;
		if(menuitem->type==Q_MENU_CHECKBOX || menuitem->type==Q_MENU_RADIO)
			check = q_menu_is_checked(menuitem->id);
		djynn_line_action(menuitem->id,check);
	}
}

enum {
	LINE_ORDERING = 0x5000,
	   SORT_ASC,
	   SORT_ASC_CASE,
	   SORT_DESC,
	   SORT_DESC_CASE,
	   REVERSE_LINES,
	REMOVE_DUP_LINES,
};

static QMenuItem line_submenu[] = {
/*   type          id                   label & tooltip                       icon                       submenu       activate */
	{ Q_MENU_LABEL, SORT_ASC,            N_("Sort Asc. [aA-zZ]"),NULL,         GTK_STOCK_SORT_ASCENDING,  NULL,         menu_activate },
	{ Q_MENU_LABEL, SORT_ASC_CASE,       N_("Sort Asc. Case [a-z-A-Z]"),NULL,  GTK_STOCK_SORT_ASCENDING,  NULL,         menu_activate },
	{ Q_MENU_LABEL, SORT_DESC,           N_("Sort Desc. [Zz-Aa]"),NULL,        GTK_STOCK_SORT_DESCENDING, NULL,         menu_activate },
	{ Q_MENU_LABEL, SORT_DESC_CASE,      N_("Sort Desc. Case [Z-A-z-a]"),NULL, GTK_STOCK_SORT_DESCENDING, NULL,         menu_activate },
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, REVERSE_LINES,       N_("Reverse Lines"),NULL,             NULL,                      NULL,         menu_activate },
	{ 0,0 }
};

static QMenuItem line_menu[] = {
/*   type          id                   label & tooltip                       icon                       submenu       activate */
	{ Q_MENU_LABEL, LINE_ORDERING,       N_("Line Ordering"),NULL,             NULL,                      line_submenu, NULL },
	{ 0,0 }
};

static QMenuItem remove_dup_menu_item[] = {
/*   type          id                   label & tooltip                       icon                       submenu       activate */
	{ Q_MENU_LABEL, REMOVE_DUP_LINES,    N_("Remove Duplicate Lines"),NULL,    NULL,                      NULL,         menu_activate },
	{ 0,0 }
};

static DjynnKeybind line_keybindings[] = {
	{ SORT_ASC,               "sort_asc",          -1, NULL },
	{ SORT_ASC_CASE,          "sort_asc_case",     -1, NULL },
	{ SORT_DESC,              "sort_desc",         -1, NULL },
	{ SORT_DESC_CASE,         "sort_desc_case",    -1, NULL },
	{ REVERSE_LINES,          "reverse",           -1, NULL },
	{ REMOVE_DUP_LINES,       "remove_dup_lines",  -1, NULL },
	{ DJYNN_KEYBIND_END }
};

enum {
	SORT		= 1,
	CASE		= 2,
	REVERSE	= 4
};

static void sort_text(gint st) {
	DjynnTextSelection selection;
	djynn_get_text_selection(&selection,djynn_cfg->line_doc_on_no_selection,TRUE);
	if(selection.text==NULL) return;
	if(selection.length>0) {
		QArray arr = q_array_new(0,0);
		gchar *s;
		q_array_split(arr,selection.text,selection.eol,ARR_SPLIT_EMPTY_ITEMS_EXCEPT_LAST);
		if(st&SORT) q_array_sort(arr,(st&CASE)? 0 : ARR_SORT_CASE_INSENSITIVE);
		if(st&REVERSE) q_array_reverse(arr);
		//dialogs_show_msgbox(GTK_MESSAGE_INFO,"Vector[0][%s]",(char *)vec_get(vec,0));
		s = q_array_join(arr,selection.eol,ARR_JOIN_SUFFIX);
		//dialogs_show_msgbox(GTK_MESSAGE_INFO,"Sort[%s]",s);
		if(selection.all) sci_set_text(selection.scintilla,(const gchar *)s);
		else sci_replace_sel(selection.scintilla,(const gchar *)s);
		free(s);
		q_array_free(arr);
	}
	g_free(selection.text);
}

/*
static void move_lines(gboolean down) {
	GeanyDocument *doc = document_get_current();
	ScintillaObject *sci = doc->editor->sci;
	gint lines = sci_get_line_count(sci);
	gboolean sel = sci_has_selection(sci);
	gint start,end,pos,start_line,end_line,start_col,end_col,end_sel,cur;
	gchar *text = NULL;
	if(doc==NULL) return;
	else {
//debug_output("move_lines(lines: %d)\n",lines);
		cur = sci_get_current_position(sci);
		if(sel) {
			start = sci_get_selection_start(sci);
			end = sci_get_selection_end(sci);
		} else {
			start = end = cur;
		}
		end_sel = end-start;
		start_line = sci_get_line_from_position(sci,start);
		end_line = sci_get_line_from_position(sci,end);
		start_col = sci_get_col_from_position(sci,start);
		end_col = sci_get_col_from_position(sci,end);
		cur -= start;
		if(end_col>0 || start==end) ++end_line;
//debug_output("move_lines(start_line: %d, end_line: %d)\n",start_line,end_line);
		if((down && end_line<lines) || (!down && start_line>0)) {
			start = sci_get_position_from_line(sci,start_line);
			end = sci_get_position_from_line(sci,end_line);
			text = sci_get_contents_range(sci,start,end);
//debug_output("move_lines(text: %s)\n",text);
			if(text!=NULL) {
				sci_set_selection_start(sci,start);
				sci_set_selection_end(sci,end);
				sci_replace_sel(sci,"");
				pos = sci_get_position_from_line(sci,start_line+(down? 1 : -1));
				sci_set_selection_start(sci,pos);
				sci_set_selection_end(sci,pos);
				if(down && end_line>=lines-1) {
					gchar *eol;
					gint eol_len;
					gint i = scintilla_send_message(sci,SCI_GETEOLMODE,0,0);
					gint l = strlen(text);
					switch(i) {
						case SC_EOL_CRLF:eol = "\r\n",eol_len = 2;break;
						case SC_EOL_CR:eol = "\r",eol_len = 1;break;
						default:eol = "\n",eol_len = 1;break;
					}
					memmove(&text[eol_len],text,l-eol_len);
					memcpy(text,eol,eol_len);
					cur += eol_len,start_col += eol_len;
				}
				sci_replace_sel(sci,text);
				if(sel) {
					sci_set_selection_start(sci,pos+start_col);
					sci_set_selection_end(sci,pos+start_col+end_sel);
				} else {
					sci_set_current_position(sci,pos+start_col+cur,TRUE);
				}
			}
		}
	}
	if(text!=NULL) g_free(text);
}
*/

static void remove_dup_lines() {
	DjynnTextSelection selection;
	djynn_get_text_selection(&selection,djynn_cfg->line_doc_on_no_selection,TRUE);
	if(selection.text==NULL) return;
	if(selection.length>0) {
		QArray arr = q_array_new(0,0);
		QArrayIter iter;
		QType val;
		gchar *s,*p1,*p2;
		q_array_split(arr,selection.text,selection.eol,ARR_SPLIT_EMPTY_ITEMS_EXCEPT_LAST);
		for(; q_array_each(arr,&val); ) {
			iter = q_array_get_iter(arr);
			p1 = val.s;
			if(*p1!='\0') {
				for(; q_array_each(arr,&val); ) {
					p2 = val.s;
					if(*p2=='\0') continue;
					if(*p1==*p2 && !strcmp(p1,p2))
						q_array_remove(arr,NULL);
				}
			}
			q_array_set_iter(arr,iter);
		}
		//dialogs_show_msgbox(GTK_MESSAGE_INFO,"Vector[0][%s]",(char *)vec_get(vec,0));
		s = q_array_join(arr,selection.eol,ARR_JOIN_SUFFIX);
		//dialogs_show_msgbox(GTK_MESSAGE_INFO,"Sort[%s]",s);
		if(selection.all) sci_set_text(selection.scintilla,(const gchar *)s);
		else sci_replace_sel(selection.scintilla,(const gchar *)s);
		free(s);
		q_array_free(arr);
	}
	g_free(selection.text);
}

void djynn_line_init(GeanyData *data) {
	djynn_line_create();
	if(!djynn_cfg->line)
		djynn_line_disable();
}

void djynn_line_create() {
	if(djynn_widget->line_menu_item==NULL) {
		q_menu_create(GTK_MENU_SHELL(djynn_widget->geany.format_menu),line_menu,Q_MENU_INSERT_AFTER,
			q_menu_item_pos(GTK_MENU_SHELL(djynn_widget->geany.format_menu),GTK_MENU_ITEM(djynn_widget->comment_menu_item)));
		djynn_widget->line_menu_item = line_menu[0].widget;
	}
	if(djynn_widget->remove_dup_lines_menu_item==NULL) {
		q_menu_create(GTK_MENU_SHELL(djynn_widget->geany.commands_menu),remove_dup_menu_item,Q_MENU_INSERT_AFTER,
			q_menu_item_pos(GTK_MENU_SHELL(djynn_widget->geany.commands_menu),GTK_MENU_ITEM(djynn_widget->geany.duplicate_line_or_selection)));
		djynn_widget->remove_dup_lines_menu_item = remove_dup_menu_item[0].widget;
	}
	djynn_keybind(line_keybindings);
}


void djynn_line_cleanup() {
	if(djynn_widget->line_menu_item!=NULL) {
		gtk_widget_destroy(djynn_widget->line_menu_item);
		djynn_widget->line_menu_item = NULL;
	}
	if(djynn_widget->remove_dup_lines_menu_item!=NULL) {
		gtk_widget_destroy(djynn_widget->remove_dup_lines_menu_item);
		djynn_widget->remove_dup_lines_menu_item = NULL;
	}
}

void djynn_line_action(gint id,gboolean check) {
	if(!djynn_cfg->line) return;
	switch(id) {
		case SORT_ASC:
			sort_text(SORT);
			break;
		case SORT_ASC_CASE:
			sort_text(SORT|CASE);
			break;
		case SORT_DESC:
			sort_text(SORT|REVERSE);
			break;
		case SORT_DESC_CASE:
			sort_text(SORT|REVERSE|CASE);
			break;
		case REVERSE_LINES:
			sort_text(REVERSE);
			break;
		case REMOVE_DUP_LINES:
			remove_dup_lines();
			break;
	}
}

gint djynn_line_keybindings() {
	return djynn_count_keybindings(line_keybindings);
}

void djynn_line_enable() {
	gtk_widget_show(djynn_widget->line_menu_item);
	gtk_widget_show(djynn_widget->remove_dup_lines_menu_item);
}

void djynn_line_disable() {
	gtk_widget_hide(djynn_widget->line_menu_item);
	gtk_widget_hide(djynn_widget->remove_dup_lines_menu_item);
}


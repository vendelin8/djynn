#ifndef _DJYNN_H_
#define _DJYNN_H_

/**
 * @file djynn.h
 * @author Per Löwgren
 * @date Modified: 2015-08-31
 * @date Created: 2014-05-22
 */ 

#include "config.h"


enum {
	DJYNN_KEYBIND_END   = 0x0000ffff,
};

typedef struct _DjynnKeybind DjynnKeybind;
typedef struct _DjynnData DjynnData;
typedef struct _DjynnWidgets DjynnWidgets;
typedef struct _DjynnConfig DjynnConfig;
typedef struct _DjynnConfigKeys DjynnConfigKeys;
typedef struct _DjynnProjectManager DjynnProjectManager;
typedef struct _DjynnTextSelection DjynnTextSelection;

typedef struct _QMenuItem QMenuItem;

struct _DjynnKeybind {
	gint id;
	gchar *name;
	gint index;
	gchar *label;
	QMenuItem *menu_item;
};

struct _DjynnData {
	gchar *config_dir;
	gchar *config_filename;
	DjynnKeybind **keybindings;
	gint keybindings_sum;
	gint keybindings_index;
	gchar *workspace;
	gchar *workspace_key;
	DjynnConfig *config;
	DjynnConfigKeys *config_keys;
	DjynnWidgets *widgets;
	DjynnProjectManager *project_manager;
};

struct _DjynnConfig {
	gboolean project_manager;
	gboolean   project_manager_show;
	gint       project_manager_pos;
	gchar      *ext_open_cmd;
	gint       workspace_list;
	gint       session_list;
	gint       show_icons;
	gboolean   show_tree_lines;
	gboolean   single_click_open;
	gboolean   double_click_ext;
	gboolean   activate_prj;
	gboolean   activate_sidebar;
	gboolean comment;
	gboolean   replace_geany_comments;
	gboolean line;
	gboolean   line_doc_on_no_selection;
	gboolean encode;
	gboolean   encode_doc_on_no_selection;
	gboolean vim;
	gboolean   vim_use_keyboard;
};

struct _DjynnConfigKeys {
	const gchar *djynn;
	const gchar *workspace;
	const gchar *workspace_id;
	const gchar *workspace_n;
	const gchar *workspace_d;
	const gchar *session;
	const gchar *session_id;
	const gchar *session_n;
	const gchar *session_d;
	const gchar *projectmanager;
	const gchar *project;
	const gchar *project_n;
	const gchar *project_d;

	const gchar *project_manager;
	const gchar   *project_manager_show;
	const gchar   *project_manager_pos;
	const gchar   *ext_open_cmd;
	const gchar   *workspace_list;
	const gchar   *session_list;
	const gchar   *show_icons;
	const gchar   *show_tree_lines;
	const gchar   *single_click_open;
	const gchar   *double_click_ext;
	const gchar   *activate_prj;
	const gchar   *activate_sidebar;
	const gchar *comment;
	const gchar   *replace_geany_comments;
	const gchar *line;
	const gchar   *line_doc_on_no_selection;
	const gchar *encode;
	const gchar   *encode_doc_on_no_selection;
	const gchar *vim;
	const gchar   *vim_use_keyboard;
};

struct _DjynnWidgets {
	struct {
		GtkWidget *edit_menu;
		GtkWidget *commands_menu;
		GtkWidget *duplicate_line_or_selection;
		GtkWidget *send_selection_to_terminal;
		GtkWidget *format_menu_item;
		GtkWidget *format_menu;
		GtkWidget *format_menu_pos;
		GtkWidget *comment_line;
		GtkWidget *uncomment_line;
		GtkWidget *toggle_line_commentation;
		GtkWidget *view_menu;
		GtkWidget *toggle_widgets_menu_item;
		GtkWidget *show_sidebar_menu_item;
		GtkWidget *project_menu;
		GtkWidget *tools_menu;
		GtkWidget *configuration_files;
		GtkWidget *configuration_files_menu;
		GtkWidget *help_menu;
		GtkWidget *donate;
		GtkWidget *about;

		GtkWidget *statusbar_hbox;

		GtkWidget *vpaned1;
		GtkWidget *hpaned1;
	} geany;

	struct {
		GtkWidget *ext_open_cmd;
		GtkWidget *workspace_list;
		GtkWidget *session_list;
		GtkWidget *show_icons;
		GtkWidget *project_manager;
		GtkWidget   *project_manager_pos;
		GtkWidget   *show_tree_lines;
		GtkWidget   *single_click_open;
		GtkWidget   *double_click_ext;
		GtkWidget   *activate_prj;
		GtkWidget   *activate_sidebar;
		GtkWidget *comment;
		GtkWidget   *replace_geany_comments;
		GtkWidget *line;
		GtkWidget   *line_doc_on_no_selection;
		GtkWidget *encode;
		GtkWidget   *encode_doc_on_no_selection;
	} config;

	GtkWidget *remove_dup_lines_menu_item;
	GtkWidget *comment_menu_item;
	GtkWidget *comment_menu;
	GtkWidget *line_menu_item;
	GtkWidget *encode_menu_item;
	GtkWidget *vim_menu_item;
	GtkWidget *vim_status;
	GtkWidget *djynn_conf_menu_item;
};

struct _DjynnTextSelection {
	GeanyDocument *document;
	ScintillaObject *scintilla;
	gchar *text;
	gboolean selection;
	gboolean all;
	gint start;
	gint end;
	gint length;
	gchar *eol;
	gint eol_len;
};


extern DjynnData *djynn;
extern DjynnWidgets *djynn_widget;
extern DjynnConfig *djynn_cfg;
extern DjynnConfigKeys *djynn_key;
extern DjynnProjectManager *djynn_pm;

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
#if GEANY_API_VERSION < 224
extern GeanyFunctions *geany_functions;
#endif

void djynn_action(int id);

gint djynn_count_keybindings(DjynnKeybind keybindings[]);
void djynn_keybind(DjynnKeybind keybindings[]);

void djynn_get_text_selection(DjynnTextSelection *selection,gboolean all_on_no_selection,gboolean full_lines);


#endif /* _DJYNN_H_ */



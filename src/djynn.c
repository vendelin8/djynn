

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <libq/string.h>
#include <libq/glib/config.h>
#include <libq/gtk/menu.h>
#include "djynn.h"
#include "dialog.h"
#include "workspace.h"
#include "project.h"
#include "session.h"
#include "comment.h"
#include "line.h"
#include "encode.h"
#include "vim.h"

GeanyPlugin *geany_plugin;
GeanyData *geany_data;
#if GEANY_API_VERSION < 224
GeanyFunctions *geany_functions;
#endif

PLUGIN_VERSION_CHECK(GEANY_API_VERSION)

PLUGIN_SET_TRANSLATABLE_INFO(
	PACKAGE_LOCALE_DIR,
	GETTEXT_PACKAGE,
	PACKAGE_NAME,
	_("A small project manager, with some additional functionality; encoding/decoding, commenting and sorting, etc."),
	PACKAGE_VERSION,
	PACKAGE_MAINTAINER " <" PACKAGE_BUGREPORT ">");

static GeanyKeyGroup *plugin_key_group;

static void menu_activate(QMenuItem *menuitem) {
	if(menuitem!=NULL)
		djynn_action(menuitem->id);
}

enum {
	DJYNN_CONF = 0x1001,
	COMMENTS,
};

static QMenuItem conf_menu_item[] = {
	{
		Q_MENU_LABEL,
		DJYNN_CONF,
		"djynn.conf",
		NULL,
		NULL,
		NULL,
		menu_activate
	},
	{ 0,0 },
};

static QMenuItem comments_submenu[] = {{ 0,0 }};

static QMenuItem comments_menu[] = {
	{
		Q_MENU_LABEL,
		COMMENTS,
		N_("Comments"),
		NULL,
		NULL,
		comments_submenu,
		NULL
	},
	{ 0,0 },
};


static DjynnConfig config;

static DjynnConfigKeys config_keys = {
	"djynn",
	"workspace",
	"workspace_id",
	"workspace_n",
	"workspace_%d",
	"session",
	"session_id",
	"session_n",
	"session_%d",
	"projectmanager",
	"project",
	"project_n",
	"project_%d",

	"project_manager",
	  "project_manager_show",
	  "project_manager_pos",
	  "ext_open_cmd",
	  "workspace_list",
	  "session_list",
	  "show_icons",
	  "show_tree_lines",
	  "single_click_open",
	  "double_click_ext",
	  "activate_prj",
	  "activate_sidebar",
	"comment",
	  "replace_geany_comments",
	"line",
	  "line_doc_on_no_selection",
	"encode",
	  "encode_doc_on_no_selection",
	"vim",
	  "vim_use_keyboard",
};

static DjynnWidgets widgets;

static DjynnData data = {
	NULL,          // config_dir
	NULL,          // config_filename
	NULL,          // keybindings
	0,					// keybindings_sum
	0,					// keybindings_index
	NULL,          // workspace
	NULL,          // workspace_key
	&config,       // config
	&config_keys,  // config_keys
	&widgets,		// widgets
	NULL,          // project_manager
};

DjynnData *djynn = &data;
DjynnConfig *djynn_cfg = &config;
DjynnConfigKeys *djynn_key = &config_keys;
DjynnWidgets *djynn_widget = &widgets;

static void keybind_activate(guint id) {
	DjynnKeybind *keybind = djynn->keybindings[id];
//debug_output("keybind_activate(id: %d, name: %s)\n",id,keybind->name);
	if(keybind!=NULL && keybind->menu_item!=NULL && keybind->menu_item->activate!=NULL) {
		QMenuItem *menuitem = keybind->menu_item;
		if(menuitem->type==Q_MENU_CHECKBOX)
			q_menu_toggle_checked(menuitem->id);
		else
			menuitem->activate(menuitem);
	}
}

gint djynn_count_keybindings(DjynnKeybind keybindings[]) {
	gint i;
	for(i=0; keybindings[i].id!=DJYNN_KEYBIND_END; ++i);
	return i;
}

void djynn_keybind(DjynnKeybind keybindings[]) {
	gint i,n;
	DjynnKeybind *keybind;
	for(i=0; 1; ++i) {
		keybind = &keybindings[i];
		if(keybind->id==DJYNN_KEYBIND_END) break;
		n = keybind->index==-1? djynn->keybindings_index : keybind->index;
		keybind->menu_item = q_menu_get(keybind->id);
		if(keybind->label==NULL) keybind->label = keybind->menu_item->label;
//debug_output("djynn_keybind(index: %d, name: %s, label: %s)\n",*index,keybind->name,keybind->menu_item->label);
		keybindings_set_item(plugin_key_group,n,keybind_activate,0,0,keybind->name,_(keybind->label),keybind->menu_item->widget);
		djynn->keybindings[n] = keybind;
		if(keybind->index==-1) keybind->index = djynn->keybindings_index++;
	}
}

void djynn_get_text_selection(DjynnTextSelection *selection,gboolean all_on_no_selection,gboolean full_lines) {
	selection->document = document_get_current();
	selection->scintilla = NULL;
	selection->text = NULL;
	selection->selection = FALSE;
	selection->all = FALSE;
	selection->start = 0;
	selection->end = 0;
	selection->eol = NULL;
	selection->eol_len = 0;
	if(selection->document!=NULL) {
		ScintillaObject *sci = selection->document->editor->sci;
		gint i;
		selection->scintilla = sci;
		selection->selection = sci_has_selection(sci);
		i = scintilla_send_message(sci,SCI_GETEOLMODE,0,0);
		switch(i) {
			case SC_EOL_CRLF:selection->eol = "\r\n",selection->eol_len = 2;break;
			case SC_EOL_CR:selection->eol = "\r",selection->eol_len = 1;break;
			default:selection->eol = "\n",selection->eol_len = 1;break;
		}
		if(selection->selection) {
			selection->start = sci_get_selection_start(sci);
			selection->end = sci_get_selection_end(sci);
			if(full_lines) {
				gint start_col = sci_get_col_from_position(sci,selection->start);
				gint end_col = sci_get_col_from_position(sci,selection->end);
				if(start_col>0) {
					gint start_line = sci_get_line_from_position(sci,selection->start);
					selection->start = sci_get_position_from_line(sci,start_line);
					sci_set_selection_start(sci,selection->start);
				}
				if(end_col>0) {
					gint end_line = sci_get_line_from_position(sci,selection->end);
					selection->end = sci_get_position_from_line(sci,end_line+1);
					sci_set_selection_end(sci,selection->end);
				}
			}
			selection->length = selection->end-selection->start;
			selection->text = sci_get_contents_range(sci,selection->start,selection->end);
		} else {
			if(all_on_no_selection) {
				selection->all = TRUE;
				selection->length = sci_get_length(sci);
				selection->end = selection->length;
				selection->text = sci_get_contents(sci,selection->length+1);
			} else {
			}
		}
		
	}
}

/*static void startup_cb(GObject *obj,gpointer user_data) {
//	gtk_widget_queue_draw(geany->main_widgets->window);
	gtk_container_propagate_expose();
}*/

static void configure_cb(GtkDialog *dlg,gint response,gpointer data) {
//	gboolean project_manager;
//	gint project_manager_pos;
	gboolean prev_project_manager = djynn_cfg->project_manager;
	gint prev_project_manager_pos = djynn_cfg->project_manager_pos;
	gboolean prev_comment = djynn_cfg->comment;
	gboolean prev_line = djynn_cfg->line;
	gboolean prev_encode = djynn_cfg->encode;

	if(response!=GTK_RESPONSE_APPLY && response!=GTK_RESPONSE_OK) return;
	q_config_open(djynn->config_filename);

#define DJYNN_CONFIG_GET_CHECK_BUTTON(key) \
	djynn_cfg->key = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(djynn_widget->config.key)); \
	q_config_set_int(djynn_key->djynn,djynn_key->key,djynn_cfg->key)

#define DJYNN_CONFIG_GET_COMBO_BOX(key) \
	djynn_cfg->key = gtk_combo_box_get_active(GTK_COMBO_BOX(djynn_widget->config.key)); \
	q_config_set_int(djynn_key->djynn,djynn_key->key,djynn_cfg->key)

#define DJYNN_CONFIG_GET_ENTRY(key) \
	if(djynn_cfg->key!=NULL) g_free(djynn_cfg->key); \
	djynn_cfg->key = gtk_editable_get_chars(GTK_EDITABLE(djynn_widget->config.key),0,-1); \
	q_config_set_str(djynn_key->djynn,djynn_key->key,djynn_cfg->key)

	DJYNN_CONFIG_GET_CHECK_BUTTON(project_manager);
	DJYNN_CONFIG_GET_COMBO_BOX(project_manager_pos);
	DJYNN_CONFIG_GET_ENTRY(ext_open_cmd);
	DJYNN_CONFIG_GET_COMBO_BOX(workspace_list);
	DJYNN_CONFIG_GET_COMBO_BOX(session_list);
	DJYNN_CONFIG_GET_COMBO_BOX(show_icons);
	DJYNN_CONFIG_GET_CHECK_BUTTON(show_tree_lines);
	DJYNN_CONFIG_GET_CHECK_BUTTON(single_click_open);
	DJYNN_CONFIG_GET_CHECK_BUTTON(double_click_ext);
	DJYNN_CONFIG_GET_CHECK_BUTTON(activate_prj);
	DJYNN_CONFIG_GET_CHECK_BUTTON(activate_sidebar);
	DJYNN_CONFIG_GET_CHECK_BUTTON(comment);
	DJYNN_CONFIG_GET_CHECK_BUTTON(replace_geany_comments);
	DJYNN_CONFIG_GET_CHECK_BUTTON(line);
	DJYNN_CONFIG_GET_CHECK_BUTTON(line_doc_on_no_selection);
	DJYNN_CONFIG_GET_CHECK_BUTTON(encode);
	DJYNN_CONFIG_GET_CHECK_BUTTON(encode_doc_on_no_selection);
	djynn_workspace_save();

	/*project_manager = djynn_cfg->project_manager;
	project_manager_pos = djynn_cfg->project_manager_pos;
	djynn_cfg->project_manager = prev_project_manager;
	djynn_cfg->project_manager_pos = prev_project_manager_pos;
	djynn_project_show_sidebar(project_manager,project_manager_pos);*/
	if(djynn_cfg->project_manager) {
		if(!prev_project_manager) djynn_project_enable();
		else if(djynn_cfg->project_manager_pos!=prev_project_manager_pos) {
			if(djynn_pm->sidebar_page!=NULL) {
				g_object_ref(djynn_pm->sidebar_page);
				djynn_project_remove_sidebar();
				djynn_project_set_sidebar();
				g_object_unref(djynn_pm->sidebar_page);
			}
		}
		djynn_project_position_lists();
		djynn_workspace_load();
		djynn_project_configure();
	} else if(prev_project_manager) djynn_project_disable();
	if(djynn_cfg->comment && !prev_comment) djynn_comment_enable();
	else if(!djynn_cfg->comment && prev_comment) djynn_comment_disable();
	if(djynn_cfg->line && !prev_line) djynn_line_enable();
	else if(!djynn_cfg->line && prev_line) djynn_line_disable();
	if(djynn_cfg->encode && !prev_encode) djynn_encode_enable();
	else if(!djynn_cfg->encode && prev_encode) djynn_encode_disable();

	q_config_close();
}

static void configure_toggled_cb(GtkWidget *widget,gpointer user_data) {
	gboolean checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	if(widget==djynn_widget->config.project_manager) {
		gtk_widget_set_sensitive(djynn_widget->config.project_manager_pos,checked);
		gtk_widget_set_sensitive(djynn_widget->config.ext_open_cmd,checked);
		gtk_widget_set_sensitive(djynn_widget->config.workspace_list,checked);
		gtk_widget_set_sensitive(djynn_widget->config.session_list,checked);
		gtk_widget_set_sensitive(djynn_widget->config.show_icons,checked);
		gtk_widget_set_sensitive(djynn_widget->config.show_tree_lines,checked);
		gtk_widget_set_sensitive(djynn_widget->config.single_click_open,checked);
		gtk_widget_set_sensitive(djynn_widget->config.double_click_ext,checked);
		gtk_widget_set_sensitive(djynn_widget->config.activate_prj,checked);
		gtk_widget_set_sensitive(djynn_widget->config.activate_sidebar,checked);
	} else if(widget==djynn_widget->config.comment) {
		gtk_widget_set_sensitive(djynn_widget->config.replace_geany_comments,checked);
	} else if(widget==djynn_widget->config.line) {
		gtk_widget_set_sensitive(djynn_widget->config.line_doc_on_no_selection,checked);
	} else if(widget==djynn_widget->config.encode) {
		gtk_widget_set_sensitive(djynn_widget->config.encode_doc_on_no_selection,checked);
	}
}

GtkWidget *plugin_configure(GtkDialog *dlg) {
	GtkWidget *vbox = gtk_vbox_new(FALSE,0);
	GtkWidget *vbox1;

	vbox1 = gtk_vbox_new(FALSE,0);
	djynn_frame(vbox,_("Project Manager"),&djynn_widget->config.project_manager,djynn_cfg->project_manager,G_CALLBACK(configure_toggled_cb),
		_("Check to activate the Djynn Project Manager."),vbox1);

	djynn_widget->config.project_manager_pos = djynn_combo_box(vbox1,NULL,_("Position:"),100,
		(const gchar *[]){ _("Sidebar Page"),_("Left"),_("Right"),NULL },djynn_cfg->project_manager_pos,NULL);

	djynn_widget->config.ext_open_cmd = djynn_entry(vbox1,NULL,_("Open Directory:"),100,djynn_cfg->ext_open_cmd,
		_("The command to execute with \"Open Directory\" in the project manager. "
		  "You can use %f and %d wildcards:\n"
		  "- %f will be replaced with the filename including full path\n"
		  "- %d will be replaced with the path name of the selected file without the filename"));

	djynn_widget->config.workspace_list = djynn_combo_box(vbox1,NULL,_("Workspace List:"),100,
		(const gchar *[]){ _("Hidden"),_("Top"),_("Bottom"),NULL },djynn_cfg->workspace_list,NULL);

	djynn_widget->config.session_list = djynn_combo_box(vbox1,NULL,_("Sessions List:"),100,
		(const gchar *[]){ _("Hidden"),_("Top"),_("Bottom"),NULL },djynn_cfg->session_list,NULL);

	djynn_widget->config.show_icons = djynn_combo_box(vbox1,NULL,_("Show Icons:"),100,
		(const gchar *[]){ _("None"),_("Basic Icons"),_("Icons by Content-type"),NULL },djynn_cfg->show_icons,
		_("Icons shown for documents in Project manager tree view."));

	djynn_widget->config.show_tree_lines = djynn_check_button(vbox1,
		_("Show tree lines"),djynn_cfg->show_tree_lines,NULL,NULL);
	djynn_widget->config.single_click_open = djynn_check_button(vbox1,
		_("Single click, open document and focus it"),
		djynn_cfg->single_click_open,NULL,NULL);
	djynn_widget->config.double_click_ext = djynn_check_button(vbox1,
		_("Double click open directory"),
		djynn_cfg->double_click_ext,NULL,NULL);
	djynn_widget->config.activate_prj = djynn_check_button(vbox1,
		_("Activate project opens config-file instead of Geany project"),djynn_cfg->activate_prj,NULL,
		_("When double clicking on the project, the project configuration file is opened in the editor "
		  "instead of opening the Geany project (see Project in the menu)."));
	djynn_widget->config.activate_sidebar = djynn_check_button(vbox1,
		_("Activate document also activates sidebar page"),djynn_cfg->activate_sidebar,NULL,
		_("When opening a document in the editor, if another page is visible in the sidebar the Project "
		  "tab is automatically activated."));
	configure_toggled_cb(djynn_widget->config.project_manager,NULL);

	vbox1 = gtk_vbox_new(FALSE,0);
	djynn_frame(vbox,_("Comments"),&djynn_widget->config.comment,djynn_cfg->comment,G_CALLBACK(configure_toggled_cb),
		_("Check to activate Djynn Comments."),vbox1);
	djynn_widget->config.replace_geany_comments = djynn_check_button(vbox1,
		_("Replace Geany's built in commenting functions"),djynn_cfg->replace_geany_comments,NULL,NULL);
	configure_toggled_cb(djynn_widget->config.comment,NULL);

	vbox1 = gtk_vbox_new(FALSE,0);
	djynn_frame(vbox,_("Line Ordering"),&djynn_widget->config.line,djynn_cfg->line,G_CALLBACK(configure_toggled_cb),
		_("Check to activate Djynn Line Ordering."),vbox1);
	djynn_widget->config.line_doc_on_no_selection = djynn_check_button(vbox1,
		_("Format entire document when no text is selected"),djynn_cfg->line_doc_on_no_selection,NULL,NULL);
	configure_toggled_cb(djynn_widget->config.line,NULL);

	vbox1 = gtk_vbox_new(FALSE,0);
	djynn_frame(vbox,_("Encode & Decode"),&djynn_widget->config.encode,djynn_cfg->encode,G_CALLBACK(configure_toggled_cb),
		_("Check to activate Djynn Encode & Decode."),vbox1);
	djynn_widget->config.encode_doc_on_no_selection = djynn_check_button(vbox1,
		_("Format entire document when no text is selected"),djynn_cfg->encode_doc_on_no_selection,NULL,NULL);
	configure_toggled_cb(djynn_widget->config.encode,NULL);

	gtk_widget_show_all(vbox);
	g_signal_connect(dlg,"response",G_CALLBACK(configure_cb),NULL);
	return vbox;
}

void plugin_init(GeanyData *data) {
	gint i,pos;
	gchar str[257],key[257],*p1;

	memset(djynn_cfg,0,sizeof(DjynnConfig));
	memset(djynn_widget,0,sizeof(DjynnWidgets));

	msgwin_status_add(PACKAGE_NAME " " PACKAGE_VERSION);

	/* Configuration: */
	djynn->config_dir = g_strconcat(geany->app->configdir,G_DIR_SEPARATOR_S,"plugins",G_DIR_SEPARATOR_S,
			djynn_key->djynn,G_DIR_SEPARATOR_S,NULL);
	if(g_file_test(djynn->config_dir,G_FILE_TEST_EXISTS)==FALSE) g_mkdir(djynn->config_dir,0700);

	djynn->config_filename = g_strconcat(djynn->config_dir,"djynn.conf",NULL);
	if(g_file_test(djynn->config_filename,G_FILE_TEST_EXISTS)==FALSE) {
		FILE *fp = fopen(djynn->config_filename,"w");
		if(fp) {
			fprintf(fp,
				"\n[djynn]\n"
				"project_manager=1\n"
				"project_manager_show=%d\n"
				"project_manager_pos=%d\n"
				"ext_open_cmd=%s\n"
				"workspace_list=1\n"
				"session_list=1\n"
				"show_icons=2\n"
				"show_tree_lines=1\n"
				"single_click_open=1\n"
				"double_click_ext=1\n"
				"activate_prj=0\n"
				"activate_sidebar=0\n"
				"comment=1\n"
				"replace_geany_comments=1\n"
				"line=1\n"
				"line_doc_on_no_selection=1\n"
				"encode=1\n"
				"encode_doc_on_no_selection=1\n"
				"vim=1\n"
				"vim_use_keyboard=0\n"
				"workspace=1\n"
				"session=1\n"
				"\n"
				"[workspace]\n"
				"workspace_id=1\n"
				"workspace_n=1\n"
				"workspace_1=0001:%s\n"
				"\n"
				"[session]\n"
				"session_id=1\n"
				"session_n=1\n"
				"session_1=0001:%s\n"
				"\n"
				"[workspace_0001]\n"
				"project_n=0\n",
				TRUE,
				DJYNN_PM_POS_SIDEBAR_PAGE,
#ifdef G_OS_WIN32
				"explorer '%d'",
#else
				"nautilus '%d'",
#endif
				_("Workspace"),
				_("Session")
				);
			fclose(fp);
		} else perror(djynn->config_filename);
	}

	q_config_open(djynn->config_filename);

	djynn_cfg->ext_open_cmd = q_config_get_str(djynn_key->djynn,djynn_key->ext_open_cmd,
#ifdef G_OS_WIN32
		"explorer '%d'"
#else
		"nautilus '%d'"
#endif
	);

#define DJYNN_CONFIG_GET_INT(key,def) \
	djynn_cfg->key = q_config_get_int(djynn_key->djynn,djynn_key->key,def)

	DJYNN_CONFIG_GET_INT(project_manager,1);
	DJYNN_CONFIG_GET_INT(project_manager_show,TRUE);
	DJYNN_CONFIG_GET_INT(project_manager_pos,DJYNN_PM_POS_SIDEBAR_PAGE);
	DJYNN_CONFIG_GET_INT(workspace_list,1);
	DJYNN_CONFIG_GET_INT(session_list,1);
	DJYNN_CONFIG_GET_INT(show_icons,2);
	DJYNN_CONFIG_GET_INT(show_tree_lines,1);
	DJYNN_CONFIG_GET_INT(single_click_open,1);
	DJYNN_CONFIG_GET_INT(double_click_ext,1);
	DJYNN_CONFIG_GET_INT(activate_prj,0);
	DJYNN_CONFIG_GET_INT(activate_sidebar,0);
	DJYNN_CONFIG_GET_INT(comment,1);
	DJYNN_CONFIG_GET_INT(replace_geany_comments,1);
	DJYNN_CONFIG_GET_INT(line,1);
	DJYNN_CONFIG_GET_INT(line_doc_on_no_selection,1);
	DJYNN_CONFIG_GET_INT(encode,1);
	DJYNN_CONFIG_GET_INT(encode_doc_on_no_selection,1);
	DJYNN_CONFIG_GET_INT(vim,1);
	DJYNN_CONFIG_GET_INT(vim_use_keyboard,0);

//debug_output("project_manager_show: %d\n",djynn_cfg->project_manager_show);

	i = q_config_get_int(djynn_key->djynn,djynn_key->workspace,1); // [djynn]:workspace - Get active workspace index
	sprintf(str,djynn_key->workspace_d,i); // str <= [workspace]:"workspace_[i]"
	strcpy(str,q_config_get_str(djynn_key->workspace,str,NULL)); // Get active workspace string
	p1 = strchr(str,':'),*p1 = '\0',++p1; // p1 <= workspace name
	djynn->workspace = g_strdup(p1);
	sprintf(key,"%s_%s",djynn_key->workspace,str); // key <= ["workspace_000x"]
	djynn->workspace_key = g_strdup(key);

	/* Keybindings: */
	djynn->keybindings_sum = 0 +
		djynn_project_keybindings() +
//		djynn_bf_keybindings()+
		djynn_encode_keybindings() +
		djynn_line_keybindings() +
		djynn_comment_keybindings() +
		djynn_vim_keybindings();
	djynn->keybindings_index = 0;
	plugin_key_group = plugin_set_key_group(geany_plugin,PACKAGE_NAME,djynn->keybindings_sum,NULL);
	djynn->keybindings = (DjynnKeybind **)g_malloc(djynn->keybindings_sum*sizeof(DjynnKeybind *));
	memset(djynn->keybindings,0,djynn->keybindings_sum*sizeof(DjynnKeybind *));

	/* Locate Geany Widgets: */
	djynn_widget->geany.edit_menu = ui_lookup_widget(geany->main_widgets->window,"edit1_menu");
	djynn_widget->geany.commands_menu = ui_lookup_widget(geany->main_widgets->window,"commands2_menu");
	djynn_widget->geany.duplicate_line_or_selection = ui_lookup_widget(geany->main_widgets->window,"duplicate_line_or_selection1");
	djynn_widget->geany.send_selection_to_terminal = ui_lookup_widget(geany->main_widgets->window,"send_selection_to_vte1");
	djynn_widget->geany.format_menu_item = ui_lookup_widget(geany->main_widgets->window,"menu_format1");
	djynn_widget->geany.format_menu = ui_lookup_widget(geany->main_widgets->window,"menu_format1_menu");
	djynn_widget->geany.format_menu_pos = ui_lookup_widget(geany->main_widgets->window,"separator28");
	djynn_widget->geany.comment_line = ui_lookup_widget(geany->main_widgets->window,"menu_comment_line1");
	djynn_widget->geany.uncomment_line = ui_lookup_widget(geany->main_widgets->window,"menu_uncomment_line1");
	djynn_widget->geany.toggle_line_commentation = ui_lookup_widget(geany->main_widgets->window,"menu_toggle_line_commentation1");
	djynn_widget->geany.view_menu = ui_lookup_widget(geany->main_widgets->window,"menu_view1_menu");
	djynn_widget->geany.toggle_widgets_menu_item = ui_lookup_widget(geany->main_widgets->window,"menu_toggle_all_additional_widgets1");
	djynn_widget->geany.show_sidebar_menu_item = ui_lookup_widget(geany->main_widgets->window,"menu_show_sidebar1");
	djynn_widget->geany.project_menu = ui_lookup_widget(geany->main_widgets->window,"menu_project1_menu");
	djynn_widget->geany.tools_menu = geany->main_widgets->tools_menu;
	djynn_widget->geany.configuration_files = ui_lookup_widget(geany->main_widgets->window,"configuration_files1");
	djynn_widget->geany.configuration_files_menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(djynn_widget->geany.configuration_files));
	djynn_widget->geany.help_menu = ui_lookup_widget(geany->main_widgets->window,"menu_help1_menu");
	djynn_widget->geany.donate = ui_lookup_widget(geany->main_widgets->window,"help_menu_item_donate");
	djynn_widget->geany.about = ui_lookup_widget(geany->main_widgets->window,"menu_info1");

	djynn_widget->geany.statusbar_hbox = ui_lookup_widget(geany->main_widgets->window,"hbox1");

	djynn_widget->geany.vpaned1 = ui_lookup_widget(geany->main_widgets->window,"vpaned1");
	djynn_widget->geany.hpaned1 = ui_lookup_widget(geany->main_widgets->window,"hpaned1");
//debug_output("djynn_widget->geany.vpaned1: %p\ndjynn_widget->geany.hpaned1: %p\n",djynn_widget->geany.vpaned1,djynn_widget->geany.hpaned1);

	/* Add djynn.conf to Tools-menu: */
	q_menu_create(GTK_MENU_SHELL(djynn_widget->geany.configuration_files_menu),conf_menu_item,Q_MENU_APPEND,0);
	djynn_widget->djynn_conf_menu_item = q_menu_get_widget(DJYNN_CONF);

	/* Insert Djynn Comments-menu: */
	pos = q_menu_item_pos(GTK_MENU_SHELL(djynn_widget->geany.format_menu),GTK_MENU_ITEM(djynn_widget->geany.format_menu_pos));
	q_menu_create(GTK_MENU_SHELL(djynn_widget->geany.format_menu),comments_menu,Q_MENU_INSERT_AFTER,pos);
	djynn_widget->comment_menu_item = q_menu_get_widget(COMMENTS);
	djynn_widget->comment_menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(djynn_widget->comment_menu_item));
	g_object_ref(djynn_widget->geany.comment_line);
	g_object_ref(djynn_widget->geany.uncomment_line);
	g_object_ref(djynn_widget->geany.toggle_line_commentation);
	gtk_container_remove(GTK_CONTAINER(djynn_widget->geany.format_menu),djynn_widget->geany.comment_line);
	gtk_container_remove(GTK_CONTAINER(djynn_widget->geany.format_menu),djynn_widget->geany.uncomment_line);
	gtk_container_remove(GTK_CONTAINER(djynn_widget->geany.format_menu),djynn_widget->geany.toggle_line_commentation);
	gtk_menu_shell_append(GTK_MENU_SHELL(djynn_widget->comment_menu),djynn_widget->geany.comment_line);
	gtk_menu_shell_append(GTK_MENU_SHELL(djynn_widget->comment_menu),djynn_widget->geany.uncomment_line);
	gtk_menu_shell_append(GTK_MENU_SHELL(djynn_widget->comment_menu),djynn_widget->geany.toggle_line_commentation);
	g_object_unref(djynn_widget->geany.comment_line);
	g_object_unref(djynn_widget->geany.uncomment_line);
	g_object_unref(djynn_widget->geany.toggle_line_commentation);

	/* Init sub-plugins: */
	djynn_project_init(data);
//	djynn_bf_init(data);
	djynn_encode_init(data);
	djynn_line_init(data);
	djynn_comment_init(data);
	djynn_vim_init(data);

	q_config_close();

//	plugin_signal_connect(geany_plugin,NULL,"geany-startup-complete",TRUE,(GCallback)&startup_cb,NULL);
}


void plugin_cleanup() {
	gint comments_pos = q_menu_item_pos(GTK_MENU_SHELL(djynn_widget->geany.format_menu),GTK_MENU_ITEM(djynn_widget->geany.format_menu_pos));

	djynn_project_cleanup();
//	djynn_bf_cleanup();
	djynn_encode_cleanup();
	djynn_comment_cleanup();
	djynn_line_cleanup();
	djynn_vim_cleanup();

	g_object_ref(djynn_widget->geany.comment_line);
	g_object_ref(djynn_widget->geany.uncomment_line);
	g_object_ref(djynn_widget->geany.toggle_line_commentation);
	gtk_container_remove(GTK_CONTAINER(djynn_widget->comment_menu),djynn_widget->geany.comment_line);
	gtk_container_remove(GTK_CONTAINER(djynn_widget->comment_menu),djynn_widget->geany.uncomment_line);
	gtk_container_remove(GTK_CONTAINER(djynn_widget->comment_menu),djynn_widget->geany.toggle_line_commentation);
	gtk_menu_shell_insert(GTK_MENU_SHELL(djynn_widget->geany.format_menu),djynn_widget->geany.comment_line,++comments_pos);
	gtk_menu_shell_insert(GTK_MENU_SHELL(djynn_widget->geany.format_menu),djynn_widget->geany.uncomment_line,++comments_pos);
	gtk_menu_shell_insert(GTK_MENU_SHELL(djynn_widget->geany.format_menu),djynn_widget->geany.toggle_line_commentation,++comments_pos);
	g_object_unref(djynn_widget->geany.comment_line);
	g_object_unref(djynn_widget->geany.uncomment_line);
	g_object_unref(djynn_widget->geany.toggle_line_commentation);

	gtk_widget_destroy(djynn_widget->djynn_conf_menu_item);
	gtk_widget_destroy(djynn_widget->comment_menu_item);

	g_free(djynn->config_dir);
	g_free(djynn->config_filename);
	g_free(djynn_cfg->ext_open_cmd);
	g_free(djynn->workspace);
	g_free(djynn->workspace_key);
//debug_output("plugin_cleanup()\n");
}

void djynn_action(int id) {
	switch(id) {
		case DJYNN_CONF:
			document_open_file(djynn->config_filename,FALSE,NULL,NULL);
			break;
	}
}



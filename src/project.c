

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <libq/string.h>
#include <libq/tree.h>
#include <libq/glib/config.h>
#include <libq/gtk/menu.h>
#include "djynn.h"
#include "workspace.h"
#include "project.h"
#include "session.h"
#include "dialog.h"


static void menu_activate(QMenuItem *menuitem) {
	if(menuitem!=NULL) {
		gboolean check = FALSE;
//debug_output("menu_activate(id: %d)\n",menuitem->id);
		if(menuitem->type==Q_MENU_CHECKBOX || menuitem->type==Q_MENU_RADIO)
			check = q_menu_is_checked(menuitem->id);
		djynn_project_action(menuitem->id,check);
	}
}

enum {
	PROJECT_MANAGER = 0x2000,
	   NEW_PROJECT1,

	CONTEXT_MENU = 0x2100,
	   OPEN_FILE,
	   OPEN_ALL_FILES,
	   OPEN_DIR,
	   OPEN_TERMINAL,
	   CLOSE_FILE,
	   NEW_PROJECT2,
	   NEW_FOLDER,
	   NEW_FILE,
	   ADD_FILE,
	   ADD_OPEN_FILES,
	   REMOVE,
	   DELETE_FROM_DISK,
	   SORT_FILES,
	   MOVE_UP,
	   MOVE_DOWN,
	   PROJECT_PREFERENCES,
	   FOLDER_PREFERENCES,
	   FILE_PROPERTIES,
	   RELOAD_WORKSPACE,
	   HIDE_SIDEBAR,

	TOGGLE_SIDEBARS,
	SHOW_PROJECT_MANAGER,
};

static QMenuItem project_manager_submenu[] = {
/*   type          id                   label & tooltip                 icon                      submenu                    activate */
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, NEW_PROJECT1,        N_("New Project..."),NULL,      GTK_STOCK_NEW,            NULL,                      menu_activate },
	{ 0,0 },
};

static QMenuItem project_manager_menu[] = {
/*   type          id                       label & tooltip                     icon                      submenu                    activate */
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, PROJECT_MANAGER,         N_("Project Manager"),NULL,         NULL,                     project_manager_submenu,   NULL },
	{ 0,0 },
};

static QMenuItem context_menu_items[] = {
/*   type          id                       label & tooltip                     icon                      submenu                    activate */
	{ Q_MENU_LABEL, OPEN_FILE,               N_("Open File"),NULL,               GTK_STOCK_OPEN,           NULL,                      menu_activate },
	{ Q_MENU_LABEL, OPEN_ALL_FILES,          N_("Open All Files"),NULL,          GTK_STOCK_OPEN,           NULL,                      menu_activate },
	{ Q_MENU_LABEL, OPEN_DIR,                N_("Open Directory"),NULL,          GTK_STOCK_OPEN,           NULL,                      menu_activate },
	{ Q_MENU_LABEL, OPEN_TERMINAL,           N_("Open Terminal"),NULL,           GTK_STOCK_OPEN,           NULL,                      menu_activate },
	{ Q_MENU_LABEL, CLOSE_FILE,              N_("Close File"),NULL,              GTK_STOCK_CLOSE,          NULL,                      menu_activate },
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, NEW_PROJECT2,            N_("New Project..."),NULL,          GTK_STOCK_NEW,            NULL,                      menu_activate },
	{ Q_MENU_LABEL, NEW_FOLDER,              N_("New Folder..."),NULL,           GTK_STOCK_NEW,            NULL,                      menu_activate },
	{ Q_MENU_LABEL, NEW_FILE,                N_("New File..."),NULL,             GTK_STOCK_NEW,            NULL,                      menu_activate },
	{ Q_MENU_LABEL, ADD_FILE,                N_("Add Active File"),NULL,         GTK_STOCK_ADD,            NULL,                      menu_activate },
	{ Q_MENU_LABEL, ADD_OPEN_FILES,          N_("Add All Open Files"),NULL,      GTK_STOCK_ADD,            NULL,                      menu_activate },
	{ Q_MENU_LABEL, REMOVE,                  N_("Remove"),NULL,                  GTK_STOCK_DELETE,         NULL,                      menu_activate },
	{ Q_MENU_LABEL, DELETE_FROM_DISK,        N_("Delete From Disk"),NULL,        GTK_STOCK_DELETE,         NULL,                      menu_activate },
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, SORT_FILES,              N_("Sort Files"),NULL,              GTK_STOCK_SORT_ASCENDING, NULL,                      menu_activate },
	{ Q_MENU_LABEL, MOVE_UP,                 N_("Move Up"),NULL,                 GTK_STOCK_GO_UP,          NULL,                      menu_activate },
	{ Q_MENU_LABEL, MOVE_DOWN,               N_("Move Down"),NULL,               GTK_STOCK_GO_DOWN,        NULL,                      menu_activate },
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, PROJECT_PREFERENCES,     N_("Project Preferences"),NULL,     GTK_STOCK_PREFERENCES,    NULL,                      menu_activate },
	{ Q_MENU_LABEL, FOLDER_PREFERENCES,      N_("Folder Preferences"),NULL,      GTK_STOCK_PREFERENCES,    NULL,                      menu_activate },
	{ Q_MENU_LABEL, FILE_PROPERTIES,         N_("Properties"),NULL,              GTK_STOCK_PROPERTIES,     NULL,                      menu_activate },
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, RELOAD_WORKSPACE,        N_("Reload Workspace"),NULL,        GTK_STOCK_REFRESH,        NULL,                      menu_activate },
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, HIDE_SIDEBAR,            N_("Hide Sidebar"),NULL,            GTK_STOCK_CLOSE,          NULL,                      menu_activate },
	{ 0,0 },
};

static QMenuItem view_toggle_sidebars_menu_items[] = {
/*   type              id                    label & tooltip                         icon   submenu  activate */
	{ Q_MENU_LABEL,     TOGGLE_SIDEBARS,       N_("Toggle Sidebars"),NULL,            NULL,  NULL,    menu_activate },
	{ 0,0 },
};

static QMenuItem view_show_pm_menu_items[] = {
/*   type              id                     label & tooltip                        icon   submenu  activate */
	{ Q_MENU_CHECKBOX,  SHOW_PROJECT_MANAGER,  N_("Show Project Manager"),NULL,       NULL,  NULL,    menu_activate },
	{ 0,0 },
};

static DjynnKeybind project_keybindings[] = {
	{ TOGGLE_SIDEBARS,       "sidebars_toggle",         -1,  NULL },
	{ SHOW_PROJECT_MANAGER,  "project_manager_toggle",  -1,  N_("Toggle Project Manager") },
	{ DJYNN_KEYBIND_END },
};

enum {
	CONTEXT_MENU_NO_SELECTION,
	CONTEXT_MENU_SELECTED_PROJECT,
	CONTEXT_MENU_SELECTED_FOLDER,
	CONTEXT_MENU_SELECTED_FILE,
};

enum {
	PRJ_TREE_ICON,
	PRJ_TREE_FILENAME,
	PRJ_TREE_PATH,
	PRJ_TREE_DATA,
	PRJ_TREE_COLS,
};

#define MOVE_BEFORE        GTK_TREE_VIEW_DROP_BEFORE
#define MOVE_AFTER         GTK_TREE_VIEW_DROP_AFTER
#define MOVE_TO_OR_BEFORE  GTK_TREE_VIEW_DROP_INTO_OR_BEFORE
#define MOVE_TO_OR_AFTER   GTK_TREE_VIEW_DROP_INTO_OR_AFTER


static const gchar *file_types = ""
"asm adb ads c h cpp cxx c++ cc hpp hxx h++ hh cs d di f for ftn f77 f90 "
"f95 f03 bas bi glsl frag vert hs lhs hx as java jsp pas pp inc dpr dpk "
"vala vapi vhd vhdl fe js lua m mq4 pl perl pm agi pod php php3 php4 php5 "
"phtml py pyw R r rb rhtml ruby sh ksh zsh ash bash tcl tk wish css htm "
"html shtml hta htd htt cfm xml sgml xsl xslt xsd xhtml txt cmake ctest "
"conf ini config rc cfg diff patch rej debdiff dpatch nsi nsh po pot tex "
"sty idx ltx rest reST rst sql yaml yml";
static const gchar *exclude_dirs = ""
"CMakeFiles "
".git "
".svn";

static DjynnProjectManager project_manager;
DjynnProjectManager *djynn_pm = &project_manager;

static void prj_free_files(DjynnProjectFile *f) {
	DjynnProjectFile *f1,*f2;
//debug_output("prj_free_files(path=%s,name=%s)\n",f->path,f->name);
	if(f->path!=NULL) g_free(f->path);
	if(f->name!=NULL) g_free(f->name);
	if(f->pattern!=NULL) g_free(f->pattern);
	if(f->tree_path!=NULL) gtk_tree_path_free(f->tree_path);
	for(f1=f->files; f1!=NULL; f1=f2) {
		f2 = f1->next;
		prj_free_files(f1);
	}
	g_free(f);
}

static void prj_file_remove(DjynnProjectFile *f) {
	DjynnProjectFile *p1 = f->parent,*f1;
	if(p1==NULL) {
		f1 = djynn_pm->projects;
		if(f==f1) {
			gint i;
			djynn_pm->projects = f->next;
			for(i=1,p1=f->next; p1!=NULL; i++,p1=p1->next) ((DjynnProject *)p1)->index = i;
			return;
		}
	} else {
		f1 = p1->files;
		if(f1==f) { p1->files = f->next;return; }
	}
	for(; f1->next!=f; f1=f1->next);
	f1->next = f->next;
}

static gboolean prj_move_to(DjynnProjectFile *f,DjynnProjectFile *dest,gint pos,const gchar **fail) {
	const gchar *dnd_fail[] = {
		_("Cannot drag-n-drop virtual files and folders."),
		_("Cannot drag-n-drop into virtual files and folders."),
		_("Cannot drag-n-drop into dynamically loaded files and folders.")
	};
	if(f==NULL || dest==NULL || f==dest) return FALSE;
	if((f->status&DJYNN_PM_VIRTUAL)) {
		if(fail!=NULL) *fail = dnd_fail[0];
		return FALSE;
	}
	if((dest->status&DJYNN_PM_VIRTUAL)) {
		if(fail!=NULL) *fail = dnd_fail[1];
		return FALSE;
	}
//debug_output("prj_move_to(%s[%d] => %s[%d] [%d,%s])\n",f->name,f->type,dest->name,dest->type,pos,
//pos==MOVE_BEFORE? "before" : (pos==MOVE_TO_OR_BEFORE? "to/before" : (pos==MOVE_AFTER? "after" : "to/after")));
	if(f->type==DJYNN_PROJECT) {
//debug_output("prj_move_to(f=project)\n");
		if(dest->type==DJYNN_PROJECT) {
			if(pos==MOVE_BEFORE || pos==MOVE_TO_OR_BEFORE) {
				if(!q_tree_insert_before((QTreeNode *)&djynn_pm->projects,(QTreeNode)dest,(QTreeNode)f)) return FALSE;
			} else {
				if(dest->next==f) return FALSE;
				if(!q_tree_insert_after((QTreeNode *)&djynn_pm->projects,(QTreeNode)dest,(QTreeNode)f)) return FALSE;
			}
		} else {
			while(dest!=NULL && dest->type!=DJYNN_PROJECT) dest = dest->parent;
			if(dest==NULL) return FALSE;
			if(!q_tree_insert_after((QTreeNode *)&djynn_pm->projects,(QTreeNode)dest,(QTreeNode)f)) return FALSE;
		}
		djynn_workspace_index_projects();
	} else if(dest->type==DJYNN_PROJECT) {
//debug_output("prj_move_to(dest=project)\n");
		if(dest->files==f) return FALSE;
		if((dest->status&DJYNN_PM_DYNAMIC)) {
			if(fail!=NULL) *fail = dnd_fail[2];
			return FALSE;
		}
		return q_tree_insert_child((QTreeNode *)&djynn_pm->projects,(QTreeNode)dest,(QTreeNode)f);
	} else if(dest->type==DJYNN_PROJECT_FILE) {
//debug_output("prj_move_to(dest=file)\n");
		if(pos==MOVE_BEFORE || pos==MOVE_TO_OR_BEFORE) {
			return q_tree_insert_before((QTreeNode *)&djynn_pm->projects,(QTreeNode)dest,(QTreeNode)f);
		} else {
			if(dest->next==f) return FALSE;
			return q_tree_insert_after((QTreeNode *)&djynn_pm->projects,(QTreeNode)dest,(QTreeNode)f);
		}
	} else {
		gboolean e = gtk_tree_view_row_expanded(GTK_TREE_VIEW(djynn_pm->project_list),dest->tree_path);
//debug_output("prj_move_to(dest=folder)\n");
		if(pos==MOVE_BEFORE) {
			return q_tree_insert_before((QTreeNode *)&djynn_pm->projects,(QTreeNode)dest,(QTreeNode)f);
		} else if(pos==MOVE_AFTER && !e) {
			if(dest->next==f) return FALSE;
			return q_tree_insert_after((QTreeNode *)&djynn_pm->projects,(QTreeNode)dest,(QTreeNode)f);
		} else {
			if(dest->files==f) return FALSE;
			if((dest->status&DJYNN_PM_DYNAMIC)) {
				if(fail!=NULL) *fail = dnd_fail[2];
				return FALSE;
			}
			return q_tree_insert_child((QTreeNode *)&djynn_pm->projects,(QTreeNode)dest,(QTreeNode)f);
		}
	}
	return FALSE;
}

void djynn_project_clear() {
	DjynnProject *p;
	DjynnProjectFile *f1,*f2;
	for(f1=djynn_pm->projects; f1!=NULL; f1=f2) {
		f2 = f1->next;
		p = (DjynnProject *)f1;
		if(p->project_filename!=NULL) g_free(p->project_filename);
		if(p->geany_project_filename!=NULL) g_free(p->geany_project_filename);
		prj_free_files(f1);
	}
	q_array_free(djynn_pm->project_files);
	djynn_pm->projects = NULL;
	djynn_pm->project_files = NULL;
}

static int sort_files_compare(const void *a,const void *b) {
	DjynnProjectFile *f1 = *(DjynnProjectFile **)a;
	DjynnProjectFile *f2 = *(DjynnProjectFile **)b;
	gchar *n1 = f1->name,*n2 = f2->name;
//debug_output("sort_files_compare(f1: [%p], f2: [%p])\n",f1,f2);
//debug_output("sort_files_compare(name1=%s,type1=%d,name2=%s,type2=%d)\n",f1->name,f1->type,f2->name,f2->type);
	if(f1->type!=DJYNN_PROJECT_FILE && f2->type==DJYNN_PROJECT_FILE) return -1;
	if(f1->type==DJYNN_PROJECT_FILE && f2->type!=DJYNN_PROJECT_FILE) return 1;
	if(n1==NULL || n2==NULL) return 0;
	while(*n1=='_') ++n1;
	while(*n2=='_') ++n2;
	return q_stricmp(n1,n2);
}

static void sort_files(DjynnProjectFile *f,gboolean r) {
	if(f!=NULL && f->files!=NULL) {
		gint i,n;
		DjynnProjectFile *f0,*f1;
		for(n=0,f1=f->files; f1!=NULL; ++n,f1=f1->next);
		if(n>1) {
			DjynnProjectFile *files[n];
			for(i=0,f1=f->files; i<n; ++i,f1=f1->next) {
				files[i] = f1;
//debug_output("sort_files([%p] name=%s)\n",files[i],f1->name);
			}
			qsort(files,n,sizeof(DjynnProjectFile *),sort_files_compare);
			for(i=0,f0=NULL; i<n; ++i,f0=f1) {
				f1 = files[i];
//debug_output("sort_files(name=%s)\n",f1->name);
				if(f0==NULL) f->files = f1;
				else f0->next = f1;
				f1->next = NULL;
				if(f1->type!=DJYNN_PROJECT_FILE && f1->files!=NULL && r)
					sort_files(f1,r);
			}
		}
	}
}

static void insert_file(DjynnProjectFile *p,DjynnProjectFile *f) {
	if(p->type!=DJYNN_PROJECT_FILE) {
		if(p->files==NULL || sort_files_compare(&f,&p->files)<0) {
			f->next = p->files,p->files = f,f->parent = p;
			return;
		}
		for(p=p->files; p->next!=NULL; p=p->next)
			if(sort_files_compare(&f,&p->next)<0) break;
	}
	f->next = p->next,p->next = f,f->parent = p->parent;
	return;
}

static DjynnProjectFile *add_file(DjynnProjectFile *f,const gchar *path,const gchar *name) {
	DjynnProjectFile *f1;
//debug_output("add_file(path=%s)\n",path);
	for(f1=(f->type==DJYNN_PROJECT_FILE? f->parent->files : f->files); f1!=NULL; f1=f1->next)
		if(f1->type==DJYNN_PROJECT_FILE && *f1->path==*path && strcmp(f1->path,path)==0) break;
	if(f1!=NULL) return f;

	f1 = (DjynnProjectFile *)g_malloc(sizeof(DjynnProjectFile));
	f1->status = 0;
	f1->type = DJYNN_PROJECT_FILE;
	f1->path = g_strdup(path);
	f1->name = g_strdup(name);
//	f1->depth = f->depth+(f->type==DJYNN_PROJECT_FILE? 0 : 1);
	f1->pattern = NULL;
	f1->tree_path = NULL;
	f1->files = NULL;
	f1->next = NULL;
//debug_output("add_file(f->name=%s,f->type=%d,f1->name=%s)\n",f->name,f->type,f1->name);
	insert_file(f,f1);
	return f1;
}

static DjynnProjectFile *add_document(DjynnProjectFile *f,GeanyDocument *doc) {
//debug_output("add_file(doc->file_name=%s)\n",doc->file_name);
	if(doc->file_name==NULL || !g_path_is_absolute(doc->file_name)) return f;
	return add_file(f,doc->file_name,strrchr(doc->file_name,G_DIR_SEPARATOR));
}

static DjynnProjectFile *add_folder(DjynnProjectFile *f,
                       const gchar *path,const gchar *name,
                       gboolean add_files,gboolean subfolders,gboolean dynamic,
                       const GRegex *regex,
                       gboolean keep_empty_folder) {
	DjynnProjectFile *f1,*f2;
//debug_output("add_folder(path=%s,name=%s,add_files=%s,subfolders=%s)\n",path,name,add_files? "true" : "false",subfolders? "true" : "false");
	if(f!=NULL) {
		for(f1=(f->type==DJYNN_PROJECT_FILE? f->parent->files : f->files); f1!=NULL; f1=f1->next)
			if(f1->type==DJYNN_PROJECT_FOLDER && strcmp(f1->name,name)==0) break;
		if(f1!=NULL) return NULL;
	}

	f1 = (DjynnProjectFile *)g_new0(DjynnProjectFile,1);
	f1->type = DJYNN_PROJECT_FOLDER;
//debug_output("add_folder(f->name=%s,f->type=%d,f1->name=%s)\n",f->name,f->type,f1->name);

	if(add_files) f1->status |= DJYNN_PM_ADD_FILES;
	if(subfolders) f1->status |= DJYNN_PM_SUBFOLDERS;
	if(dynamic) f1->status |= DJYNN_PM_DYNAMIC;

	if(add_files || subfolders) {
		GDir *d = g_dir_open(path,0,NULL);
		const gchar *nm;
		gchar str[1025],*ext;
		if(d!=NULL) {
			while((nm=g_dir_read_name(d))!=NULL) {
				if(strcmp(".",nm)==0 || strcmp("..",nm)==0) continue;
				f2 = NULL;
				sprintf(str,"%s%c%s",path,G_DIR_SEPARATOR,nm);
				if(g_file_test(str,G_FILE_TEST_IS_DIR)) {
//debug_output("add_folder(path=%s,name=%s)\n",str,nm);
					if(subfolders && strstr(exclude_dirs,nm)==NULL)
						f2 = add_folder(f1,str,nm,add_files,subfolders,FALSE,regex,FALSE);
				} else if(add_files) {
					GError *error = NULL;
//debug_output("add_folder(path=%s,name=%s,regex=%p)\n",str,nm,regex);
					if((regex!=NULL && g_regex_match_full(regex,nm,-1,0,0,NULL,&error)!=TRUE) ||
						(regex==NULL && ((ext=strrchr(nm,'.'))==NULL || strstr(file_types,++ext)==NULL))) {

						if(error!=NULL) {
							g_printerr("Regex error: #%d %s\n",error->code,error->message);
							g_error_free(error);
						} else if(regex!=NULL) {
//debug_output("add_folder(regex=%s)\n",g_regex_get_pattern(regex));
						}
						continue;
					}
//if(regex!=NULL) debug_output("add_folder(%s OK)\n",nm);
					f2 = add_file(f1,str,nm);
				}
				if(f2!=NULL) f2->status |= DJYNN_PM_VIRTUAL;
			}
			g_dir_close(d);
		}
	}

	if(add_files && f1->files==NULL && !keep_empty_folder) {
		g_free(f1);
		return NULL;
	}

	f1->path = g_strdup(path);
	f1->name = g_strdup(name);
	f1->pattern = regex!=NULL? g_strdup(g_regex_get_pattern(regex)) : NULL;

	if(f!=NULL) insert_file(f,f1);

//	if(f1->files!=NULL) sort_files(f1,TRUE);

	return f1;
}

static DjynnProjectFile *get_selected_file() {
	GtkTreeModel *model;
	GtkTreeIter iter;
	DjynnProjectFile *f = NULL;
	if(gtk_tree_selection_get_selected(djynn_pm->project_selection,&model,&iter))
		gtk_tree_model_get(model,&iter,PRJ_TREE_DATA,&f,-1);
	return f;
}

static void open_document(DjynnProjectFile *f) {
	if(f==NULL) f = get_selected_file();
	if(f==NULL) return;
	if(f->type==DJYNN_PROJECT) {
		DjynnProject *p = (DjynnProject *)f;
		document_open_file(p->geany_project_filename,FALSE,NULL,NULL);
	} else if(f->type==DJYNN_PROJECT_FILE)
		document_open_file(f->path,FALSE,NULL,NULL);
}

static void open_documents(DjynnProjectFile *f) {
//debug_output("open_documents()\n");
	if(f!=NULL) {
		DjynnProjectFile *f1;
		for(f1=f; f1!=NULL; f1=f1->next) {
//debug_output("open_documents(%s)\n",f1->path);
			if(f1->type==DJYNN_PROJECT_FILE)
				document_open_file(f1->path,FALSE,NULL,NULL);
		}
		for(f1=f; f1!=NULL; f1=f1->next)
			if(f1->files!=NULL) open_documents(f1->files);
	}
}

static void open_selected_folder_documents() {
	DjynnProjectFile *f = get_selected_file();
//debug_output("open_selected_folder_documents()\n");
	if(f!=NULL) {
		if(f->files!=NULL) open_documents(f->files);
		else if(f->type==DJYNN_PROJECT_FILE)
			document_open_file(f->path,FALSE,NULL,NULL);
	}
}

static void create_context_menu() {
	djynn_pm->context_menu = gtk_menu_new();
	q_menu_create(GTK_MENU_SHELL(djynn_pm->context_menu),context_menu_items,Q_MENU_APPEND,0);
}

static GtkWidget *get_context_menu(DjynnProjectFile *f) {
	DjynnProjectFile *prev = NULL;
	/*                       54-2-098-654-2109876-43210 */
	guint64 disable_mask = 0b00000000000000000000000000;
	guint64 hide_mask    = 0b10000111111111111110111111;
	if(f!=NULL) {
		GeanyDocument *doc = NULL;
		gboolean exists = TRUE;
		if(f->type==DJYNN_PROJECT) {
			DjynnProject *p = (DjynnProject *)f;
			doc = document_find_by_real_path(p->geany_project_filename);
			/*                 54-2-098-654-2109876-43210 */
			disable_mask   = 0b00000000000000000000000000;
			hide_mask      = 0b00000110000000000001011000;
		} else if(f->type==DJYNN_PROJECT_FOLDER) {
			exists = g_file_test(f->path,G_FILE_TEST_IS_DIR);
			/*                 54-2-098-654-2109876-43210 */
			disable_mask   = 0b00000000000000000000000000;
			hide_mask      = 0b00000101000000000001011001;
		} else if(f->type==DJYNN_PROJECT_FILE) {
			doc = document_find_by_real_path(f->path);
			exists = g_file_test(f->path,G_FILE_TEST_EXISTS);
			/*                 54-2-098-654-2109876-43210 */
			disable_mask   = 0b00000000000000000000000000;
			hide_mask      = 0b00000011000100000001000010;
		}

		if((f->status&DJYNN_PM_DYNAMIC)) {
			/*                 54-2-098-654-2109876-43210 */
			disable_mask  |= 0b00000000000100111010000000;
		} else if((f->status&DJYNN_PM_VIRTUAL)) {
			/*                 54-2-098-654-2109876-43210 */
			disable_mask  |= 0b00000011011100111010000000;
		}

		if(f->parent!=NULL && f->parent->files!=f)
			for(prev=f->parent->files; prev->next!=f; prev=prev->next);
		if(prev==NULL) {
			/*                 54-2-098-654-2109876-43210 */
			disable_mask  |= 0b00000000001000000000000000;
		}
		if(f->next==NULL) {
			/*                 54-2-098-654-2109876-43210 */
			disable_mask  |= 0b00000000010000000000000000;
		}
		if(f->files==NULL) {
			/*                 54-2-098-654-2109876-43210 */
			disable_mask  |= 0b00000000000100000000000000;
		}
		if(doc==NULL) {
			/*                 54-2-098-654-2109876-43210 */
			disable_mask  |= 0b00000000000000000000010000;
		} else {
			/*                 54-2-098-654-2109876-43210 */
			disable_mask  |= 0b00000000000000000000000001;
		}
		if(!exists) {
			/*                 54-2-098-654-2109876-43210 */
			disable_mask  |= 0b00000100000001000000000111;
		}
	}
	if(djynn_cfg->project_manager_pos!=DJYNN_PM_POS_SIDEBAR_PAGE) {
		/*                    54-2-098-654-2109876-43210 */
		disable_mask     |= 0b00000000000000000000000000;
		hide_mask        |= 0b01100000000000000000000000;
	}
	q_menu_state(context_menu_items,disable_mask,hide_mask);
	return djynn_pm->context_menu;
}


static GdkPixbuf *utils_pixbuf_from_path(const gchar *path) {
	GIcon *icon;
	GdkPixbuf *ret = NULL;
	GtkIconInfo *info;
	gchar *ctype;
	gint width;
	ctype = g_content_type_guess(path,NULL,0,NULL);
	icon = g_content_type_get_icon(ctype);
	g_free(ctype);
	if(icon!=NULL) {
		gtk_icon_size_lookup(GTK_ICON_SIZE_MENU,&width,NULL);
		info = gtk_icon_theme_lookup_by_gicon(gtk_icon_theme_get_default(),icon,width,GTK_ICON_LOOKUP_USE_BUILTIN);
		g_object_unref(icon);
		if(!info) return djynn_pm->icons[DJYNN_PROJECT_FILE];
		ret = gtk_icon_info_load_icon(info,NULL);
		gtk_icon_info_free(info);
	}
	return ret;
//	return djynn_pm->icons[DJYNN_PROJECT_FILE];
}

static void create_tree(GtkTreeStore *store,GtkWidget *cellview,GtkTreeIter *iter_parent,DjynnProjectFile *f) {
	GtkTreeIter iter;
	GdkPixbuf *icon;
	gchar str[1025],status[257] = "";
	for(; f!=NULL; f=f->next) {
//debug_output("create_tree(name=%s,%p)\n",f->name,f);
		icon = djynn_pm->icons[f->type];
		if(f->status&(DJYNN_PM_DYNAMIC)) sprintf(status," [%s]",_("dynamic"));
		else if(f->status&(DJYNN_PM_VIRTUAL)) sprintf(status," [%s]",_("virtual"));
		if(f->type==DJYNN_PROJECT) sprintf(str,"%s: %s%s\n%s:\n%s",_("Project"),f->name,status,_("Config File"),((DjynnProject *)f)->project_filename);
		else if(f->type==DJYNN_PROJECT_FOLDER) sprintf(str,"%s: %s%s\n%s",_("Folder"),f->name,status,f->path);
		else {
			sprintf(str,"%s: %s%s\n%s",_("File"),f->name,status,f->path);
			if(djynn_cfg->show_icons==0) icon = NULL;
			else if(djynn_cfg->show_icons==2) icon = utils_pixbuf_from_path(f->path);
		}
		gtk_tree_store_append(store,&iter,iter_parent);
		gtk_tree_store_set(store,&iter,
				PRJ_TREE_ICON,icon,
				PRJ_TREE_FILENAME,f->name,
				PRJ_TREE_PATH,str,
				PRJ_TREE_DATA,(gpointer)f,-1);
		if(f->tree_path!=NULL) gtk_tree_path_free(f->tree_path);
		f->tree_path = gtk_tree_model_get_path(GTK_TREE_MODEL(store),&iter);

		if(f->type==DJYNN_PROJECT_FILE)
			q_array_put_pointer(djynn_pm->project_files,f->path,f);

		if(f->files!=NULL) create_tree(store,cellview,&iter,f->files);
		if(icon!=NULL && icon!=djynn_pm->icons[f->type]) g_object_unref(icon);
	}
}

void djynn_project_update() {
	GtkWidget *cellview = gtk_cell_view_new();
//debug_output("djynn_project_update(1)\n");

	if(djynn_pm->project_files!=NULL) q_array_free(djynn_pm->project_files);
	djynn_pm->project_files = q_array_new(0,0);

	if(djynn_pm->project_store==NULL) {
		djynn_pm->project_store = gtk_tree_store_new(PRJ_TREE_COLS,GDK_TYPE_PIXBUF,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_POINTER);
	} else gtk_tree_store_clear(djynn_pm->project_store);
	create_tree(djynn_pm->project_store,cellview,NULL,(DjynnProjectFile *)djynn_pm->projects);
//debug_output("djynn_project_update(done)\n");
}

void djynn_project_add_file(DjynnProjectFile *f,gboolean reload) {
	add_document(f,document_get_current());
	if(reload)
		djynn_workspace_reload();
}

DjynnProjectFile *djynn_project_add_folder(DjynnProjectFile *f,
                              const gchar *path,const gchar *name,
                              gboolean add_files,gboolean subfolders,gboolean dynamic,
                              const gchar *pattern,
                              gboolean reload) {
	DjynnProjectFile *f1;
	GRegex *regex = NULL;
	if(pattern!=NULL && *pattern!='\0') {
		GError *error = NULL;
		regex = g_regex_new(pattern,0,0,&error);
		if(regex==NULL) {
			g_printerr("Error regex pattern \"%s\": #%d %s\n",pattern,error->code,error->message);
			g_error_free(error);
		}
	}
//debug_output("djynn_project_add_folder(pattern=%s,regex=%p)\n",pattern,regex);
	f1 = add_folder(f,path,name,add_files,subfolders,dynamic,regex,TRUE);
	if(regex!=NULL) g_regex_unref(regex);
	if(reload && f1!=NULL)
		djynn_workspace_reload();
	return f1;
}

DjynnProjectFile *djynn_project_create_file(DjynnProjectFile *f,
                              const gchar *path,const gchar *name,gboolean reload) {
	DjynnProjectFile *f1;
	gchar str[1025];
	sprintf(str,"%s%c%s",path,G_DIR_SEPARATOR,name);
//debug_output("djynn_project_create_file(path=%s,name=%s)\n",path,name);
	f1 = add_file(f,str,name);
//debug_output("djynn_project_create_file(f1=%p)\n",f1);
	if(f1!=NULL) {
//debug_output("djynn_project_create_file(path=%s)\n",f1->path);
		if(!g_file_test(str,G_FILE_TEST_EXISTS)) {
			// TODO: Use standard filetype templates when creating files.
			FILE *fp = fopen(str,"wb");
			fclose(fp);
		}
		if(reload)
			djynn_workspace_reload();
		document_open_file(str,FALSE,NULL,NULL);
	}
	return f1;
}

void djynn_project_sort_files(DjynnProjectFile *f,gboolean reload) {
	if(f!=NULL && f->files!=NULL) {
		sort_files(f,TRUE);
		if(reload)
			djynn_workspace_reload();
	}
}

void djynn_project_add_open_files(DjynnProjectFile *f,gboolean reload) {
	gint i;
	GeanyDocument *doc;
	for(i=0; 1; ++i) {
		doc = document_get_from_page(i);
		if(doc==NULL) break;
		f = add_document(f,doc);
	}
	if(reload)
		djynn_workspace_reload();
}

static void delete_from_disk(DjynnProjectFile *f) {
	if(f->type==DJYNN_PROJECT) {
		DjynnProject *p = (DjynnProject *)f;
		g_remove(p->project_filename);
		g_remove(p->geany_project_filename);
	} else if(f->type==DJYNN_PROJECT_FILE) {
		g_remove(f->path);
	} else {
		if(f->files!=NULL) {
			DjynnProjectFile *f1;
			for(f1=f->files; f1!=NULL; f1=f1->next)
				delete_from_disk(f1);
		}
		if(f->path!=NULL && *f->path!='\0' && g_file_test(f->path,G_FILE_TEST_IS_DIR)) {
			GDir *d = g_dir_open(f->path,0,NULL);
			const gchar *nm = NULL;
			if(d!=NULL) {
				while((nm=g_dir_read_name(d))!=NULL) {
					if(strcmp(".",nm)==0 || strcmp("..",nm)==0) continue;
					else break;
				}
				g_dir_close(d);
				if(nm==NULL) g_remove(f->path);
			}
		}
	}
}

void djynn_project_delete(DjynnProjectFile *f,gboolean disk,gboolean reload) {
	gchar *title;
	gchar *message;
	gint r = 0;
	if(f==NULL) return;
	if(f->type==DJYNN_PROJECT) {
		title = disk? _("Delete Project") : _("Remove Project");
		message = disk?
			_("Are you sure you want to permanently delete project \"%s\" from disk?\n"
			"Project files and folders remain untouched, only the project is deleted.") :
			_("Are you sure you want to remove project \"%s\" from this workspace?");
	} else if(f->type==DJYNN_PROJECT_FOLDER) {
		title = disk? _("Delete Folder") : _("Remove Folder");
		message = disk?
			_("Are you sure you want to permanently delete folder \"%s\" and all its "
			"files and subfolders from disk?") :
			_("Are you sure you want to remove folder \"%s\" and all its files and "
			"subfolders from the project?");
	} else {
		title = disk? _("Delete File") : _("Remove File");
		message = disk?
			_("Are you sure you want to permanently delete file \"%s\" from disk?") :
			_("Are you sure you want to remove file \"%s\" from the project?");
	}
	r = djynn_msgbox_ask(title,message,f->name);
	if(r!=GTK_RESPONSE_OK) return;

	if(disk) delete_from_disk(f);

	if(f->type==DJYNN_PROJECT) {
		q_config_open(djynn->config_filename);
		q_config_remove_from_list(djynn->workspace_key,djynn_key->project,((DjynnProject *)f)->index);
		q_config_close();
	}
	prj_file_remove(f);
	prj_free_files(f);
	if(reload)
		djynn_workspace_reload();
}

void djynn_project_move_up(DjynnProjectFile *f,gboolean reload) {
	if(f==NULL) return;
	if(f->type==DJYNN_PROJECT) {
		gint i = ((DjynnProject *)f)->index;
		if(i<=1) return;
		else {
			gchar key1[64],key2[64],*p1,*p2;
			q_config_open(djynn->config_filename);
			sprintf(key1,djynn_key->project_d,i-1);
			p1 = q_config_get_str(djynn->workspace_key,key1,NULL);
			sprintf(key2,djynn_key->project_d,i);
			p2 = q_config_get_str(djynn->workspace_key,key2,NULL);
			q_config_set_str(djynn->workspace_key,key1,p2);
			q_config_set_str(djynn->workspace_key,key2,p1);
			q_config_close();
			g_free(p1);
			g_free(p2);
		}
	} else {
		DjynnProjectFile *f1 = f->parent->files;
		if(f1==f) return;
		if(f1->next==f) f1->next = f->next,f->next = f1,f->parent->files = f;
		else {
			for(; f1->next->next!=f; f1=f1->next);
			f1->next->next = f->next,f->next = f1->next,f1->next = f;
		}
	}
	if(reload)
		djynn_workspace_reload();
}

void djynn_project_move_down(DjynnProjectFile *f,gboolean reload) {
	if(f==NULL) return;
	if(f->type==DJYNN_PROJECT) {
		gint i = ((DjynnProject *)f)->index,n;
		q_config_open(djynn->config_filename);
		n = q_config_get_int(djynn->workspace_key,djynn_key->project_n,0);
		if(i>=n) {
			q_config_close();
			return;
		} else {
			gchar key1[64],key2[64],*p1,*p2;
			sprintf(key1,djynn_key->project_d,i);
			p1 = q_config_get_str(djynn->workspace_key,key1,NULL);
			sprintf(key2,djynn_key->project_d,i+1);
			p2 = q_config_get_str(djynn->workspace_key,key2,NULL);
			q_config_set_str(djynn->workspace_key,key1,p2);
			q_config_set_str(djynn->workspace_key,key2,p1);
			q_config_close();
			g_free(p1);
			g_free(p2);
		}
	} else {
		DjynnProjectFile *f1 = f->parent->files;
		if(f->next==NULL) return;
		if(f1==f) f1 = f->next,f->parent->files = f1,f->next = f1->next,f1->next = f;
		else {
			for(; f1->next!=f; f1=f1->next);
			f1->next = f->next,f1 = f->next,f->next = f1->next,f1->next = f;
		}
	}
	if(reload)
		djynn_workspace_reload();
}

void djynn_project_close_file(DjynnProjectFile *f) {
	if(f!=NULL && f->type==DJYNN_PROJECT_FILE) {
		GeanyDocument *doc = document_find_by_real_path(f->path);
		if(doc!=NULL) document_close(doc);
	}
}

static void open_externally(gchar *path) {
	if(path!=NULL && *path!='\0' && g_file_test(path,G_FILE_TEST_EXISTS)) {
		gchar *cmd,*locale_cmd,*dir,*c;
		GString *cmd_str = g_string_new(djynn_cfg->ext_open_cmd);
		GError *error = NULL;
		gboolean is_dir = g_file_test(path,G_FILE_TEST_IS_DIR);
		dir = is_dir? g_strdup(path) : g_path_get_dirname(path);
		utils_string_replace_all(cmd_str,"%f",path);
		utils_string_replace_all(cmd_str,"%d",dir);
		cmd = g_string_free(cmd_str,FALSE);
		locale_cmd = utils_get_locale_from_utf8(cmd);
		if(!g_spawn_command_line_async(locale_cmd,&error)) {
			c = strchr(cmd,' ');
			if(c!=NULL) *c = '\0';
//			msgwin_status_add(_("Could not execute configured external command '%s' (%s)."),cmd,error->message);
			ui_set_statusbar(TRUE,_("Could not execute configured external command '%s' (%s)."),cmd,error->message);
			g_error_free(error);
		}
		g_free(locale_cmd);
		g_free(cmd);
		g_free(dir);
		return;
	}
//	msgwin_status_add(_("Not a valid directory or file: %s"),path);
	ui_set_statusbar(TRUE,_("Not a valid directory or file: %s"),path);
}

static gchar *get_terminal() {
	gchar *terminal;
#ifdef G_OS_WIN32
	terminal = g_strdup("cmd");
#else
	const gchar *term = g_getenv("TERM");
	if(term!=NULL) terminal = g_strdup(term);
	else terminal = g_strdup("xterm");
#endif
	return terminal;
}

static void open_terminal(gchar *path) {
	if(path!=NULL && *path!='\0' && g_file_test(path,G_FILE_TEST_EXISTS)) {
		gchar *argv[2] = { get_terminal(),NULL };
		gchar *dir = g_file_test(path,G_FILE_TEST_IS_DIR)? g_strdup(path) : g_path_get_dirname(path);
		if(!g_spawn_async(dir,argv,NULL,G_SPAWN_SEARCH_PATH,NULL,NULL,NULL,NULL)) {
//			msgwin_status_add(_("Could not open terminal '%s' for directory: %s"),argv[0],dir);
			ui_set_statusbar(TRUE,_("Could not open terminal '%s' for directory: %s"),argv[0],dir);
		}
		g_free(dir);
		g_free(argv[0]);
	}
//	msgwin_status_add(_("Not a valid directory or file: %s"),path);
	ui_set_statusbar(TRUE,_("Not a valid directory or file: %s"),path);
}

void djynn_project_open_externally(DjynnProjectFile *f) {
	if(f!=NULL) open_externally(f->path);
}

void djynn_project_open_terminal(DjynnProjectFile *f) {
	if(f!=NULL) open_terminal(f->path);
}

static gboolean prj_tree_view_clicked_cb(GtkWidget *widget,GdkEventButton *event,GtkTreeSelection *selection) {
	GtkWidgetClass *widget_class = GTK_WIDGET_GET_CLASS(widget);
	gboolean ret = FALSE;

	if(widget_class->button_press_event)
		ret = widget_class->button_press_event(widget,event);

	if(event->button==1) {
		DjynnProjectFile *f = get_selected_file();
		if(event->type==GDK_2BUTTON_PRESS) {
			if(f!=NULL) {
				if(f->type==DJYNN_PROJECT) {
					if(djynn_cfg->activate_prj) {
						document_open_file(((DjynnProject *)f)->project_filename,FALSE,NULL,NULL);
					} else {
// TODO						project_close(FALSE);
// TODO						project_load_file(((DjynnProject *)f)->geany_project_filename);

//						project_load_file_with_session(((DjynnProject *)f)->geany_project_filename);
					}
				} else if(f->type==DJYNN_PROJECT_FOLDER) {
					GtkTreeModel *model = NULL;
					GtkTreeIter iter;
					if(gtk_tree_selection_get_selected(selection,&model,&iter)) {
						if(gtk_tree_model_iter_has_child(model,&iter)) {
							GtkTreePath *path = gtk_tree_model_get_path(model,&iter);
							if(gtk_tree_view_row_expanded(GTK_TREE_VIEW(widget),path))
								gtk_tree_view_collapse_row(GTK_TREE_VIEW(widget),path);
							else
								gtk_tree_view_expand_row(GTK_TREE_VIEW(widget),path,FALSE);
							gtk_tree_path_free(path);
							ret = TRUE;
						}
					}
				} else if(f->type==DJYNN_PROJECT_FILE) {
					if(!djynn_cfg->single_click_open) open_document(NULL);
					if(djynn_cfg->double_click_ext) open_externally(f->path);
				}
				ret = TRUE;
			}
		} else if(event->type==GDK_BUTTON_PRESS) {
			if(f!=NULL) {
				if(f->type==DJYNN_PROJECT_FILE) {
					if(djynn_cfg->single_click_open) {
						open_document(NULL);
						ret = TRUE;
					}
				}
			}
		}
	} else if(event->button==3) {
		DjynnProjectFile *f = get_selected_file();
		gtk_menu_popup(GTK_MENU(get_context_menu(f)),NULL,NULL,NULL,NULL,event->button,event->time);
		ret = TRUE;
	}
	return ret;
}


/******************************************************************************/
/* Signal receivable by destination */

/* Emitted when the data has been received from the source. It should check
 * the GtkSelectionData sent by the source, and do something with it. Finally
 * it needs to finish the operation by calling gtk_drag_finish, which will emit
 * the "data-delete" signal if told to. */
//static void prj_tree_view_drag_data_received_cb(GtkWidget *widget,GdkDragContext *context,gint x,gint y,GtkSelectionData *data,guint info,guint time,gpointer user_data) {
//debug_output("prj_tree_view_drag_data_received_cb(data: %d, format: %d, action: %x, actions: %x, suggested_action: %x, info: %d)\n",
//	data->length,data->format,context->action,context->actions,context->suggested_action,info);

//}

/* Emitted when a drag is over the destination */
//static gboolean prj_tree_view_drag_motion_cb(GtkWidget *widget,GdkDragContext *context,gint x,gint y,guint t,gpointer user_data) {
	// Fancy stuff here. This signal spams the console something horrible.
	//const gchar *name = gtk_widget_get_name (widget);
	//g_print ("%s: drag_motion_handl\n", name);
//	return  FALSE;
//}

/* Emitted when a drag leaves the destination */
//static void prj_tree_view_drag_leave_cb(GtkWidget *widget,GdkDragContext *context,guint time,gpointer user_data) {
//debug_output("prj_tree_view_drag_leave_cb(action: %x, actions: %x, suggested_action: %x)\n",context->action,context->actions,context->suggested_action);
//}

/* Emitted when the user releases (drops) the selection. It should check that
 * the drop is over a valid part of the widget (if its a complex widget), and
 * itself to return true if the operation should continue. Next choose the
 * target type it wishes to ask the source for. Finally call gtk_drag_get_data
 * which will emit "drag-data-get" on the source. */
static gboolean prj_tree_view_drag_drop_cb(GtkWidget *widget,GdkDragContext *context,gint x,gint y,guint time,gpointer user_data) {
	DjynnProjectFile *file;
	DjynnProjectFile *dest;
	GtkTreeViewDropPosition pos;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
//debug_output("prj_tree_view_drag_drop_cb(action: %x, actions: %x, suggested_action: %x)\n",context->action,context->actions,context->suggested_action);

	file = get_selected_file();
	gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(widget),x,y,&path,&pos);
	if(path==NULL) return FALSE;
	gtk_tree_model_get_iter(GTK_TREE_MODEL(djynn_pm->project_store),&iter,path);
	gtk_tree_model_get(GTK_TREE_MODEL(djynn_pm->project_store),&iter,PRJ_TREE_DATA,&dest,-1);

//debug_output("prj_tree_view_drag_drop_cb(%s => %s (%d))\n",file->path,dest->path,pos);

	if(file!=NULL && dest!=NULL) {
		const gchar *fail = NULL;
		if(prj_move_to(file,dest,pos,&fail))
			djynn_workspace_save();
		else if(fail!=NULL)
			ui_set_statusbar(TRUE,"%s",fail);
	}

	return FALSE;
}


/******************************************************************************/
/* Signals receivable by source */

/* Emitted after "drag-data-received" is handled, and gtk_drag_finish is called
 * with the "delete" parameter set to TRUE (when DnD is GDK_ACTION_MOVE). */
//static void prj_tree_view_drag_data_delete_cb(GtkWidget *widget,GdkDragContext *context,gpointer user_data) {
//debug_output("prj_tree_view_drag_data_delete_cb(action: %x, actions: %x, suggested_action: %x)\n",context->action,context->actions,context->suggested_action);
//}

/* Emitted when the destination requests data from the source via
 * gtk_drag_get_data. It should attempt to provide its data in the form
 * requested in the target_type passed to it from the destination. If it cannot,
 * it should default to a "safe" type such as a string or text, even if only to
 * print an error. Then use gtk_selection_data_set to put the source data into
 * the allocated selection_data object, which will then be passed to the
 * destination. This will cause "drag-data-received" to be emitted on the
 * destination. GdkSelectionData is based on X's selection mechanism which,
 * via X properties, is only capable of storing data in blocks of 8, 16, or
 * 32 bit units. */
//static void prj_tree_view_drag_data_get_cb(GtkWidget *widget,GdkDragContext *context,GtkSelectionData *data,guint info,guint time,gpointer use_data) {
//debug_output("prj_tree_view_drag_data_get_cb(action: %x, actions: %x, suggested_action: %x)\n",context->action,context->actions,context->suggested_action);
//}

/* Emitted when DnD begins. This is often used to present custom graphics. */
//static void prj_tree_view_drag_begin_cb(GtkWidget *widget,GdkDragContext *context,gpointer user_data) {
//debug_output("prj_tree_view_drag_begin_cb(action: %x, actions: %x, suggested_action: %x)\n",context->action,context->actions,context->suggested_action);
//}

/* Emitted when DnD ends. This is used to clean up any leftover data. */
static void prj_tree_view_drag_end_cb(GtkWidget *widget,GdkDragContext *context,gpointer user_data) {
//debug_output("prj_tree_view_drag_end_cb(action: %x, actions: %x, suggested_action: %x)\n",context->action,context->actions,context->suggested_action);
	djynn_workspace_load();
}


static void doc_activate_cb(GObject *obj,GeanyDocument *doc,gpointer user_data) {
	if(doc->file_name!=NULL && g_path_is_absolute(doc->file_name)) {
		DjynnProjectFile *f;
//debug_output("doc_activate_cb(file_name=%s)\n",doc->file_name);
		f = (DjynnProjectFile *)q_array_get_pointer(djynn_pm->project_files,doc->file_name);
		if(f!=NULL) {
//debug_output("doc_activate_cb(dir=%s,name=%s)\n",f->path,f->name);
			if(djynn_cfg->activate_sidebar)
				gtk_notebook_set_current_page(GTK_NOTEBOOK(geany->main_widgets->sidebar_notebook),djynn_pm->page_number);
			gtk_tree_view_expand_to_path(GTK_TREE_VIEW(djynn_pm->project_list),f->tree_path);
			gtk_tree_view_set_cursor(GTK_TREE_VIEW(djynn_pm->project_list),f->tree_path,NULL,FALSE);
		}
	}
}

static void doc_open_cb(GObject *obj,GeanyDocument *doc,gpointer user_data) {
	doc_activate_cb(obj,doc,user_data);
}

/*
static void doc_close_cb(GObject *obj,GeanyDocument *doc,gpointer user_data) {
}
*/

static void startup_cb(GObject *obj,gpointer user_data) {
	gtk_widget_queue_draw(djynn_pm->sidebar_page);
	gtk_widget_queue_draw(djynn_pm->scroll);
	gtk_widget_queue_draw(geany->main_widgets->message_window_notebook);
}

/*static void toggle_all_cb(GObject *obj,gpointer user_data) {
debug_output("toggle_all_cb()\n");
}*/

void djynn_project_configure() {
#if GTK_CHECK_VERSION(2,10,0)
	if(djynn_pm->project_list!=NULL)
		gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW(djynn_pm->project_list),djynn_cfg->show_tree_lines);
#endif
}

void djynn_project_init(GeanyData *data) {
	djynn->project_manager = &project_manager;
	memset(djynn->project_manager,0,sizeof(DjynnProject));
	djynn_project_create();
	if(!djynn_cfg->project_manager)
		djynn_project_disable();
}

void djynn_project_create() {
	const gchar *stock_icons[] = {
		GTK_STOCK_CONVERT,		// DJYNN_WORKSPACE
		GTK_STOCK_INDEX,			// DJYNN_SESSION
		GTK_STOCK_EXECUTE,		// DJYNN_PROJECT
		GTK_STOCK_DIRECTORY,		// DJYNN_PROJECT_FOLDER
		GTK_STOCK_FILE,			// DJYNN_PROJECT_FILE
	};
	GtkCellRenderer *icon_renderer;
	GtkCellRenderer *text_renderer;
	gint i;

	for(i=0; i<DJYNN_PM_TYPES; ++i)
		if(djynn_pm->icons[i]==NULL)
			djynn_pm->icons[i] = gtk_widget_render_icon(geany->main_widgets->window,stock_icons[i],GTK_ICON_SIZE_MENU,NULL);

	create_context_menu();

	if(djynn_pm->project_menu_item==NULL) {
		q_menu_create(GTK_MENU_SHELL(djynn_widget->geany.project_menu),project_manager_menu,Q_MENU_APPEND,0);
		djynn_pm->project_menu_sep = project_manager_menu[0].widget;
		djynn_pm->project_menu_item = project_manager_menu[1].widget;
		djynn_pm->project_menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(djynn_pm->project_menu_item));
	}

	if(djynn_pm->toggle_sidebars_menu_item==NULL) {
		q_menu_create(GTK_MENU_SHELL(djynn_widget->geany.view_menu),view_toggle_sidebars_menu_items,Q_MENU_INSERT_AFTER,
			q_menu_item_pos(GTK_MENU_SHELL(djynn_widget->geany.view_menu),GTK_MENU_ITEM(djynn_widget->geany.toggle_widgets_menu_item)));
		djynn_pm->toggle_sidebars_menu_item = view_toggle_sidebars_menu_items[0].widget;
	}

	if(djynn_pm->sidebar_menu_item==NULL) {
		q_menu_create(GTK_MENU_SHELL(djynn_widget->geany.view_menu),view_show_pm_menu_items,Q_MENU_INSERT_AFTER,
			q_menu_item_pos(GTK_MENU_SHELL(djynn_widget->geany.view_menu),GTK_MENU_ITEM(djynn_widget->geany.show_sidebar_menu_item)));
		djynn_pm->sidebar_menu_item = view_show_pm_menu_items[0].widget;
	}

	djynn_session_init(NULL);
	djynn_workspace_init(NULL);

	if(djynn_pm->sidebar_page==NULL) {
		const GtkTargetEntry row_targets[] = { { "GTK_TREE_MODEL_ROW",GTK_TARGET_SAME_WIDGET,0 } };

		djynn_pm->sidebar_page = gtk_vbox_new(FALSE,0);

		djynn_pm->scroll = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(djynn_pm->scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(djynn_pm->scroll),GTK_SHADOW_IN);

		djynn_pm->project_list = gtk_tree_view_new();
		gtk_widget_style_get(djynn_pm->project_list,"expander-size",&djynn_pm->project_expander_size,NULL);
		gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(djynn_pm->project_list),GDK_BUTTON1_MASK,row_targets,G_N_ELEMENTS(row_targets),GDK_ACTION_MOVE);
		gtk_tree_view_enable_model_drag_dest(GTK_TREE_VIEW(djynn_pm->project_list),row_targets,G_N_ELEMENTS(row_targets),GDK_ACTION_MOVE);

		djynn_project_update();
		gtk_tree_view_set_model(GTK_TREE_VIEW(djynn_pm->project_list),GTK_TREE_MODEL(djynn_pm->project_store));
		g_object_unref(djynn_pm->project_store);
		djynn_pm->project_column = gtk_tree_view_column_new();
		icon_renderer = gtk_cell_renderer_pixbuf_new();
		gtk_tree_view_column_pack_start(djynn_pm->project_column,icon_renderer,FALSE);
		gtk_tree_view_column_set_attributes(djynn_pm->project_column,icon_renderer,"pixbuf",PRJ_TREE_ICON,NULL);
		text_renderer = gtk_cell_renderer_text_new();
		gtk_tree_view_column_pack_start(djynn_pm->project_column,text_renderer,TRUE);
		gtk_tree_view_column_add_attribute(djynn_pm->project_column,text_renderer,"text",PRJ_TREE_FILENAME);
		gtk_tree_view_append_column(GTK_TREE_VIEW(djynn_pm->project_list),djynn_pm->project_column);
		ui_widget_modify_font_from_string(djynn_pm->project_list,geany->interface_prefs->tagbar_font);
		g_object_set(djynn_pm->project_list,"has-tooltip",TRUE,"tooltip-column",PRJ_TREE_PATH,NULL);

		djynn_pm->project_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(djynn_pm->project_list));
		g_signal_connect(djynn_pm->project_list,"button-press-event",G_CALLBACK(prj_tree_view_clicked_cb),djynn_pm->project_selection);

//		g_signal_connect(djynn_pm->project_list,"drag-data-received",G_CALLBACK(prj_tree_view_drag_data_received_cb),NULL);
//		g_signal_connect(djynn_pm->project_list,"drag-leave",G_CALLBACK(prj_tree_view_drag_leave_cb),NULL);
//		g_signal_connect(djynn_pm->project_list,"drag-motion",G_CALLBACK(prj_tree_view_drag_motion_cb),NULL);
		g_signal_connect(djynn_pm->project_list,"drag-drop",G_CALLBACK(prj_tree_view_drag_drop_cb),NULL);
//		g_signal_connect(djynn_pm->project_list,"drag-data-get",G_CALLBACK(prj_tree_view_drag_data_get_cb),NULL);
//		g_signal_connect(djynn_pm->project_list,"drag-data-delete",G_CALLBACK(prj_tree_view_drag_data_delete_cb),NULL);
//		g_signal_connect(djynn_pm->project_list,"drag-begin",G_CALLBACK(prj_tree_view_drag_begin_cb),NULL);
		g_signal_connect(djynn_pm->project_list,"drag-end",G_CALLBACK(prj_tree_view_drag_end_cb),NULL);

		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(djynn_pm->project_list),FALSE);
		gtk_container_add(GTK_CONTAINER(djynn_pm->scroll),djynn_pm->project_list);

		gtk_box_pack_start(GTK_BOX(djynn_pm->sidebar_page),djynn_pm->scroll,TRUE,TRUE,0);
		if(djynn_pm->workspace_list!=NULL)
			gtk_box_pack_start(GTK_BOX(djynn_pm->sidebar_page),djynn_pm->workspace_list,FALSE,TRUE,0);
		if(djynn_pm->session_list!=NULL)
			gtk_box_pack_start(GTK_BOX(djynn_pm->sidebar_page),djynn_pm->session_list,FALSE,TRUE,0);

		gtk_widget_show_all(djynn_pm->sidebar_page);

		djynn_project_position_lists();
		djynn_project_set_sidebar();

		plugin_signal_connect(geany_plugin,NULL,"document-activate",TRUE,(GCallback)&doc_activate_cb,NULL);
		plugin_signal_connect(geany_plugin,NULL,"document-open",TRUE,(GCallback)&doc_open_cb,NULL);
//		plugin_signal_connect(geany_plugin,NULL,"document-close",TRUE,(GCallback)&doc_close_cb,NULL);
	}

	djynn_project_configure();
	djynn_workspace_expand();

	plugin_signal_connect(geany_plugin,NULL,"geany-startup-complete",TRUE,(GCallback)&startup_cb,NULL);

//	g_signal_connect(G_OBJECT(djynn_widget->geany.toggle_widgets_menu_item),"activate",(GCallback)&toggle_all_cb,NULL);
	djynn_keybind(project_keybindings);
}


void djynn_project_cleanup() {
	gint i;

	djynn_workspace_cleanup();
	djynn_session_cleanup();

	if(djynn_pm->context_menu!=NULL) {
		gtk_widget_destroy(djynn_pm->context_menu);
		djynn_pm->context_menu = NULL;
	}
	if(djynn_pm->project_menu_sep!=NULL) {
		gtk_widget_destroy(djynn_pm->project_menu_sep);
		djynn_pm->project_menu_sep = NULL;
	}
	if(djynn_pm->project_menu_item!=NULL) {
		gtk_widget_destroy(djynn_pm->project_menu_item);
		djynn_pm->project_menu_item = NULL;
		djynn_pm->project_menu = NULL;
	}
	if(djynn_pm->toggle_sidebars_menu_item!=NULL) {
		gtk_widget_destroy(djynn_pm->toggle_sidebars_menu_item);
		djynn_pm->toggle_sidebars_menu_item = NULL;
	}
	if(djynn_pm->sidebar_menu_item!=NULL) {
		gtk_widget_destroy(djynn_pm->sidebar_menu_item);
		djynn_pm->sidebar_menu_item = NULL;
	}

	djynn_project_clear();

	if(djynn_pm->sidebar_page!=NULL) {
		gtk_widget_destroy(djynn_pm->sidebar_page);
		djynn_pm->sidebar_page = NULL;
		djynn_pm->project_list = NULL;
		djynn_pm->project_store = NULL;
	}

	if(djynn_pm->sidebar_paned!=NULL) {
		djynn_project_remove_sidebar();
	}

	for(i=0; i<DJYNN_PM_TYPES; ++i)
		if(djynn_pm->icons[i]!=NULL) {
			g_object_unref(djynn_pm->icons[i]);
			djynn_pm->icons[i] = NULL;
		}
}

void djynn_project_action(gint id,gboolean check) {
	if(!djynn_cfg->project_manager) return;
	switch(id) {
		case OPEN_FILE:
			open_document(NULL);
			break;
		case OPEN_ALL_FILES:
			open_selected_folder_documents();
			break;
		case OPEN_DIR:
			djynn_project_open_externally(get_selected_file());
			break;
		case OPEN_TERMINAL:
			djynn_project_open_terminal(get_selected_file());
			break;
		case CLOSE_FILE:
			djynn_project_close_file(get_selected_file());
			break;
		case NEW_PROJECT1:
		case NEW_PROJECT2:
			djynn_project_dialog(NULL,TRUE);
			break;
		case NEW_FOLDER:
			djynn_project_folder_dialog(get_selected_file(),TRUE);
			break;
		case NEW_FILE:
			djynn_project_file_dialog(get_selected_file());
			break;
		case ADD_FILE:
			djynn_project_add_file(get_selected_file(),TRUE);
			break;
		case ADD_OPEN_FILES:
			djynn_project_add_open_files(get_selected_file(),TRUE);
			break;
		case REMOVE:
			djynn_project_delete(get_selected_file(),FALSE,TRUE);
			break;
		case DELETE_FROM_DISK:
			djynn_project_delete(get_selected_file(),TRUE,TRUE);
			break;
		case SORT_FILES:
			djynn_project_sort_files(get_selected_file(),TRUE);
			break;
		case MOVE_UP:
			djynn_project_move_up(get_selected_file(),TRUE);
			break;
		case MOVE_DOWN:
			djynn_project_move_down(get_selected_file(),TRUE);
			break;
		case PROJECT_PREFERENCES:
		{
			DjynnProjectFile *f = get_selected_file();
			if(f!=NULL && f->type==DJYNN_PROJECT)
				djynn_project_dialog((DjynnProject *)f,FALSE);
			break;
		}
		case FOLDER_PREFERENCES:
			djynn_project_folder_dialog(get_selected_file(),FALSE);
			break;
		case FILE_PROPERTIES:
			djynn_project_file_properties_dialog(get_selected_file());
			break;
		case RELOAD_WORKSPACE:
			djynn_workspace_load();
			break;
		case TOGGLE_SIDEBARS:
			djynn_project_toggle_sidebars();
			break;
		case HIDE_SIDEBAR:
			keybindings_send_command(GEANY_KEY_GROUP_VIEW,GEANY_KEYS_VIEW_SIDEBAR);
			break;
		case SHOW_PROJECT_MANAGER:
			djynn_project_show_sidebar();
			break;
	}
}

gint djynn_project_keybindings() {
	return djynn_count_keybindings(project_keybindings)+
	       djynn_workspace_keybindings() +
	       djynn_session_keybindings();
}

void djynn_project_enable() {
	djynn_project_set_sidebar();
	g_object_unref(djynn_pm->sidebar_page);
	if(djynn_pm->project_menu_sep!=NULL)
		gtk_widget_show(djynn_pm->project_menu_sep);
	if(djynn_pm->project_menu_item!=NULL)
		gtk_widget_show(djynn_pm->project_menu_item);
}

void djynn_project_disable() {
	g_object_ref(djynn_pm->sidebar_page);
	djynn_project_remove_sidebar();
	if(djynn_pm->project_menu_sep!=NULL)
		gtk_widget_hide(djynn_pm->project_menu_sep);
	if(djynn_pm->project_menu_item!=NULL)
		gtk_widget_hide(djynn_pm->project_menu_item);
}

void djynn_project_toggle_sidebars() {
	GtkWidget *menu_item1 = djynn_widget->geany.show_sidebar_menu_item;
	GtkWidget *menu_item2 = view_show_pm_menu_items[0].widget;
	gboolean hide_all = (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu_item1)) ||
	                     gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu_item2)));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item1),!hide_all);
	if(djynn_cfg->project_manager_pos!=DJYNN_PM_POS_SIDEBAR_PAGE)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item2),!hide_all);
}

void djynn_project_show_sidebar() {
	gboolean show = q_menu_is_checked(SHOW_PROJECT_MANAGER);
	if(show==djynn_cfg->project_manager_show) return;
	djynn_cfg->project_manager_show = show;
	if(show) {
		gtk_widget_show(djynn_pm->sidebar_page);
		if(djynn_cfg->project_manager_pos==DJYNN_PM_POS_SIDEBAR_PAGE)
			gtk_notebook_set_current_page(GTK_NOTEBOOK(geany->main_widgets->sidebar_notebook),djynn_pm->page_number);
	} else {
		gtk_widget_hide(djynn_pm->sidebar_page);
	}
	q_config_open(djynn->config_filename);
	q_config_set_int(djynn_key->djynn,djynn_key->project_manager_show,show);
	q_config_close();
}

void djynn_project_remove_sidebar() {
	if(djynn_pm->sidebar_paned==NULL) {
		if(djynn_pm->sidebar_page!=NULL)
			gtk_notebook_remove_page(GTK_NOTEBOOK(geany->main_widgets->sidebar_notebook),djynn_pm->page_number);
	} else {
		g_object_ref(geany->main_widgets->notebook);
		gtk_container_remove(GTK_CONTAINER(djynn_pm->sidebar_paned),geany->main_widgets->notebook);
		if(djynn_pm->sidebar_page!=NULL)
			gtk_container_remove(GTK_CONTAINER(djynn_pm->sidebar_paned),djynn_pm->sidebar_page);
		gtk_widget_destroy(djynn_pm->sidebar_paned);
		if(gtk_paned_get_child1(GTK_PANED(djynn_widget->geany.hpaned1))==NULL)
			gtk_paned_pack1(GTK_PANED(djynn_widget->geany.hpaned1),geany->main_widgets->notebook,TRUE,TRUE);
		else
			gtk_paned_pack2(GTK_PANED(djynn_widget->geany.hpaned1),geany->main_widgets->notebook,TRUE,TRUE);
		g_object_unref(geany->main_widgets->notebook);
/*		g_object_ref(djynn_widget->geany.hpaned1);
		gtk_container_remove(GTK_CONTAINER(djynn_pm->sidebar_paned),djynn_widget->geany.hpaned1);
		if(djynn_pm->sidebar_page!=NULL)
			gtk_container_remove(GTK_CONTAINER(djynn_pm->sidebar_paned),djynn_pm->sidebar_page);
		gtk_widget_destroy(djynn_pm->sidebar_paned);
		if(gtk_paned_get_child1(GTK_PANED(djynn_widget->geany.vpaned1))==NULL)
			gtk_paned_pack1(GTK_PANED(djynn_widget->geany.vpaned1),djynn_widget->geany.hpaned1,TRUE,TRUE);
		else
			gtk_paned_pack2(GTK_PANED(djynn_widget->geany.vpaned1),djynn_widget->geany.hpaned1,TRUE,TRUE);
		g_object_unref(djynn_widget->geany.hpaned1);*/
		djynn_pm->sidebar_paned = NULL;
	}
}

void djynn_project_set_sidebar() {
	if(djynn_cfg->project_manager_pos!=DJYNN_PM_POS_SIDEBAR_PAGE) {
		djynn_pm->sidebar_paned = gtk_hpaned_new();
		g_object_ref(geany->main_widgets->notebook);
		gtk_container_remove(GTK_CONTAINER(djynn_widget->geany.hpaned1),geany->main_widgets->notebook);
		if(djynn_cfg->project_manager_pos==DJYNN_PM_POS_LEFT) {
			gtk_paned_pack1(GTK_PANED(djynn_pm->sidebar_paned),djynn_pm->sidebar_page,TRUE,TRUE);
			gtk_paned_pack2(GTK_PANED(djynn_pm->sidebar_paned),geany->main_widgets->notebook,TRUE,TRUE);
			gtk_paned_set_position(GTK_PANED(djynn_pm->sidebar_paned),200);
		} else {
			GtkAllocation allocation;
			gint handle_size;
			gtk_widget_get_allocation(djynn_widget->geany.hpaned1,&allocation);
			gtk_widget_style_get(djynn_widget->geany.hpaned1,"handle-size",&handle_size,NULL);
			gtk_paned_pack1(GTK_PANED(djynn_pm->sidebar_paned),geany->main_widgets->notebook,TRUE,TRUE);
			gtk_paned_pack2(GTK_PANED(djynn_pm->sidebar_paned),djynn_pm->sidebar_page,TRUE,TRUE);
			gtk_paned_set_position(GTK_PANED(djynn_pm->sidebar_paned),allocation.width-handle_size-200);
		}
		if(gtk_paned_get_child1(GTK_PANED(djynn_widget->geany.hpaned1))==NULL)
			gtk_paned_pack1(GTK_PANED(djynn_widget->geany.hpaned1),djynn_pm->sidebar_paned,TRUE,TRUE);
		else
			gtk_paned_pack2(GTK_PANED(djynn_widget->geany.hpaned1),djynn_pm->sidebar_paned,TRUE,TRUE);
		g_object_unref(geany->main_widgets->notebook);
/*		g_object_ref(djynn_widget->geany.hpaned1);
		gtk_container_remove(GTK_CONTAINER(djynn_widget->geany.vpaned1),djynn_widget->geany.hpaned1);
		if(djynn_cfg->project_manager_pos==1) {
			gtk_paned_pack1(GTK_PANED(djynn_pm->sidebar_paned),djynn_pm->sidebar_page,TRUE,TRUE);
			gtk_paned_pack2(GTK_PANED(djynn_pm->sidebar_paned),djynn_widget->geany.hpaned1,TRUE,TRUE);
			gtk_paned_set_position(GTK_PANED(djynn_pm->sidebar_paned),200);
		} else {
			GtkAllocation allocation;
			gint handle_size;
			gtk_widget_get_allocation(djynn_widget->geany.vpaned1,&allocation);
			gtk_widget_style_get(djynn_widget->geany.vpaned1,"handle-size",&handle_size,NULL);
			gtk_paned_pack1(GTK_PANED(djynn_pm->sidebar_paned),djynn_widget->geany.hpaned1,TRUE,TRUE);
			gtk_paned_pack2(GTK_PANED(djynn_pm->sidebar_paned),djynn_pm->sidebar_page,TRUE,TRUE);
			gtk_paned_set_position(GTK_PANED(djynn_pm->sidebar_paned),allocation.width-handle_size-200);
		}
		if(gtk_paned_get_child1(GTK_PANED(djynn_widget->geany.vpaned1))==NULL)
			gtk_paned_pack1(GTK_PANED(djynn_widget->geany.vpaned1),djynn_pm->sidebar_paned,TRUE,TRUE);
		else
			gtk_paned_pack2(GTK_PANED(djynn_widget->geany.vpaned1),djynn_pm->sidebar_paned,TRUE,TRUE);
		g_object_unref(djynn_widget->geany.hpaned1);*/
		gtk_widget_show(djynn_pm->sidebar_paned);
	} else {
		djynn_pm->sidebar_paned = NULL;
		djynn_pm->page_number = gtk_notebook_append_page(GTK_NOTEBOOK(geany->main_widgets->sidebar_notebook),
				djynn_pm->sidebar_page,gtk_label_new(_("Project")));
	}

//	if(!djynn_cfg->project_manager_show)
//		gtk_widget_hide(djynn_pm->sidebar_page);
	q_menu_set_checked(SHOW_PROJECT_MANAGER,djynn_cfg->project_manager_show);
	if(!djynn_cfg->project_manager_show)
		gtk_widget_hide(djynn_pm->sidebar_page);
}

void djynn_project_position_lists() {
	gint start = 0,end = 1;
	gtk_box_reorder_child(GTK_BOX(djynn_pm->sidebar_page),djynn_pm->scroll,0);
	if(djynn_pm->workspace_list!=NULL) {
		if(djynn_cfg->workspace_list==0) gtk_widget_hide(djynn_pm->workspace_list);
		else {
			gtk_widget_show(djynn_pm->workspace_list);
			gtk_box_reorder_child(GTK_BOX(djynn_pm->sidebar_page),djynn_pm->workspace_list,
				(djynn_cfg->workspace_list==1? (start++) : start+(end++)));
		}
	}
	if(djynn_pm->session_list!=NULL) {
		if(djynn_cfg->session_list==0) gtk_widget_hide(djynn_pm->session_list);
		else {
			gtk_widget_show(djynn_pm->session_list);
			gtk_box_reorder_child(GTK_BOX(djynn_pm->sidebar_page),djynn_pm->session_list,
				(djynn_cfg->session_list==1? (start++) : start+(end++)));
		}
	}
}


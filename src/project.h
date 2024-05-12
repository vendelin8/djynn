#ifndef _DJYNN_PROJECT_H_
#define _DJYNN_PROJECT_H_

/**
 * @file project.h
 * @author Per Löwgren
 * @date Modified: 2015-08-31
 * @date Created: 2014-05-22
 */ 

#include <libq/array.h>
#include "djynn.h"

typedef enum {
	DJYNN_WORKSPACE,
	DJYNN_SESSION,
	DJYNN_PROJECT,
	DJYNN_PROJECT_FOLDER,
	DJYNN_PROJECT_FILE,
	DJYNN_PM_TYPES
} DjynnPMType;

typedef enum {
	DJYNN_PM_EXPANDED          = 0x0001,
	DJYNN_PM_ADD_FILES         = 0x0002,
	DJYNN_PM_SUBFOLDERS        = 0x0004,
	DJYNN_PM_DYNAMIC           = 0x0008,
	DJYNN_PM_VIRTUAL           = 0x0010,
} DjynnPMStatus;

typedef enum {
	DJYNN_PM_POS_SIDEBAR_PAGE,
	DJYNN_PM_POS_LEFT,
	DJYNN_PM_POS_RIGHT,
} DjynnPMPosition;

typedef struct _DjynnProjectFile DjynnProjectFile;
typedef struct _DjynnProject DjynnProject;
typedef struct _DjynnProjectManager DjynnProjectManager;

struct _DjynnProjectFile {
	DjynnProjectFile *parent;
	DjynnProjectFile *files;
	DjynnProjectFile *next;
	gchar *path;
	gchar *name;
	DjynnPMType type;
	gint status;
	gchar *pattern;
	GtkTreePath *tree_path;
};

struct _DjynnProject {
	DjynnProjectFile file;
	gint index;
	gchar *project_filename;
	gchar *geany_project_filename;
};

struct _DjynnProjectManager {
	gint page_number;
	DjynnProjectFile *projects;
	QArray project_files;

	GdkPixbuf *icons[DJYNN_PM_TYPES];
	GtkWidget *context_menu;

	GtkWidget *project_menu_sep;
	GtkWidget *project_menu_item;
	GtkWidget *project_menu;

	GtkWidget *toggle_sidebars_menu_item;

	GtkWidget *sidebar_menu_item;
	GtkWidget *sidebar_paned;
	GtkWidget *sidebar_page;
	GtkWidget *scroll;
	GtkWidget *project_list;
	GtkTreeStore *project_store;
	GtkTreeSelection *project_selection;
	GtkTreeViewColumn *project_column;
	gint project_expander_size;
	GtkWidget *workspace_list;
	GtkListStore *workspace_store;
	GtkWidget *session_list;
	GtkListStore *session_store;
};

void djynn_project_configure();
void djynn_project_init(GeanyData *data);
void djynn_project_create();
void djynn_project_cleanup();
void djynn_project_action(gint id,gboolean check);
gint djynn_project_keybindings();
void djynn_project_enable();
void djynn_project_disable();
void djynn_project_toggle_sidebars();
void djynn_project_show_sidebar();
void djynn_project_remove_sidebar();
void djynn_project_set_sidebar();
void djynn_project_position_lists();

void djynn_project_clear();
void djynn_project_add_file(DjynnProjectFile *f,gboolean reload);
DjynnProjectFile *djynn_project_add_folder(DjynnProjectFile *f,
                              const gchar *path,const gchar *name,
                              gboolean add_files,gboolean subfolders,gboolean dynamic,
                              const gchar *pattern,
                              gboolean reload);
DjynnProjectFile *djynn_project_create_file(DjynnProjectFile *f,
                              const gchar *path,const gchar *name,gboolean reload);
void djynn_project_sort_files(DjynnProjectFile *f,gboolean reload);
void djynn_project_add_open_files(DjynnProjectFile *f,gboolean reload);
void djynn_project_delete(DjynnProjectFile *f,gboolean disk,gboolean reload);
void djynn_project_move_up(DjynnProjectFile *f,gboolean reload);
void djynn_project_move_down(DjynnProjectFile *f,gboolean reload);
void djynn_project_close_file(DjynnProjectFile *f);
void djynn_project_open_externally(DjynnProjectFile *f);
void djynn_project_open_terminal(DjynnProjectFile *f);
void djynn_project_update();


#endif /* _DJYNN_PROJECT_H_ */





#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <monetary.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <glib/gstdio.h>
#include <libq/string.h>
#include <libq/glib/config.h>
#include "djynn.h"
#include "dialog.h"
#include "session.h"
#include "workspace.h"


typedef enum {
  BUTTONS_NONE,
  BUTTONS_YES,
  BUTTONS_OK,
  BUTTONS_CANCEL,
  BUTTONS_CLOSE,
  BUTTONS_HELP,
  BUTTONS_YES_NO,
  BUTTONS_OK_CANCEL,
  BUTTONS_YES_NO_CANCEL,
  BUTTONS_OK_CANCEL_HELP,
  BUTTONS_OK_CANCEL_APPLY_HELP,
} ButtonsType;

gint djynn_msgbox_ask(const gchar *title,const gchar *msg,const gchar *item) {
	gint r;
	GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(geany->main_widgets->window),GTK_DIALOG_MODAL,
			GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,msg,item);
	gtk_window_set_title(GTK_WINDOW(dlg),title);
	r = (gint)gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
	return r;
}

void djynn_msgbox_warn(const gchar *title,const gchar *msg,const gchar *item) {
	GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(geany->main_widgets->window),GTK_DIALOG_MODAL,
			GTK_MESSAGE_WARNING,GTK_BUTTONS_OK,msg,item);
	gtk_window_set_title(GTK_WINDOW(dlg),title);
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
}

static gboolean delete_event_cb(GtkWidget *widget,GtkWidget *event,gpointer data) {
	gtk_widget_destroy(widget);
	return FALSE;
}

static void browse_select_folder_cb(GtkWidget *widget,gpointer data) {
	GtkWidget *file_chooser;
	GtkWidget *directory = (GtkWidget *)data;
	const gchar *dir;
	gint r;
	file_chooser = gtk_file_chooser_dialog_new(_("Select Folder"),
		GTK_WINDOW(geany_data->main_widgets->window),
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);
	dir = gtk_entry_get_text(GTK_ENTRY(directory));
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),dir);
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(file_chooser),TRUE);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(file_chooser),TRUE);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser),FALSE);
	gtk_file_chooser_set_create_folders(GTK_FILE_CHOOSER(file_chooser),TRUE);
	r = gtk_dialog_run(GTK_DIALOG(file_chooser));
	gtk_widget_hide(file_chooser);
	if(r==GTK_RESPONSE_OK) {
		dir = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
		gtk_entry_set_text(GTK_ENTRY(directory),dir);
	}
	gtk_widget_destroy(file_chooser);
}

void djynn_workspace_dialog(gboolean create) {
	DjynnDialog dialog = { create? _("New Workspace") : _("Rename Workspace"),320,-1,NULL };
	GtkWidget *vbox;
	GtkWidget *name;
	gint r;

	q_config_open(djynn->config_filename);

	vbox = gtk_vbox_new(FALSE,0);
	name = djynn_entry(vbox,NULL,
				_("Workspace Name:"),0,
				create? NULL : djynn->workspace,NULL);

	djynn_dialog(&dialog,vbox,BUTTONS_OK_CANCEL);
	while(1) {
		r = gtk_dialog_run(GTK_DIALOG(dialog.dialog));
		if(r==GTK_RESPONSE_OK) {
			const gchar *nm = gtk_entry_get_text(GTK_ENTRY(name));
			if(nm==NULL || *nm=='\0') djynn_msgbox_warn(dialog.title,_("Select a name for your workspace!"),NULL);
			else if(create) {
				gint id,n;
				gchar ws[5],key[64],str[257];
				djynn_workspace_save();
				id = q_config_get_int(djynn_key->workspace,djynn_key->workspace_id,0)+1;
				sprintf(ws,"%04d",id);
				q_config_set_int(djynn_key->workspace,djynn_key->workspace_id,id);
				n = q_config_get_int(djynn_key->workspace,djynn_key->workspace_n,0)+1;
				sprintf(key,djynn_key->workspace_d,n);
				sprintf(str,"%s:%s",ws,nm);
				q_config_set_str(djynn_key->workspace,key,str);
				q_config_set_int(djynn_key->workspace,djynn_key->workspace_n,n);
				sprintf(str,"%s_%s",djynn_key->workspace,ws);
				q_config_set_int(str,djynn_key->project_n,0);
				djynn_workspace_add(nm);
				djynn_workspace_select(n);
				break;
			} else {
				if(strcmp(djynn->workspace,nm)!=0) {
					gchar key[64],str[257];
					gint n;
					n = q_config_get_int(djynn_key->djynn,djynn_key->workspace,1);
					sprintf(key,djynn_key->workspace_d,n);
					sprintf(str,"%s:%s",strchr(djynn->workspace_key,'_')+1,nm);
					q_config_set_str(djynn_key->workspace,key,str);
					djynn_workspace_set(n,nm);
				}
				break;
			}
		} else break;
	}
	q_config_close();
	gtk_widget_destroy(dialog.dialog);
}

void djynn_session_dialog(gboolean create) {
	DjynnDialog dialog = { create? _("New Session") : _("Rename Session"),320,-1,NULL };
	GtkWidget *vbox;
	GtkWidget *name;
	gint r;
	gint n;
	gchar sess[5],key[64],str[257],*p1,*p2;

	q_config_open(djynn->config_filename);
	n = q_config_get_int(djynn_key->djynn,djynn_key->session,1);
	sprintf(key,djynn_key->session_d,n);
	p1 = q_config_get_str(djynn_key->session,key,NULL);
	p2 = strchr(p1,':'),*p2 = '\0',++p2;
	strcpy(sess,p1);
	strcpy(str,p2);
	g_free(p1);

	vbox = gtk_vbox_new(FALSE,0);
	name = djynn_entry(vbox,NULL,
				_("Session Name:"),0,
				create? NULL : str,NULL);

	djynn_dialog(&dialog,vbox,BUTTONS_OK_CANCEL);
	while(1) {
		r = gtk_dialog_run(GTK_DIALOG(dialog.dialog));
		if(r==GTK_RESPONSE_OK) {
			const gchar *nm = gtk_entry_get_text(GTK_ENTRY(name));
			if(nm==NULL || *nm=='\0') djynn_msgbox_warn(dialog.title,_("Select a name for your session!"),NULL);
			else {
				if(create) {
					gint id = q_config_get_int(djynn_key->session,djynn_key->session_id,0)+1;
					sprintf(sess,"%04d",id);
					q_config_set_int(djynn_key->session,djynn_key->session_id,id);
					n = q_config_get_int(djynn_key->session,djynn_key->session_n,0)+1;
					sprintf(key,djynn_key->session_d,n);
					sprintf(str,"%s:%s",sess,nm);
					q_config_set_str(djynn_key->session,key,str);
					q_config_set_int(djynn_key->session,djynn_key->session_n,n);
					djynn_session_list_add(nm);
					djynn_session_list_select(n);
					djynn_session_save();
					break;
				} else if(strcmp(str,nm)!=0) {
					sprintf(str,"%s:%s",sess,nm);
					q_config_set_str(djynn_key->session,key,str);
					djynn_session_list_set(n,nm);
				}
				break;
			}
		} else break;
	}
	q_config_close();
	gtk_widget_destroy(dialog.dialog);
}

void djynn_project_dialog(DjynnProject *project,gboolean create) {
	DjynnDialog dialog = { create? _("New Project") : _("Project Preferences"),320,-1,NULL };
	DjynnProjectFile *file = (DjynnProjectFile *)project;
	GtkWidget *vbox;
	GtkWidget *name;
	GtkWidget *directory;
	GtkWidget *description;
	gint r;
	gchar *str = NULL;

	if(!create) {
		if(file==NULL) {
			djynn_msgbox_warn(dialog.title,_("No project has been selected for editing preferences!"),NULL);
			return;
		} else {
			gchar prj[257];
			sprintf(prj,"%s%s." GEANY_PROJECT_EXT,djynn->config_dir,file->name);
			if(g_file_test(prj,G_FILE_TEST_EXISTS)) {
				q_config_open(prj);
				str = q_config_get_str("project","description",NULL);
				q_config_close();
			}
		}
	}

	q_config_open(djynn->config_filename);

	vbox = gtk_vbox_new(FALSE,0);
	name = djynn_entry(vbox,NULL,
				_("Project Name:"),0,
				create? NULL : file->name,NULL);
	directory = djynn_file_browser_entry(vbox,NULL,
				_("Base Path:"),0,
				create? g_get_home_dir() : file->path,NULL);
	description = djynn_entry(vbox,NULL,
				_("Description:"),0,
				str,NULL);
	if(str!=NULL) g_free(str);

	djynn_dialog(&dialog,vbox,BUTTONS_OK_CANCEL);
	while(1) {
		r = gtk_dialog_run(GTK_DIALOG(dialog.dialog));
		if(r==GTK_RESPONSE_OK) {
			const gchar *nm = gtk_entry_get_text(GTK_ENTRY(name));
			const gchar *dir = gtk_entry_get_text(GTK_ENTRY(directory));
			if(nm==NULL || *nm=='\0') djynn_msgbox_warn(dialog.title,_("Select a name for your project!"),NULL);
			else if(dir==NULL || *dir=='\0' || !g_file_test(dir,G_FILE_TEST_IS_DIR)) {
				djynn_msgbox_warn(dialog.title,_("Not a valid directory!"),NULL);
			} else {
				const gchar *descr = gtk_entry_get_text(GTK_ENTRY(description));
				gint n;
				gchar key[64],prj1[257],prj2[257];
				FILE *fp;
// TODO				project_close(FALSE);
				sprintf(prj1,"%s%s.project",djynn->config_dir,nm);
				sprintf(prj2,"%s%s." GEANY_PROJECT_EXT,djynn->config_dir,nm);
				if(create) {
					n = q_config_get_int(djynn->workspace_key,djynn_key->project_n,0)+1;
					q_config_set_int(djynn->workspace_key,djynn_key->project_n,n);
					sprintf(key,djynn_key->project_d,n);
					q_config_set_str(djynn->workspace_key,key,prj1);
				} else if(strcmp(file->name,nm)!=0) {
					gchar prj3[257],prj4[257];
					sprintf(prj3,"%s%s.project",djynn->config_dir,file->name);
					sprintf(prj4,"%s%s." GEANY_PROJECT_EXT,djynn->config_dir,file->name);
					if(g_file_test(prj1,G_FILE_TEST_EXISTS) || g_file_test(prj2,G_FILE_TEST_EXISTS)) {
						gchar msg[257];
						sprintf(msg,_("Project \"%s\" already exists, delete the old project or select another name!"),nm);
						djynn_msgbox_warn(dialog.title,msg,NULL);
						continue;
					}
					if(g_file_test(prj3,G_FILE_TEST_EXISTS)) g_rename(prj3,prj1);
					if(g_file_test(prj4,G_FILE_TEST_EXISTS)) g_rename(prj4,prj2);
					sprintf(key,djynn_key->project_d,project->index);
					q_config_set_str(djynn->workspace_key,key,prj1);
				}
				if(!g_file_test(prj1,G_FILE_TEST_EXISTS)) {
					if((fp=fopen(prj1,"w"))) {
						fprintf(fp,"+%s\n"
							"%s\n"
							"%s\n",
							nm,dir,prj2);
						fclose(fp);
					} else perror(prj1);
				}
				if(!g_file_test(prj2,G_FILE_TEST_EXISTS)) {
					if((fp=fopen(prj2,"w"))) {
						const GeanyIndentPrefs *ind = editor_get_indent_prefs(NULL);
						fprintf(fp,"\n"
							"[indentation]\n"
							"indent_width=%d\n"
							"indent_type=%d\n"
							"indent_hard_tab_width=%d\n"
							"detect_indent=false\n"
							"indent_mode=2\n"
							"\n"
							"[project]\n"
							"name=%s\n"
							"base_path=%s\n"
							"make_in_base_path=true\n"
							"description=%s\n"
							"run_cmd=%s\n"
							"file_patterns=*.c;*.cc;*.cfg;*.conf;*.cpp;*.css;*.cxx;*.h;*.hh;*.hpp;*.html;*.hxx;*.ini;*.java;*.js;*.lua;*.pas;*.perl;*.php;*.pl;*.po;*.py;*.rc;*.sh;*.txt;*.vala;*.xhtml;*.xml;\n",
							ind->width,
							ind->type,
							ind->hard_tab_width,
							nm,
							dir,
							descr,
							nm
						);
						fclose(fp);
					} else perror(prj2);
				} else {
					q_config_open(prj2);
					q_config_set_str("project","name",nm);
					q_config_set_str("project","base_path",dir);
					q_config_set_str("project","description",descr);
					q_config_set_str("project","run_cmd",nm);
					q_config_close();
				}
				if(!create) {
					if(file->path!=NULL) g_free(file->path);
					file->path = g_strdup(dir);
					if(file->name!=NULL) g_free(file->name);
					file->name = g_strdup(nm);
					if(project->project_filename!=NULL) g_free(project->project_filename);
					project->project_filename = g_strdup(prj1);
					if(project->geany_project_filename!=NULL) g_free(project->geany_project_filename);
					project->geany_project_filename = g_strdup(prj2);
				}
				djynn_workspace_reload();
// TODO				project_load_file(prj2);
				break;
			}
		} else break;
	}
	q_config_close();
	gtk_widget_destroy(dialog.dialog);
}

void djynn_project_folder_dialog(DjynnProjectFile *file,gboolean create) {
	DjynnDialog dialog = { create? _("New Folder") : _("Folder Preferences"),320,-1,NULL };
	DjynnProject *project = NULL;
	GtkWidget *vbox;
	GtkWidget *name;
	GtkWidget *add_files;
	GtkWidget *subfolders;
	GtkWidget *dynamic;
	GtkWidget *directory;
	GtkWidget *regex;

	if(file==NULL) {
		djynn_msgbox_warn(dialog.title,_("No project, folder or file selected!"),NULL);
		return;
	} else if(!create && file->type!=DJYNN_PROJECT_FOLDER) {
		return;
	} else {
		DjynnProjectFile *f1 = file;
		while(f1->parent!=NULL && f1->type!=DJYNN_PROJECT) f1 = f1->parent;
		if(f1!=NULL) project = (DjynnProject *)f1;
	}

	vbox = gtk_vbox_new(FALSE,0);

	name = djynn_entry(vbox,NULL,
				_("Folder Name:"),0,
				create? NULL : file->name,NULL);

	add_files = djynn_check_button(vbox,_("Add files in directory"),create? FALSE : (file->status&DJYNN_PM_ADD_FILES),NULL,NULL);
	subfolders = djynn_check_button(vbox,_("Recurse in subfolders"),create? FALSE : (file->status&DJYNN_PM_SUBFOLDERS),NULL,NULL);
	dynamic = djynn_check_button(vbox,_("Dynamically load files in folder"),create? FALSE : (file->status&DJYNN_PM_DYNAMIC),NULL,NULL);

	{
		gchar *dir = NULL,*p;
		if(create) {
			if(file->type==DJYNN_PROJECT_FILE) {
				dir = g_strdup(file->path);
				p = strrchr(dir,G_DIR_SEPARATOR);
				if(p!=NULL) *p = '\0';
			}
			else if(project!=NULL) dir = g_strdup(((DjynnProjectFile *)project)->path);
			else dir = g_strdup(g_get_home_dir());
		}
		directory = djynn_file_browser_entry(vbox,NULL,NULL,0,create? dir : file->path,NULL);
		if(dir!=NULL) g_free(dir);
	}

	regex = djynn_entry(vbox,NULL,_("Match files with regex-pattern:"),0,create? NULL : file->pattern,NULL);

	djynn_dialog(&dialog,vbox,BUTTONS_OK_CANCEL);
	while(gtk_dialog_run(GTK_DIALOG(dialog.dialog))==GTK_RESPONSE_OK) {
		const gchar *nm = gtk_entry_get_text(GTK_ENTRY(name));
		const gchar *dir = gtk_entry_get_text(GTK_ENTRY(directory));
		if(nm==NULL || *nm=='\0') djynn_msgbox_warn(dialog.title,_("Select a name for the folder!"),NULL);
		else if(dir==NULL || *dir=='\0' || !g_file_test(dir,G_FILE_TEST_IS_DIR)) {
				djynn_msgbox_warn(dialog.title,_("Not a valid directory!"),NULL);
		} else {
			const gchar *rx = gtk_entry_get_text(GTK_ENTRY(regex));
			gboolean b1 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(add_files));
			gboolean b2 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(subfolders));
			gboolean b3 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dynamic));
			if(create) {
				djynn_project_add_folder(file,dir,nm,b1,b2,b3,rx,TRUE);
			} else {
				if(file->path!=NULL) g_free(file->path);
				file->path = g_strdup(dir);
				if(file->name!=NULL) g_free(file->name);
				file->name = g_strdup(nm);
				if(b1) file->status |= DJYNN_PM_ADD_FILES;
				else file->status &= ~DJYNN_PM_ADD_FILES;
				if(b2) file->status |= DJYNN_PM_SUBFOLDERS;
				else file->status &= ~DJYNN_PM_SUBFOLDERS;
				if(b3) file->status |= DJYNN_PM_DYNAMIC;
				else file->status &= ~DJYNN_PM_DYNAMIC;
				if(file->pattern!=NULL) g_free(file->pattern);
				file->pattern = g_strdup(rx);
				djynn_workspace_reload();
			}
			break;
		}
	}
	gtk_widget_destroy(dialog.dialog);
}

void djynn_project_file_dialog(DjynnProjectFile *file) {
	DjynnDialog dialog = { _("New File"),320,-1,NULL };
	DjynnProject *project = NULL;
	GtkWidget *vbox;
	GtkWidget *name;
	GtkWidget *directory;

	if(file==NULL) {
		djynn_msgbox_warn(dialog.title,_("No project, folder or file selected!"),NULL);
		return;
	} else {
		DjynnProjectFile *f1 = file;
		while(f1->parent!=NULL && f1->type!=DJYNN_PROJECT) f1 = f1->parent;
		if(f1!=NULL) project = (DjynnProject *)f1;
	}

	vbox = gtk_vbox_new(FALSE,0);

	name = djynn_entry(vbox,NULL,_("File Name:"),0,NULL,NULL);

	{
		gchar *dir = NULL,*p;
		if(file->type==DJYNN_PROJECT_FILE) {
			dir = g_strdup(file->path);
			p = strrchr(dir,G_DIR_SEPARATOR);
			if(p!=NULL) *p = '\0';
		}
		else if(file->type==DJYNN_PROJECT_FOLDER) dir = g_strdup(file->path);
		else if(project!=NULL) dir = g_strdup(((DjynnProjectFile *)project)->path);
		else dir = g_strdup(g_get_home_dir());
		directory = djynn_file_browser_entry(vbox,NULL,NULL,0,dir,NULL);
		if(dir!=NULL) g_free(dir);
	}

	djynn_dialog(&dialog,vbox,BUTTONS_OK_CANCEL);
	while(gtk_dialog_run(GTK_DIALOG(dialog.dialog))==GTK_RESPONSE_OK) {
		const gchar *nm = gtk_entry_get_text(GTK_ENTRY(name));
		const gchar *dir = gtk_entry_get_text(GTK_ENTRY(directory));
		if(nm==NULL || *nm=='\0') djynn_msgbox_warn(dialog.title,_("Select a name for the file!"),NULL);
		else if(dir==NULL || *dir=='\0' || !g_file_test(dir,G_FILE_TEST_IS_DIR)) {
			djynn_msgbox_warn(dialog.title,_("Not a valid directory!"),NULL);
		} else {
			djynn_project_create_file(file,dir,nm,TRUE);
			break;
		}
	}
	gtk_widget_destroy(dialog.dialog);
}

// buf needs to store 33 characters
static gint timespec2str(gchar *buf,struct timespec *ts) {
	struct tm t;
	* buf = '\0';
	tzset();
	if(localtime_r(&(ts->tv_sec),&t)==NULL) return 1;
	if(strftime(buf,32,"%F %T",&t)==0) return 2;
	return 0;
}

void djynn_project_file_properties_dialog(DjynnProjectFile *file) {
	DjynnDialog dialog = { _("Properties"),320,-1,NULL };
	if(file==NULL) {
		djynn_msgbox_warn(dialog.title,_("No file selected!"),NULL);
		return;
	} else if(file->type!=DJYNN_PROJECT_FILE) {
		return;
	} else {
		double d;
		gchar *dir;
		gchar size[65];
		gchar access[33] = "";
		gchar mod[33] = "";
		gchar *p,buf[65],label[33],value[257];
		GStatBuf stat_buf;
		struct passwd *usr;
		struct group *grp;
		GtkWidget *vbox = gtk_vbox_new(FALSE,0);

		if(g_stat(file->path,&stat_buf)!=0) return;
		timespec2str(access,&stat_buf.st_atim);
		timespec2str(mod,&stat_buf.st_mtim);
		usr = getpwuid(stat_buf.st_uid);
		grp = getgrgid(stat_buf.st_gid);

		d = (double)stat_buf.st_size,p = size;
		strfmon(buf,32,"%!.0i",d);
		if(d<1000.0) p += sprintf(p,"%s %s",buf,d==1? _("byte") : _("bytes"));
		else {
			if(d<1000000.0) p += sprintf(p,"%0.2f kB, %0.2f KiB",d/1000.0,d/1024.0);
			else if(d<1000000000.0) p += sprintf(p,"%0.2f MB, %0.2f MiB",d/1000000.0,d/(1024.0*1024.0));
			else if(d<1000000000000.0) p += sprintf(p,"%0.2f GB, %0.2f GiB",d/1000000000.0,d/(1024.0*1024.0*1024.0));
			else p += sprintf(p,"%0.2f TB, %0.2f TiB",d/1000000000000.0,d/(1024.0*1024.0*1024.0*1024.0));
			p += sprintf(p," (%s %s)",buf,_("bytes"));
		}

		dir = g_strdup(file->path);
		p = strrchr(dir,G_DIR_SEPARATOR);
		if(p!=NULL) *p = '\0';

#define PRJFILEDLG_W 80
#define PRJFILEDLG_P 10

		sprintf(label,"%s:",_("Name"));
		djynn_label(vbox,NULL,label,PRJFILEDLG_W,1.0,TRUE,PRJFILEDLG_P,file->name,0.0,FALSE);
		sprintf(label,"%s:",_("Location"));
		djynn_label(vbox,NULL,label,PRJFILEDLG_W,1.0,TRUE,PRJFILEDLG_P,dir,0.0,FALSE);
		sprintf(label,"%s:",_("Accessed"));
		djynn_label(vbox,NULL,label,PRJFILEDLG_W,1.0,TRUE,PRJFILEDLG_P,access,0.0,FALSE);
		sprintf(label,"%s:",_("Modified"));
		djynn_label(vbox,NULL,label,PRJFILEDLG_W,1.0,TRUE,PRJFILEDLG_P,mod,0.0,FALSE);
		sprintf(label,"%s:",_("Size"));
		djynn_label(vbox,NULL,label,PRJFILEDLG_W,1.0,TRUE,PRJFILEDLG_P,size,0.0,FALSE);
		sprintf(label,"%s:",_("User"));
		sprintf(value,"%s (%d)",usr->pw_name,(int)stat_buf.st_uid);
		djynn_label(vbox,NULL,label,PRJFILEDLG_W,1.0,TRUE,PRJFILEDLG_P,value,0.0,FALSE);
		sprintf(label,"%s:",_("Group"));
		sprintf(value,"%s (%d)",grp->gr_name,(int)stat_buf.st_gid);
		djynn_label(vbox,NULL,label,PRJFILEDLG_W,1.0,TRUE,PRJFILEDLG_P,value,0.0,FALSE);
		sprintf(label,"%s:",_("Hard Links"));
		sprintf(value,"%d",(int)stat_buf.st_nlink);
		djynn_label(vbox,NULL,label,PRJFILEDLG_W,1.0,TRUE,PRJFILEDLG_P,value,0.0,FALSE);

		djynn_dialog(&dialog,vbox,BUTTONS_CLOSE);
		gtk_dialog_run(GTK_DIALOG(dialog.dialog));
		gtk_widget_destroy(dialog.dialog);
		g_free(dir);
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

static GtkWidget *vbox_label_widget(GtkWidget *vbox,const gchar *text,gint width,gfloat align,gboolean bold,gint padding,GtkWidget *widget) {
	GtkWidget *label = NULL;
	if(vbox!=NULL) {
		if(text!=NULL && *text!='\0') {
			label = create_label_widget(text,align,0.5,bold);
			if(width>0) {
				GtkWidget *hbox = gtk_hbox_new(FALSE,0);
				gtk_widget_set_size_request(label,width,-1);
				gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,padding);
				gtk_box_pack_start(GTK_BOX(hbox),widget,TRUE,TRUE,0);
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

void djynn_dialog(DjynnDialog *dialog,GtkWidget *vbox,guint buttons) {
	if(dialog->dialog==NULL) {
		GtkWidget *area;
//		GtkWidget *hbox;
		dialog->dialog = gtk_dialog_new_with_buttons(dialog->title,
			GTK_WINDOW(geany_data->main_widgets->window),
			GTK_DIALOG_MODAL|0,
		NULL);
		djynn_dialog_buttons(GTK_DIALOG(dialog->dialog),buttons,NULL);
		gtk_window_set_default_size(GTK_WINDOW(dialog->dialog),dialog->width,dialog->height);
		gtk_window_set_position(GTK_WINDOW(dialog->dialog),GTK_WIN_POS_CENTER);
//		gtk_container_set_border_width(GTK_CONTAINER(dialog->dialog),5);
		area = gtk_dialog_get_content_area(GTK_DIALOG(dialog->dialog));
		gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
		gtk_box_pack_start(GTK_BOX(area),vbox,TRUE,TRUE,0);
		gtk_dialog_set_default_response(GTK_DIALOG(dialog->dialog),GTK_RESPONSE_OK);
		dialog->ok = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog->dialog),GTK_RESPONSE_OK);
		g_signal_connect(dialog->dialog,"delete-event",G_CALLBACK(delete_event_cb),dialog);
		gtk_widget_show_all(dialog->dialog);
	}
}

void djynn_dialog_buttons(GtkDialog *dialog,guint buttons,GtkWidget *list[]) {
	GtkWidget *button,*def = NULL;
	switch(buttons) {
		case BUTTONS_NONE:break;
		case BUTTONS_YES:def = gtk_dialog_add_button(dialog,GTK_STOCK_YES,GTK_RESPONSE_YES);break;
		case BUTTONS_OK:def = gtk_dialog_add_button(dialog,GTK_STOCK_OK,GTK_RESPONSE_OK);break;
		case BUTTONS_CANCEL:def = gtk_dialog_add_button(dialog,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL);break;
		case BUTTONS_CLOSE:def = gtk_dialog_add_button(dialog,GTK_STOCK_CLOSE,GTK_RESPONSE_CLOSE);break;
		case BUTTONS_HELP:def = gtk_dialog_add_button(dialog,GTK_STOCK_HELP,GTK_RESPONSE_HELP);break;
		case BUTTONS_YES_NO:
			button = gtk_dialog_add_button(dialog,GTK_STOCK_NO,GTK_RESPONSE_NO);
			if(list!=NULL) list[1] = button;
			def = gtk_dialog_add_button(dialog,GTK_STOCK_YES,GTK_RESPONSE_YES);
//			gtk_dialog_set_default_response(dialog,GTK_RESPONSE_YES);
			break;
		case BUTTONS_OK_CANCEL:
			button = gtk_dialog_add_button(dialog,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL);
			if(list!=NULL) list[1] = button;
			def = gtk_dialog_add_button(dialog,GTK_STOCK_OK,GTK_RESPONSE_OK);
//			gtk_dialog_set_default_response(dialog,GTK_RESPONSE_OK);
			break;
		case BUTTONS_YES_NO_CANCEL:
			button = gtk_dialog_add_button(dialog,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL);
			if(list!=NULL) list[2] = button;
			button = gtk_dialog_add_button(dialog,GTK_STOCK_NO,GTK_RESPONSE_NO);
			if(list!=NULL) list[1] = button;
			def = gtk_dialog_add_button(dialog,GTK_STOCK_YES,GTK_RESPONSE_YES);
//			gtk_dialog_set_default_response(dialog,GTK_RESPONSE_YES);
			break;
		case BUTTONS_OK_CANCEL_HELP:
			button = gtk_dialog_add_button(dialog,GTK_STOCK_HELP,GTK_RESPONSE_HELP);
			if(list!=NULL) list[2] = button;
			button = gtk_dialog_add_button(dialog,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL);
			if(list!=NULL) list[1] = button;
			def = gtk_dialog_add_button(dialog,GTK_STOCK_OK,GTK_RESPONSE_OK);
//			gtk_dialog_set_default_response(dialog,GTK_RESPONSE_OK);
			break;
		case BUTTONS_OK_CANCEL_APPLY_HELP:
			button = gtk_dialog_add_button(dialog,GTK_STOCK_HELP,GTK_RESPONSE_HELP);
			if(list!=NULL) list[3] = button;
			button = gtk_dialog_add_button(dialog,GTK_STOCK_APPLY,GTK_RESPONSE_APPLY);
			if(list!=NULL) list[2] = button;
			button = gtk_dialog_add_button(dialog,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL);
			if(list!=NULL) list[1] = button;
			def = gtk_dialog_add_button(dialog,GTK_STOCK_OK,GTK_RESPONSE_OK);
//			gtk_dialog_set_default_response(dialog,GTK_RESPONSE_OK);
			break;
	}
	if(def!=NULL) {
		if(list!=NULL) list[0] = def;
		gtk_widget_set_can_default(def,TRUE);
		gtk_widget_grab_default(def);
	}
}

GtkWidget *djynn_frame(GtkWidget *vbox,
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
		gtk_widget_set_tooltip_text(label,label_tooltip);
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

GtkWidget *djynn_label(GtkWidget *vbox,GtkWidget **label_widget,
                       const gchar *label_text,gint label_width,gfloat label_align,gboolean label_bold,gint label_padding,
                       const gchar *value_text,gfloat value_align,gboolean value_bold) {
	GtkWidget *label;
	GtkWidget *value_label = NULL;
	value_label = create_label_widget(value_text,value_align,0.5,value_bold);
	label = vbox_label_widget(vbox,label_text,label_width,label_align,label_bold,label_padding,value_label);
	if(label_widget!=NULL) *label_widget = label;
	return value_label;
}

GtkWidget *djynn_entry(GtkWidget *vbox,GtkWidget **label_widget,
                       const gchar *label_text,gint label_width,
                       const gchar *entry_text,const gchar *entry_tooltip) {
	GtkWidget *label;
	GtkWidget *entry = gtk_entry_new();
	if(entry_text!=NULL && *entry_text!='\0')
		gtk_entry_set_text(GTK_ENTRY(entry),entry_text);
	gtk_entry_set_activates_default(GTK_ENTRY(entry),TRUE);
	if(entry_tooltip!=NULL && *entry_tooltip!='\0')
		gtk_widget_set_tooltip_text(entry,entry_tooltip);
	label = vbox_label_widget(vbox,label_text,label_width,0.0,FALSE,label_width<=0? 0 : 5,entry);
	if(label_widget!=NULL) *label_widget = label;
	return entry;
}

GtkWidget *djynn_combo_box(GtkWidget *vbox,GtkWidget **label_widget,
                           const gchar *label_text,gint label_width,
                           const gchar *combo_box_text[],gint combo_box_active,const gchar *combo_box_tooltip) {
	GtkWidget *label;
	GtkWidget *combo_box = gtk_combo_box_new();
	gint i;
	if(combo_box_text!=NULL)
		for(i=0; combo_box_text[i]!=NULL; ++i)
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box),combo_box_text[i]);
	if(combo_box_tooltip!=NULL && *combo_box_tooltip!='\0')
		gtk_widget_set_tooltip_text(combo_box,combo_box_tooltip);
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box),combo_box_active);
	label = vbox_label_widget(vbox,label_text,label_width,0.0,FALSE,label_width<=0? 0 : 5,combo_box);
	if(label_widget!=NULL) *label_widget = label;
	return combo_box;
}

GtkWidget *djynn_check_button(GtkWidget *vbox,const gchar *text,gboolean checked,GCallback toggled,const gchar *tooltip) {
	GtkWidget *check_button = gtk_check_button_new_with_label(text);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button),checked);
	if(toggled!=NULL)
		g_signal_connect(check_button,"toggled",toggled,NULL);
	if(tooltip!=NULL && *tooltip!='\0')
		gtk_widget_set_tooltip_text(check_button,tooltip);
	if(vbox!=NULL)
		gtk_box_pack_start(GTK_BOX(vbox),check_button,FALSE,FALSE,0);
	return check_button;
}

GtkWidget *djynn_file_browser_entry(GtkWidget *vbox,GtkWidget **label_widget,
                       const gchar *label_text,gint label_width,
                       const gchar *entry_text,const gchar *entry_tooltip) {
	GtkWidget *label;
	GtkWidget *hbox = gtk_hbox_new(FALSE,5);
	GtkWidget *entry = gtk_entry_new();
	GtkWidget *button = gtk_button_new_with_label(_("Browse..."));
	gtk_entry_set_activates_default(GTK_ENTRY(entry),TRUE);
	if(entry_text!=NULL && *entry_text!='\0')
		gtk_entry_set_text(GTK_ENTRY(entry),entry_text);
	if(entry_tooltip!=NULL && *entry_tooltip!='\0')
		gtk_widget_set_tooltip_text(entry,entry_tooltip);
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,0);
	g_signal_connect(button,"clicked",G_CALLBACK(browse_select_folder_cb),entry);
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
	label = vbox_label_widget(vbox,label_text,label_width,0.0,FALSE,label_width<=0? 0 : 5,hbox);
	if(label_widget!=NULL) *label_widget = label;
	return entry;
}




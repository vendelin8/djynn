

#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>
#include <libq/string.h>
#include <libq/glib/config.h>
#include <libq/gtk/menu.h>
#include "djynn.h"
#include "workspace.h"
#include "project.h"
#include "dialog.h"



enum {
	WORKSPACE_LIST_ICON,
	WORKSPACE_LIST_WORKSPACE,
	WORKSPACE_LIST_COLS,
};

static void menu_activate(QMenuItem *menuitem) {
	if(menuitem!=NULL) {
		gboolean check = FALSE;
		if(menuitem->type==Q_MENU_CHECKBOX || menuitem->type==Q_MENU_RADIO)
			check = q_menu_is_checked(menuitem->id);
		djynn_workspace_action(menuitem->id,check);
	}
}

enum {
	WORKSPACE_NEW = 0x2301,
	WORKSPACE_RENAME,
	WORKSPACE_RELOAD,
	WORKSPACE_DELETE,
};

static QMenuItem workspace_menu_items[] = {
/*   type          id                   label & tooltip                 icon               submenu   activate */
	{ Q_MENU_LABEL, WORKSPACE_NEW,       N_("New Workspace..."),NULL,    GTK_STOCK_NEW,     NULL,     menu_activate },
	{ Q_MENU_LABEL, WORKSPACE_RENAME,    N_("Rename Workspace..."),NULL, GTK_STOCK_SAVE_AS, NULL,     menu_activate },
	{ Q_MENU_LABEL, WORKSPACE_RELOAD,    N_("Reload Workspace"),NULL,    GTK_STOCK_REFRESH, NULL,     menu_activate },
	{ Q_MENU_LABEL, WORKSPACE_DELETE,    N_("Delete Workspace"),NULL,    GTK_STOCK_DELETE,  NULL,     menu_activate },
	{ 0,0 }
};


static void parse_project(DjynnProject *p,gchar *data) {
//debug_output("parse_project()\n");
	DjynnPMType type;
	gint status,indent,prev_indent = 0;
	gchar *path,*name,*pattern,*endl;
	DjynnProjectFile *f = (DjynnProjectFile *)p,*f1,*f2;
	gchar str[1025],*ptr;

	while(data!=NULL && *data!='\0') {
//debug_output("parse_project(1)\n");

		for(; *data!='\0' && *data=='\n'; ++data);
		if(*data=='\0') break;

		for(indent=1; *data!='\0' && *data=='\t'; ++indent,++data);
		if(*data=='\0') break;

		endl = strchr(data,'\n');
		if(endl!=NULL) *endl++ = '\0';

//debug_output("parse_project(indent=%d,data=%s)\n",indent,data);
		type = DJYNN_PROJECT_FILE;
		status = 0;
		pattern = NULL;

		path = data;
		if(*path=='+' || *path=='-') {
			type = DJYNN_PROJECT_FOLDER;
			if(*path=='+') status |= DJYNN_PM_EXPANDED;
			++path;

			if(*path=='[') {
				++path;
				if(*path=='1') status |= DJYNN_PM_ADD_FILES,++path;
				else if(*path=='0') ++path;
				if(*path=='1') status |= DJYNN_PM_SUBFOLDERS,++path;
				else if(*path=='0') ++path;
				if(*path=='1') status |= DJYNN_PM_DYNAMIC,++path;
				else if(*path=='0') ++path;
				if(*path!=']') {
					gchar *p;
					int nested;
					for(p=path,nested=0; *p!='\0' && (*p!=']' || nested>0); ++p)
						if(*p=='[') ++nested;
						else if(*p==']') --nested;
						else if(*p=='\\') ++p;
					if(*p=='\0') return;
					*p = '\0';
					pattern = path;
					path = p;
				}
				++path;
			}
		}

		ptr = NULL;
		if(*path=='"')
			for(ptr=path+1; *ptr!='"' && *ptr!='\0'; ++ptr)
				if(*ptr=='\\') ++ptr;
		if(ptr!=NULL && *ptr=='"' && ptr[1]==':') {
			*ptr = '\0';
			name = path+1;
			path = ptr+2;
		} else {
			name = strrchr(path,G_DIR_SEPARATOR);
			name = name==NULL? path : name+1;
		} 

		if(*path!=G_DIR_SEPARATOR) {
			sprintf(str,"%s%c%s",p->file.path,G_DIR_SEPARATOR,path);
			path = str;
		}

		data = endl;

		if(type==DJYNN_PROJECT_FOLDER && (status&DJYNN_PM_DYNAMIC)) {
//debug_output("parse_project(path=%s,name=%s,pattern=%s)\n",path,name,pattern);
			f1 = djynn_project_add_folder(NULL,path,name,
                              !!(status&DJYNN_PM_ADD_FILES),
                              !!(status&DJYNN_PM_SUBFOLDERS),
                              !!(status&DJYNN_PM_DYNAMIC),
                              pattern,
                              FALSE);
			if(f1==NULL) continue;
			if(status&DJYNN_PM_EXPANDED) f1->status |= DJYNN_PM_EXPANDED;
		} else/* if(type!=DJYNN_PROJECT_FILE || g_file_test(path,G_FILE_TEST_EXISTS))*/ {
			f1 = (DjynnProjectFile *)g_malloc(sizeof(DjynnProjectFile));
			f1->type = type;
			f1->status = status;
			f1->path = g_strdup(path);
			f1->name = g_strdup(name);
			f1->pattern = NULL;
			f1->tree_path = NULL;
			f1->parent = NULL;
			f1->files = NULL;
			f1->next = NULL;
		}/* else {
			msgwin_status_add("File %s doesn't exist.",path);
			continue;
		}*/

//debug_output("parse_project(prev_indent=%d,indent=%d,path=%s,name=%s)\n",prev_indent,indent,f1->path,f1->name);

		if(indent==prev_indent) f->next = f1,f1->parent = f->parent;
		else if(indent>prev_indent) f1->parent = f,f->files = f1,indent = prev_indent+1;
		else if(indent<prev_indent) {
			while(indent-1<prev_indent) f = f->parent,--prev_indent;
			f1->parent = f;
			for(f2=f->files; f2->next!=NULL; f2=f2->next);
			f2->next = f1;
		}
		prev_indent = indent;
		f = f1;
//debug_output("parse_project(2)\n");
	}
//debug_output("parse_project(done)\n");
}

static void write_project(DjynnProject *p) {
	FILE *fp = fopen(p->project_filename,"w");
	if(fp!=NULL) {
		gint i,n = strlen(p->file.path),indent = 0;
		gboolean e = p->file.tree_path!=NULL? gtk_tree_view_row_expanded(GTK_TREE_VIEW(djynn_pm->project_list),p->file.tree_path) : FALSE;
		DjynnProjectFile *f;
		gchar *s;
		fprintf(fp,"%c%s\n%s\n%s\n",e? '+' : '-',p->file.name,p->file.path,p->geany_project_filename);
//debug_output("write_project(project_filename=%s,name=%c%s,dir=%s)\n",p->project_filename,e? '+' : '-',p->name,p->path);
		for(f=p->file.files; f!=NULL; ) {
			for(i=0; i<indent; ++i) fputc('\t',fp);
			if(f->type==DJYNN_PROJECT_FOLDER) {
				e = f->tree_path!=NULL? gtk_tree_view_row_expanded(GTK_TREE_VIEW(djynn_pm->project_list),f->tree_path) : FALSE;
				fprintf(fp,"%c[%d%d%d%s]",
								e? '+' : '-',
								!!(f->status&DJYNN_PM_ADD_FILES),
								!!(f->status&DJYNN_PM_SUBFOLDERS),
								!!(f->status&DJYNN_PM_DYNAMIC),
								f->pattern!=NULL? f->pattern : "");
//debug_output("write_project(%c%s)\n",e? '+' : '-',f->name);
			}
			
			if(f->path==NULL || *f->path=='\0') s = f->name;
			else if(strncmp(f->path,p->file.path,n)==0 && f->path[n]==G_DIR_SEPARATOR) s = &f->path[n+1];
			else s = f->path;
			if(f->type==DJYNN_PROJECT_FOLDER && f->name!=NULL && strcmp(s,f->name)!=0) fprintf(fp,"\"%s\":%s\n",f->name,s);
			else fprintf(fp,"%s\n",s);
//debug_output("write_project(%s)\n",f->path);
			if(f->files!=NULL && !(f->status&DJYNN_PM_DYNAMIC)) f = f->files,++indent;
			else if(f->next!=NULL) f = f->next;
			else {
				for(f=f->parent,--indent; f!=NULL && f->next==NULL; f=f->parent,--indent);
				if(f==NULL/* || indent==0*/) break;
				f = f->next;
				if(f->type==DJYNN_PROJECT) break;
			}
		}
		fclose(fp);
	} else perror(p->project_filename);
}

void djynn_workspace_expand() {
	DjynnProjectFile *f1 = (DjynnProjectFile *)djynn_pm->projects,*f2;
	if(djynn_pm->project_list==NULL) return;
//debug_output("djynn_workspace_expand(1)\n");
	while(f1!=NULL) {
//debug_output("djynn_workspace_expand(name=%s)\n",f1->name);
		if((f1->status&DJYNN_PM_EXPANDED) && f1->tree_path!=NULL)
			gtk_tree_view_expand_row(GTK_TREE_VIEW(djynn_pm->project_list),f1->tree_path,FALSE);

		if(f1->files!=NULL) f1 = f1->files;
		else if(f1->next!=NULL) f1 = f1->next;
		else if(f1->parent!=NULL) {
			for(f2=f1->parent; f2->next==NULL && f2->parent!=NULL; f2=f2->parent);
			f1 = f2->next;
		} else f1 = NULL;
	}
//debug_output("djynn_workspace_expand(done)\n");
}

static void free_project(DjynnProject *f) {
	if(f->file.path!=NULL) g_free(f->file.path);
	if(f->file.name!=NULL) g_free(f->file.name);
	g_free(f);
}

void djynn_workspace_read() {
	gint i,n,len,index;
	gchar key[64];
	gchar *value,*d = NULL,*d1,*d2,c;
	FILE *fp;
	DjynnProject *p1 = NULL,*p2 = NULL;

//debug_output("djynn_workspace_read(workspace=%s,key=%s)\n",djynn->workspace,djynn->workspace_key);
	q_config_open(djynn->config_filename);

	if(djynn_pm->projects!=NULL) djynn_project_clear();

	n = q_config_get_int(djynn->workspace_key,djynn_key->project_n,0);
//debug_output("project_n=%d\n",n);
	if(n>0) {
		for(i=1,index=1; i<=n; ++i) {
			if(d!=NULL) g_free(d);
			if(p2!=NULL) free_project(p2);

			sprintf(key,djynn_key->project_d,i);
			value = q_config_get_str(djynn->workspace_key,key,NULL);
//debug_output("%s=%s\n",key,value);
			if(!(fp=fopen(value,"r"))) {
				perror(value);
				continue;
			}
			fseek(fp,0,SEEK_END);
			len = ftell(fp);
			if(len==0) {
				fclose(fp);
				continue;
			}
			fseek(fp,0,SEEK_SET);
			d = (gchar *)g_malloc(len+1);
			for(d1=d; (c=fgetc(fp))!=EOF; *d1++=c);
			*d1 = '\0';
			fclose(fp);

			p2 = (DjynnProject *)g_malloc(sizeof(DjynnProject));
			memset(p2,0,sizeof(DjynnProject));
			p2->file.parent = NULL;
			p2->file.files = NULL;
			p2->file.next = NULL;
			p2->file.tree_path = NULL;
			d1 = d,d2 = strchr(d1,'\n');
			if(d2==NULL) continue;
			*d2++ = '\0';
			p2->file.status = 0;
			if(*d1=='+') p2->file.status |= DJYNN_PM_EXPANDED,++d1;
			else if(*d1=='-') ++d1;
			p2->file.name = g_strdup(d1);
			d1 = d2,d2 = strchr(d1,'\n');
			if(d2==NULL) continue;
			*d2++ = '\0';
			p2->file.path = g_strdup(d1);
			p2->file.type = DJYNN_PROJECT;
			p2->index = index;
			p2->project_filename = value;
			d1 = d2,d2 = strchr(d1,'\n');
			if(d2==NULL) continue;
			*d2++ = '\0';
			p2->geany_project_filename = g_strdup(d1);
//debug_output("name=%s, path=%s, index=%d\n",p2->name,p2->path,p2->index);

			parse_project(p2,d2);
			if(djynn_pm->projects==NULL) djynn_pm->projects = (DjynnProjectFile *)p2,p1 = p2;
			else p1->file.next = (DjynnProjectFile *)p2,p1 = p2;
			p2 = NULL;
			++index;
		}
		if(d!=NULL) g_free(d);
		if(p2!=NULL) free_project(p2);
	}
	q_config_close();
//debug_output("djynn_workspace_read(done)\n");
}

static void workspace_set(gint index) {
	gint n;
	q_config_open(djynn->config_filename);
	n = q_config_get_int(djynn_key->workspace,djynn_key->workspace_n,0);
	if(index>=1 && index<=n) {
		n = q_config_get_int(djynn_key->djynn,djynn_key->workspace,1);
		if(index!=n) {
			gchar key[257],*p1,*p2;
			djynn_workspace_save();
			n = q_config_get_int(djynn_key->workspace,djynn_key->workspace_n,0);
			q_config_set_int(djynn_key->djynn,djynn_key->workspace,index);
			sprintf(key,djynn_key->workspace_d,index);
//debug_output("workspace_set(n=%d,key=%s)\n",index,key);
			g_free(djynn->workspace);
			g_free(djynn->workspace_key);
			p1 = q_config_get_str(djynn_key->workspace,key,NULL);
			p2 = strchr(p1,':'),*p2 = '\0',++p2;
			sprintf(key,"%s_%s",djynn_key->workspace,p1);
			djynn->workspace = g_strdup(p2);
			djynn->workspace_key = g_strdup(key);
			g_free(p1);
//debug_output("workspace_set(workspace=%s)\n",djynn->workspace);
			djynn_workspace_load();
		}
	}
	q_config_close();
}

void djynn_workspace_select(gint index) {
	gtk_combo_box_set_active(GTK_COMBO_BOX(djynn_pm->workspace_list),index-1);
}

void djynn_workspace_remove(gint index) {
	GtkTreeIter iter;
	if(gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(djynn_pm->workspace_store),&iter,NULL,index-1))
		gtk_list_store_remove(djynn_pm->workspace_store,&iter);
}

void djynn_workspace_add(const gchar *name) {
	GtkTreeIter iter;
	gtk_list_store_append(djynn_pm->workspace_store,&iter);
	gtk_list_store_set(djynn_pm->workspace_store,&iter,WORKSPACE_LIST_ICON,djynn_pm->icons[DJYNN_WORKSPACE],WORKSPACE_LIST_WORKSPACE,name,-1);
}

void djynn_workspace_set(gint index,const gchar *name) {
	GtkTreeIter iter;
	if(gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(djynn_pm->workspace_store),&iter,NULL,index-1))
		gtk_list_store_set(djynn_pm->workspace_store,&iter,WORKSPACE_LIST_ICON,djynn_pm->icons[DJYNN_WORKSPACE],WORKSPACE_LIST_WORKSPACE,name,-1);
}

void djynn_workspace_save() {
	DjynnProjectFile *f = djynn_pm->projects;
	for(; f!=NULL; f=f->next) write_project((DjynnProject *)f);
}

void djynn_workspace_load() {
	djynn_workspace_read();
	djynn_project_update();
	djynn_workspace_expand();
}

void djynn_workspace_reload() {
	djynn_workspace_save();
	djynn_workspace_load();
}

void djynn_workspace_delete() {
	gint index = 1,n;
	gchar *title = _("Delete Workspace");
	gint r = 0;
	r = djynn_msgbox_ask(title,_("Are you sure you want to delete the entire workspace \"%s\" and all its projects?"),djynn->workspace);
	if(r==GTK_RESPONSE_OK) {
		q_config_open(djynn->config_filename);
		n = q_config_get_int(djynn_key->workspace,djynn_key->workspace_n,0);
		if(n==1) djynn_msgbox_warn(title,_("There must be at least one workspace!"),NULL);
		else {
			gchar key[257];
			strcpy(key,djynn->workspace_key);
//debug_output("djynn_workspace_delete(index=%d,workspace=%s)\n",index,djynn->workspace);
			index = q_config_get_int(djynn_key->djynn,djynn_key->workspace,1);
			djynn_workspace_select(index>1? index-1 : index+1);
			q_config_remove_group(key);
			q_config_remove_from_list(djynn_key->workspace,djynn_key->workspace,index);
			djynn_workspace_remove(index);
		}
		q_config_close();
	}
}

void djynn_workspace_index_projects() {
	DjynnProjectFile *f = djynn_pm->projects;
	DjynnProject *prj;
	gint i;
	gchar key[64];
	q_config_open(djynn->config_filename);
	for(i=1; f!=NULL; ++i,f=f->next) {
		prj = (DjynnProject *)f;
		prj->index = i;
		sprintf(key,djynn_key->project_d,prj->index);
		q_config_set_str(djynn->workspace_key,key,prj->project_filename);
	}
	q_config_close();
}

static void workspace_changed_cb(GtkComboBox *combo,gpointer data) {
	workspace_set(gtk_combo_box_get_active(combo)+1);
}

void djynn_workspace_init(GeanyData *data) {
	GtkCellRenderer *icon_renderer;
	GtkCellRenderer *text_renderer;
	gint i,n;
	gchar key[64],*value;

	q_menu_create(GTK_MENU_SHELL(djynn_pm->project_menu),workspace_menu_items,Q_MENU_INSERT_BEFORE,0);

	djynn_pm->workspace_store = gtk_list_store_new(WORKSPACE_LIST_COLS,GDK_TYPE_PIXBUF,G_TYPE_STRING);
	djynn_pm->workspace_list = gtk_combo_box_new_with_model(GTK_TREE_MODEL(djynn_pm->workspace_store));
	n = q_config_get_int(djynn_key->workspace,djynn_key->workspace_n,0);
	for(i=1; i<=n; ++i) {
		sprintf(key,djynn_key->workspace_d,i);
		value = q_config_get_str(djynn_key->workspace,key,NULL);
		djynn_workspace_add(strchr(value,':')+1);
		g_free(value);
	}
	g_object_unref(djynn_pm->workspace_store);
	icon_renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(djynn_pm->workspace_list),icon_renderer,FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(djynn_pm->workspace_list),icon_renderer,"pixbuf",WORKSPACE_LIST_ICON,NULL);
	text_renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(djynn_pm->workspace_list),text_renderer,TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(djynn_pm->workspace_list),text_renderer,"text",WORKSPACE_LIST_WORKSPACE,NULL);
	n = q_config_get_int(djynn_key->djynn,djynn_key->workspace,1);
	gtk_combo_box_set_active(GTK_COMBO_BOX(djynn_pm->workspace_list),n-1);
	g_signal_connect(djynn_pm->workspace_list,"changed",G_CALLBACK(workspace_changed_cb),NULL);
	gtk_widget_set_tooltip_text(djynn_pm->workspace_list,_("Switch between workspaces."));

	djynn_workspace_read();
}

void djynn_workspace_cleanup() {
	djynn_workspace_save();
}

void djynn_workspace_action(gint id,gboolean check) {
	if(!djynn_cfg->project_manager) return;
	switch(id) {
		case WORKSPACE_NEW:
			djynn_workspace_dialog(TRUE);
			break;
		case WORKSPACE_RENAME:
			djynn_workspace_dialog(FALSE);
			break;
		case WORKSPACE_RELOAD:
			djynn_workspace_load();
			break;
		case WORKSPACE_DELETE:
			djynn_workspace_delete();
			break;
	}
}

gint djynn_workspace_keybindings() {
	return 0;
}


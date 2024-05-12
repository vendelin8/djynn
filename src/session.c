

#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>
#include <libq/string.h>
#include <libq/glib/config.h>
#include <libq/gtk/menu.h>
#include "djynn.h"
#include "project.h"
#include "session.h"
#include "dialog.h"



enum {
	SESSION_LIST_ICON,
	SESSION_LIST_SESSION,
	SESSION_LIST_COLS,
};

static void menu_activate(QMenuItem *menuitem) {
	if(menuitem!=NULL) {
		gboolean check = FALSE;
		if(menuitem->type==Q_MENU_CHECKBOX || menuitem->type==Q_MENU_RADIO)
			check = q_menu_is_checked(menuitem->id);
		djynn_session_action(menuitem->id,check);
	}
}

enum {
	SESSION_NEW = 0x2201,
	SESSION_SAVE,
	SESSION_RENAME,
	SESSION_RELOAD,
	SESSION_DELETE,
};

static QMenuItem session_menu_items[] = {
/*   type          id                   label & tooltip               icon               submenu   activate */
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, SESSION_NEW,         N_("New Session..."),NULL,    GTK_STOCK_NEW,     NULL,     menu_activate },
	{ Q_MENU_LABEL, SESSION_SAVE,        N_("Save Session"),NULL,      GTK_STOCK_SAVE,    NULL,     menu_activate },
	{ Q_MENU_LABEL, SESSION_RENAME,      N_("Rename Session..."),NULL, GTK_STOCK_SAVE_AS, NULL,     menu_activate },
	{ Q_MENU_LABEL, SESSION_RELOAD,      N_("Reload Session"),NULL,    GTK_STOCK_REFRESH, NULL,     menu_activate },
	{ Q_MENU_LABEL, SESSION_DELETE,      N_("Delete Session"),NULL,    GTK_STOCK_DELETE,  NULL,     menu_activate },
	{ 0,0 }
};


/*static void session_write_from_geany_cfg() {
	gint i,n;
	gchar key[64],*s,*p,*fn;
	gchar **arr;
	gsize len;
	FILE *fp;
	GKeyFile *conf = g_key_file_new();;

	fn = g_strconcat(geany->app->configdir,G_DIR_SEPARATOR_S,"geany.conf",NULL);
	g_key_file_load_from_file(conf,fn,G_KEY_FILE_NONE,NULL);
	g_free(fn);

	q_config_open(djynn->config_filename);
	n = q_config_get_int(djynn_key->djynn,djynn_key->session);
	sprintf(key,djynn_key->session_d,n);
	s = q_config_get_str(djynn_key->session,key);
	p = strchr(s,':'),*p = '\0',++p;
	fn = g_strconcat(geany->app->configdir,G_DIR_SEPARATOR_S,"plugins",G_DIR_SEPARATOR_S,
			djynn_key->djynn,G_DIR_SEPARATOR_S,"session",s,".txt",NULL);
debug_output("session_write_from_geany_cfg(s=%s,p=%s,fn=%s)\n",s,p,fn);
	g_free(s);
	fp = fopen(fn,"w");

	if(fp!=NULL) {
		gchar str[1025];
		gint c = g_key_file_get_integer(conf,"files","current_page",NULL);
		for(n=0; 1; ++n) {
			sprintf(key,"FILE_NAME_%d",n);
			arr = g_key_file_get_string_list(conf,"files",key,&len,NULL);
			if(arr==NULL) break;
			else {
				for(i=0,p=arr[7]; *p!='\0'; ++i,++p)
					if(*p=='%') str[i] = (xtoi(p[1])<<4)|xtoi(p[2]),p += 2;
					else str[i] = *p;
				str[i] = '\0';
				fprintf(fp,"%s;%d;%s\n",arr[0],n==c,str);
debug_output("session_write_from_geany_cfg(file=%s)\n",str);
			}
		}
		fclose(fp);
	} else perror(fn);
	g_free(fn);
	q_config_close();

	g_key_file_free(conf);
}*/

static void session_write() {
	gint n;
	gchar key[64],*s,*p,*fn;
	FILE *fp;
	q_config_open(djynn->config_filename);
	n = q_config_get_int(djynn_key->djynn,djynn_key->session,1);
	sprintf(key,djynn_key->session_d,n);
	s = q_config_get_str(djynn_key->session,key,NULL);
	p = strchr(s,':'),*p = '\0',++p;
	fn = g_strconcat(geany->app->configdir,G_DIR_SEPARATOR_S,"plugins",G_DIR_SEPARATOR_S,
			djynn_key->djynn,G_DIR_SEPARATOR_S,"session",s,".txt",NULL);
//debug_output("session_write(s=%s,p=%s,fn=%s)\n",s,p,fn);
	g_free(s);
	fp = fopen(fn,"w");
	if(fp!=NULL) {
		GeanyDocument *doc = document_get_current();
		if(doc!=NULL) {
			gint c = doc->index;
			for(n=0; 1; ++n) {
				doc = document_get_from_page(n);
				if(doc==NULL) break;
				if(doc->file_name!=NULL && g_path_is_absolute(doc->file_name)) {
					fprintf(fp,"%d;%d;%s\n",sci_get_current_position(doc->editor->sci),doc->index==c,doc->file_name);
//debug_output("session_write(file=%s)\n",doc->file_name);
				}
			}
		}
		fclose(fp);
	} else perror(fn);
	g_free(fn);
	q_config_close();
}

void session_open(const gchar *fn) {
	FILE *fp = fopen(fn,"r");
	if(fp!=NULL) {
		gint pos,sel;
		gchar str[1024],*p1,*p2,open[1024] = "";
		GeanyDocument *doc;
// TODO		document_close_all();
		while((doc=document_get_current())!=NULL)
			document_close(doc);
		while(!feof(fp)) {
			p1 = fgets(str,1024,fp);
			if(p1==NULL) break;
			p2 = strchr(p1,';'),*p2 = '\0',++p2;
			pos = atoi(p1),p1 = p2;
			p2 = strchr(p1,';'),*p2 = '\0',++p2;
			sel = atoi(p1),p1 = p2;
			if((p2=strchr(p1,'\n'))!=NULL) *p2 = '\0';
			if((p2=strchr(p1,'\r'))!=NULL) *p2 = '\0';
//debug_output("session_open(pos=%d,sel=%d,path=%s)\n",pos,sel,p1);
			if(g_file_test(p1,G_FILE_TEST_EXISTS)) {
				doc = document_open_file(p1,FALSE,NULL,NULL);
				sci_set_current_position(doc->editor->sci,pos,TRUE);
				if(sel) strcpy(open,p1);
			} else djynn_msgbox_warn("Open Session",_("The file \"%s\" no longer exists."),p1);
		}
		fclose(fp);
		if(*open!='\0') document_open_file(open,FALSE,NULL,NULL);
	} else perror(fn);
}

void djynn_session_save() {
	session_write();
}

void djynn_session_load() {
	gint n;
	gchar key[64],*s,*p,*fn;
	q_config_open(djynn->config_filename);
	n = q_config_get_int(djynn_key->djynn,djynn_key->session,1);
//debug_output("djynn_session_load(n=%d)\n",n);
	sprintf(key,djynn_key->session_d,n);
	s = q_config_get_str(djynn_key->session,key,NULL);
	p = strchr(s,':'),*p = '\0',++p;
	fn = g_strconcat(geany->app->configdir,G_DIR_SEPARATOR_S,"plugins",G_DIR_SEPARATOR_S,
			djynn_key->djynn,G_DIR_SEPARATOR_S,"session",s,".txt",NULL);
//debug_output("djynn_session_set(s=%s,p=%s,fn=%s)\n",s,p,fn);
	g_free(s);
	if(g_file_test(fn,G_FILE_TEST_EXISTS)) session_open(fn);
	g_free(fn);
	q_config_close();
}

void djynn_session_set(gint index) {
	gint n;
//	gchar key[64],*s,*p,*fn;
	q_config_open(djynn->config_filename);
	n = q_config_get_int(djynn_key->session,djynn_key->session_n,0);
	if(index>=1 && index<=n) {
		n = q_config_get_int(djynn_key->djynn,djynn_key->session,1);
		if(index!=n) {
//debug_output("djynn_session_set(n=%d)\n",index);
			q_config_set_int(djynn_key->djynn,djynn_key->session,index);
			djynn_session_load();
/*			sprintf(key,djynn_key->session_d,index);
			s = q_config_get_str(djynn_key->session,key);
			p = strchr(s,':'),*p = '\0',++p;
			fn = g_strconcat(geany->app->configdir,G_DIR_SEPARATOR_S,"plugins",G_DIR_SEPARATOR_S,
					djynn_key->djynn,G_DIR_SEPARATOR_S,"session",s,".txt",NULL);
debug_output("djynn_session_set(s=%s,p=%s,fn=%s)\n",s,p,fn);
			g_free(s);
			session_open(fn);
			g_free(fn);*/
		}
	}
	q_config_close();
}

void djynn_session_list_select(gint index) {
	gtk_combo_box_set_active(GTK_COMBO_BOX(djynn_pm->session_list),index-1);
}

void djynn_session_list_remove(gint index) {
	GtkTreeIter iter;
	if(gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(djynn_pm->session_store),&iter,NULL,index-1))
		gtk_list_store_remove(djynn_pm->session_store,&iter);
}

void djynn_session_list_add(const gchar *name) {
	GtkTreeIter iter;
	gtk_list_store_append(djynn_pm->session_store,&iter);
	gtk_list_store_set(djynn_pm->session_store,&iter,SESSION_LIST_ICON,djynn_pm->icons[DJYNN_SESSION],SESSION_LIST_SESSION,name,-1);
}

void djynn_session_list_set(gint index,const gchar *name) {
	GtkTreeIter iter;
	if(gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(djynn_pm->session_store),&iter,NULL,index-1))
		gtk_list_store_set(djynn_pm->session_store,&iter,SESSION_LIST_ICON,djynn_pm->icons[DJYNN_SESSION],SESSION_LIST_SESSION,name,-1);
}

void djynn_session_delete() {
	gint index = 1,n;
	gchar *title = _("Delete Session");
	gint r = 0;
	gchar key[64],str[257],*p1,*p2,*fn;

	q_config_open(djynn->config_filename);
	n = q_config_get_int(djynn_key->djynn,djynn_key->session,1);
	sprintf(key,djynn_key->session_d,n);
	p1 = q_config_get_str(djynn_key->session,key,NULL);
	p2 = strchr(p1,':'),*p2 = '\0',++p2;
	strcpy(key,p1);
	strcpy(str,p2);
	g_free(p1);

	r = djynn_msgbox_ask(title,_("Are you sure you want to delete the session \"%s\"?"),str);
	if(r==GTK_RESPONSE_OK) {
		n = q_config_get_int(djynn_key->session,djynn_key->session_n,0);
		if(n==1) djynn_msgbox_warn(title,_("There must be at least one session!"),NULL);
		else {
//debug_output("djynn_session_delete(index=%d,workspace=%s)\n",index,djynn->workspace);
			index = q_config_get_int(djynn_key->djynn,djynn_key->session,1);
			djynn_session_list_select(index>1? index-1 : index+1);
			q_config_remove_from_list(djynn_key->session,djynn_key->session,index);
			djynn_session_list_remove(index);

			fn = g_strconcat(geany->app->configdir,G_DIR_SEPARATOR_S,"plugins",G_DIR_SEPARATOR_S,
					djynn_key->djynn,G_DIR_SEPARATOR_S,"session",key,".txt",NULL);
			g_remove(fn);
			g_free(fn);
		}
	}
	q_config_close();
}

static void session_list_changed_cb(GtkComboBox *combo,gpointer data) {
	session_write();
	djynn_session_set(gtk_combo_box_get_active(combo)+1);
}


void djynn_session_init(GeanyData *data) {
	GtkCellRenderer *icon_renderer;
	GtkCellRenderer *text_renderer;
	gint i,n;
	gchar key[64],*value;

	q_menu_create(GTK_MENU_SHELL(djynn_pm->project_menu),session_menu_items,Q_MENU_INSERT_BEFORE,0);

	djynn_pm->session_store = gtk_list_store_new(SESSION_LIST_COLS,GDK_TYPE_PIXBUF,G_TYPE_STRING);
	djynn_pm->session_list = gtk_combo_box_new_with_model(GTK_TREE_MODEL(djynn_pm->session_store));
	n = q_config_get_int(djynn_key->session,djynn_key->session_n,0);
	for(i=1; i<=n; ++i) {
		sprintf(key,djynn_key->session_d,i);
		value = q_config_get_str(djynn_key->session,key,NULL);
		djynn_session_list_add(strchr(value,':')+1);
		g_free(value);
	}
	g_object_unref(djynn_pm->session_store);
	icon_renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(djynn_pm->session_list),icon_renderer,FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(djynn_pm->session_list),icon_renderer,"pixbuf",SESSION_LIST_ICON,NULL);
	text_renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(djynn_pm->session_list),text_renderer,TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(djynn_pm->session_list),text_renderer,"text",SESSION_LIST_SESSION,NULL);
	n = q_config_get_int(djynn_key->djynn,djynn_key->session,1);
	gtk_combo_box_set_active(GTK_COMBO_BOX(djynn_pm->session_list),n-1);
	g_signal_connect(djynn_pm->session_list,"changed",G_CALLBACK(session_list_changed_cb),NULL);
	gtk_widget_set_tooltip_text(djynn_pm->session_list,_("Switch between sessions."));
}

void djynn_session_cleanup() {
}

void djynn_session_action(gint id,gboolean check) {
	if(!djynn_cfg->project_manager) return;
	switch(id) {
		case SESSION_NEW:
			djynn_session_dialog(TRUE);
			break;
		case SESSION_RENAME:
			djynn_session_dialog(FALSE);
			break;
		case SESSION_SAVE:
			djynn_session_save();
			break;
		case SESSION_RELOAD:
			djynn_session_load();
			break;
		case SESSION_DELETE:
			djynn_session_delete();
			break;
	}
}

gint djynn_session_keybindings() {
	return 0;
}



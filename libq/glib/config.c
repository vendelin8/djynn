
#include "../libq.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "config.h"


typedef struct _QConfigFile QConfigFile;

struct _QConfigFile {
	gchar *filename;
	GKeyFile *keyfile;
	gint ref;
	gboolean changed;
	QConfigFile *prev;
	QConfigFile *next;
};

static QConfigFile *config_first = NULL;
static QConfigFile *config_last = NULL;
static QConfigFile *config = NULL;


void q_config_open(const gchar *filename) {
	if(filename==NULL) return;
	if(config==NULL || strcmp(config->filename,filename)!=0) {
		for(config=config_last; config!=NULL; config=config->next)
			if(strcmp(config->filename,filename)==0) break;
		if(config==NULL) {
			config = (QConfigFile *)g_new0(QConfigFile,1);
			if(config_first==NULL) config_first = config;
			if(config_last!=NULL) config_last->prev = config;
			config->next = config_last;
			config_last = config;

			config->filename = g_strdup(filename);
			config->keyfile = g_key_file_new();
			if(g_file_test(config->filename,G_FILE_TEST_EXISTS))
				g_key_file_load_from_file(config->keyfile,config->filename,G_KEY_FILE_NONE,NULL);
			config->ref = 0;
			config->changed = FALSE;
		}
	}
	++config->ref;
//fprintf(stderr,"q_config_open(%s [ref: %d])\n",config->filename,config->ref);
}

void q_config_save() {
	if(config==NULL) return;
//fprintf(stderr,"q_config_save(%s)\n",config->filename);
	if(config->keyfile!=NULL) {
		gchar *data;
		gsize size;
		gboolean ret;
		data = g_key_file_to_data(config->keyfile,&size,NULL);
//fprintf(djynn->log,"q_config_save(\n%s\n)\n",data);
		ret = g_file_set_contents(config->filename,data,size,NULL);
		g_free(data);
		(void)ret;
		config->changed = FALSE;
	}
}

void q_config_close() {
	if(config==NULL) return;
	--config->ref;
//fprintf(stderr,"q_config_close(%s [ref: %d])\n",config->filename,config->ref);
	if(config->ref<=0) {
		
		if(config->keyfile!=NULL) {
			if(config->changed)
				q_config_save();
			g_key_file_free(config->keyfile);
			config->keyfile = NULL;
		}
		if(config==config_first) config_first = config->prev;
		if(config==config_last) config_last = config->next;
		if(config->prev!=NULL) config->prev->next = config->next;
		if(config->next!=NULL) config->next->prev = config->prev;
		if(config->filename!=NULL) {
			g_free(config->filename);
			config->filename = NULL;
		}
		g_free(config);
		config = config_last;
	}
}

gboolean q_config_has_group(const gchar *group) {
	if(config==NULL || config->keyfile==NULL) return FALSE;
	return g_key_file_has_group(config->keyfile,group);
}

gboolean q_config_has_key(const gchar *group,const gchar *key) {
	if(config==NULL || config->keyfile==NULL) return FALSE;
	return g_key_file_has_key(config->keyfile,group,key,NULL);
}

void q_config_remove_group(const gchar *group) {
	if(config==NULL || config->keyfile==NULL) return;
//fprintf(djynn->log,"q_config_remove_group(group=%s)\n",group);
//fflush(djynn->log);
	g_key_file_remove_group(config->keyfile,group,NULL);
	config->changed = TRUE;
}

void q_config_remove(const gchar *group,const gchar *key) {
	if(config==NULL || config->keyfile==NULL) return;
	g_key_file_remove_key(config->keyfile,group,key,NULL);
	config->changed = TRUE;
}

void q_config_remove_from_list(const gchar *group,const gchar *key,gint index) {
	gint i,n;
	gchar key_n[64];
	if(config==NULL || config->keyfile==NULL) return;
	sprintf(key_n,"%s_n",key);
	n = q_config_get_int(group,key_n,0);
	if(index>=1 && index<=n) {
		gchar key_d[64];
		gchar k[64],*s;
		sprintf(key_d,"%s_%%d",key);
		for(i=index; i<n; ++i) {
			sprintf(k,key_d,i+1);
			s = q_config_get_str(group,k,NULL);
			sprintf(k,key_d,i);
			q_config_set_str(group,k,s);
			g_free(s);
		}
		sprintf(k,key_d,n);
		q_config_remove(group,k);
		q_config_set_int(group,key_n,n-1);
		config->changed = TRUE;
	}
}

void q_config_remove_comment(const gchar *group,const gchar *key) {
	if(config==NULL || config->keyfile==NULL) return;
	g_key_file_remove_comment(config->keyfile,group,key,NULL);
	config->changed = TRUE;
}

gchar *q_config_get_comment(const gchar *group,const gchar *key) {
	if(config==NULL || config->keyfile==NULL) return NULL;
	return g_key_file_get_comment(config->keyfile,group,key,NULL);
}

void q_config_set_comment(const gchar *group,const gchar *key,const gchar *comment) {
	if(config==NULL || config->keyfile==NULL) return;
	g_key_file_set_comment(config->keyfile,group,key,comment,NULL);
	config->changed = TRUE;
}

gint q_config_get_int(const gchar *group,const gchar *key,gint def) {
	GError *error = NULL;
	if(config==NULL || config->keyfile==NULL) return def;
	gint n = g_key_file_get_integer(config->keyfile,group,key,&error);
	if(error!=NULL) {
		g_error_free(error);
		q_config_set_int(group,key,def);
		return def;
	}
	return n;
}

void q_config_set_int(const gchar *group,const gchar *key,gint n) {
	if(config==NULL || config->keyfile==NULL) return;
	g_key_file_set_integer(config->keyfile,group,key,n);
	config->changed = TRUE;
}

gchar *q_config_get_str(const gchar *group,const gchar *key,const gchar *def) {
	gchar *str = NULL;
	if(config!=NULL && config->keyfile!=NULL)
		str = g_key_file_get_string(config->keyfile,group,key,NULL);
	if(str==NULL && def!=NULL) {
		if(config!=NULL && config->keyfile!=NULL)
			q_config_set_str(group,key,def);
		str = g_strdup(def);
	}
	return str;
}

void q_config_set_str(const gchar *group,const gchar *key,const gchar *str) {
	if(config==NULL || config->keyfile==NULL) return;
	g_key_file_set_string(config->keyfile,group,key,str);
	config->changed = TRUE;
}




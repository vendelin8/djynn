#ifndef _DJYNN_WORKSPACE_H_
#define _DJYNN_WORKSPACE_H_

/**
 * @file workspace.h
 * @author Per Löwgren
 * @date Modified: 2015-08-31
 * @date Created: 2014-05-22
 */ 

#include "djynn.h"


void djynn_workspace_init(GeanyData *data);
void djynn_workspace_cleanup();
void djynn_workspace_action(gint id,gboolean check);
gint djynn_workspace_keybindings();

void djynn_workspace_read();
void djynn_workspace_expand();
void djynn_workspace_save();
void djynn_workspace_load();
void djynn_workspace_reload();
void djynn_workspace_select(gint index);
void djynn_workspace_remove(gint index);
void djynn_workspace_add(const gchar *name);
void djynn_workspace_set(gint index,const gchar *name);
void djynn_workspace_delete();
void djynn_workspace_index_projects();


#endif /* _DJYNN_WORKSPACE_H_ */



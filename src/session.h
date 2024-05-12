#ifndef _DJYNN_SESSION_H_
#define _DJYNN_SESSION_H_

/**
 * @file session.h
 * @author Per Löwgren
 * @date Modified: 2015-08-31
 * @date Created: 2014-05-22
 */ 

#include "djynn.h"


void djynn_session_init(GeanyData *data);
void djynn_session_cleanup();
void djynn_session_action(gint id,gboolean check);
gint djynn_session_keybindings();

void djynn_session_set(gint index);
void djynn_session_save();
void djynn_session_load();
void djynn_session_list_select(gint index);
void djynn_session_list_remove(gint index);
void djynn_session_list_add(const gchar *name);
void djynn_session_list_set(gint index,const gchar *name);
void djynn_session_delete();


#endif /* _DJYNN_SESSION_H_ */



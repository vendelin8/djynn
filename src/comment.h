#ifndef _DJYNN_COMMENT_H_
#define _DJYNN_COMMENT_H_

/**
 * @file comment.h
 * @author Per Löwgren
 * @date Modified: 2015-08-31
 * @date Created: 2014-05-22
 */ 

#include "djynn.h"

void djynn_comment_init(GeanyData *data);
void djynn_comment_create();
void djynn_comment_cleanup();
void djynn_comment_action(gint id,gboolean check);
gint djynn_comment_keybindings();
void djynn_comment_enable();
void djynn_comment_disable();
void djynn_comment_update_menu(gboolean show);


#endif /* _DJYNN_H_ */



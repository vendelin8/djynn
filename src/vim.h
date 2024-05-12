#ifndef _DJYNN_VIM_H_
#define _DJYNN_VIM_H_

/**
 * @file vim.h
 * @author Per Löwgren
 * @date Modified: 2016-12-03
 * @date Created: 2016-12-03
 */ 

#include "djynn.h"

void djynn_vim_init(GeanyData *data);
void djynn_vim_create();
void djynn_vim_cleanup();
void djynn_vim_action(gint id,gboolean check);
gint djynn_vim_keybindings();
void djynn_vim_enable();
void djynn_vim_disable();
void djynn_vim_keyboard();
void djynn_vim_connect();


#endif /* _DJYNN_VIM_H_ */


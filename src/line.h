#ifndef _DJYNN_LINE_H_
#define _DJYNN_LINE_H_

/**
 * @file line.h
 * @author Per Löwgren
 * @date Modified: 2015-08-31
 * @date Created: 2014-05-22
 */ 

#include "djynn.h"

void djynn_line_init(GeanyData *data);
void djynn_line_create();
void djynn_line_cleanup();
void djynn_line_action(gint id,gboolean check);
gint djynn_line_keybindings();
void djynn_line_enable();
void djynn_line_disable();


#endif /* _DJYNN_LINE_H_ */



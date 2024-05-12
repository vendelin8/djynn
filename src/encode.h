#ifndef _DJYNN_ENCODE_H_
#define _DJYNN_ENCODE_H_

/**
 * @file encode.h
 * @author Per Löwgren
 * @date Modified: 2015-08-31
 * @date Created: 2014-05-22
 */ 

#include "djynn.h"

void djynn_encode_init(GeanyData *data);
void djynn_encode_create();
void djynn_encode_cleanup();
void djynn_encode_action(gint id,gboolean check);
gint djynn_encode_keybindings();
void djynn_encode_enable();
void djynn_encode_disable();


#endif /* _DJYNN_ENCODE_H_ */



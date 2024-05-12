#ifndef _LIBQ_GLIB_CONFIG_H_
#define _LIBQ_GLIB_CONFIG_H_

/**
 * @file libq/config.h
 * @author Per Löwgren
 * @date Modified: 2015-08-05
 * @date Created: 2014-05-22
 */ 

#include <glib.h>


void q_config_open(const gchar *filename);
void q_config_save();
void q_config_close();
gboolean q_config_has_group(const gchar *group);
void q_config_remove_group(const gchar *group);
void q_config_remove(const gchar *group,const gchar *key);
void q_config_remove_from_list(const gchar *group,const gchar *key,gint index);
void q_config_remove_comment(const gchar *group,const gchar *key);
gchar *q_config_get_comment(const gchar *group,const gchar *key);
void q_config_set_comment(const gchar *group,const gchar *key,const gchar *comment);
gint q_config_get_int(const gchar *group,const gchar *key,gint def);
void q_config_set_int(const gchar *group,const gchar *key,gint n);
gchar *q_config_get_str(const gchar *group,const gchar *key,const gchar *def);
void q_config_set_str(const gchar *group,const gchar *key,const gchar *str);



#endif /* _LIBQ_GLIB_CONFIG_H_ */



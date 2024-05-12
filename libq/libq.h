#ifndef _LIBQ_LIBQ_H_
#define _LIBQ_LIBQ_H_

/**
 * @file libq/libq.h
 * @author Per Löwgren
 * @date Modified: 2015-08-04
 * @date Created: 2015-08-04
 */ 


enum {
	__LIBQ_ARRAY__ = 1,
	__LIBQ_BASE58__,
	__LIBQ_BASE64__,
	__LIBQ_BYTES__,
	__LIBQ_BF__,
	__LIBQ_COIN__,
	__LIBQ_DECIMAL__,
	__LIBQ_ERROR__,
	__LIBQ_JSON__,
	__LIBQ_LZW__,
	__LIBQ_RANDOM__,
	__LIBQ_RLE__,
	__LIBQ_STRING__,
	__LIBQ_TREE__,
	__LIBQ_TYPE__,
};

#include <libq/config.h>

#ifdef USE_GLIB
#include <glib.h>
#define q_malloc g_malloc
#define q_realloc g_realloc
#define q_strdup g_strdup
#define q_free g_free
#else
#define q_malloc malloc
#define q_realloc realloc
#define q_strdup strdup
#define q_free free
#endif


#endif /* _LIBQ_LIBQ_H_ */



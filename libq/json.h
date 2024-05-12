#ifndef _LIBQ_JSON_H_
#define _LIBQ_JSON_H_

/**
 * @file libq/json.h  
 * @author Per Löwgren
 * @date Modified: 2015-07-25
 * @date Created: 2014-05-27
 */ 


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "libq.h"
#include "array.h"
#include "string.h"

enum {
	ERR_JSON = (__LIBQ_JSON__<<16),
	ERR_JSON_DECODE,
	ERR_JSON_ENCODE,
	ERR_JSON_GET
};


enum {
	JSON_UNDEFINED       = ARR_UNDEFINED,
	JSON_NULL            = ARR_NULL,
	JSON_BOOLEAN         = ARR_BOOLEAN,
	JSON_INTEGER         = ARR_INTEGER,
	JSON_DOUBLE          = ARR_DECIMAL,
	JSON_STRING          = ARR_STRING,
	JSON_ARRAY           = ARR_ARRAY,
	JSON_OBJECT          = 0x11,
	JSON_FUNCTION        = 0x12,
};

enum {
	JSON_PRETTY				= 0x0001,
	JSON_ESCAPED			= 0x0002,
	JSON_SQL					= 0x0004,
};

/*
#define q_json_free             q_js_free
#define q_json_decode           q_js_decode
#define q_json_encode           q_js_encode
#define q_json_encode_string    q_js_encodes
#define q_json_encode_array     q_js_encodea
#define q_json_get              q_js_get
#define q_json_read             q_js_read
#define q_json_write            q_js_write
#define q_json_print            q_js_print
*/

typedef struct _QJson *QJson;


void q_json_free(QJson json);
QJson q_json_decode(const char *json);
char *q_json_encode(const QJson json,int style);
char *q_json_encode_string(const char *str,int style);
char *q_json_encode_array(QArray arr,int style);

/** Get the json-value of the given key.
 * The key should be the name of the field in an object, or numerical index
 * (starting with zero) in an array which should start with a hash-sign '#'.
 * In nested arrays, separate keys with a colon ':'.
 * Example: key "array:#2" for json {"array":["abc","def","ghi"]} will return "ghi".
 * When the json is neither object nor array, the content is always returned
 * and key ignored.
 * @param json JSON-container
 * @param key Key to look for in the json (can be NULL).
 * @return Value of given key if found or 0. */
QType q_json_get(QJson json,const char *key);

QJson q_json_read(const char *fn);
void q_json_write(const char *fn,const QJson json,int style);

void q_json_print(FILE *fp,const QJson json);


#endif /* _LIBQ_JSON_H_ */


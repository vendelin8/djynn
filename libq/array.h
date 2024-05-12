#ifndef _LIBQ_ARRAY_H_
#define _LIBQ_ARRAY_H_

/**
 * @file libq/array.h  
 * @author Per Löwgren
 * @date Modified: 2015-07-25
 * @date Created: 2014-05-30
 */ 

#include <stdio.h>
#include <stdint.h>
#include "libq.h"
#include "decimal.h"
#include "type.h"

enum {
	ERR_ARRAY = (__LIBQ_ARRAY__<<16),
};


enum {
	ARR_UNDEFINED                       = 0x0000,   //!< 
	ARR_NULL                            = 0x0001,   //!< 
	ARR_BOOLEAN                         = 0x0002,   //!< 
	ARR_INTEGER                         = 0x0003,   //!< 
	ARR_FLOAT                           = 0x0004,   //!< 
	ARR_DECIMAL                         = 0x0005,   //!< 
	ARR_STRING                          = 0x0006,   //!< 
	ARR_ARRAY                           = 0x0007,   //!< 
	ARR_POINTER                         = 0x0008,   //!< 
};

enum {
	ARR_ST_VECTOR                       = 0x0001,   //!< Flag is set if array contains integer keys
	ARR_ST_HASHTABLE                    = 0x0002,   //!< Flag is set if array contains string keys
	ARR_ST_CASE_INSENSITIVE             = 0x0004,   //!< Whether string keys are case insensitive
	ARR_ST_KEY_MULTIPLES                = 0x0008,   //!< Allow multiples of same key (experimental)
	ARR_ST_STRING_POINTER               = 0x0010,   //!< Store strings, both keys and values, as pointers instead of duplicating
	ARR_ST_STRING_COPY                  = 0x0020,   //!< Copy string keys & values (default)
	ARR_ST_STRING_FREE                  = 0x0040,   //!< Free string keys & values when freeing array
	ARR_ST_ARRAY_POINTER                = 0x0080,   //!< Not implemented
	ARR_ST_ARRAY_COPY                   = 0x0100,   //!< Not implemented
	ARR_ST_ARRAY_FREE                   = 0x0200,   //!< Free QArray-values
};

enum {
	ARR_SPLIT_EMPTY_ITEMS               = 0x01,     //!< Split empty items, e.g. "a,,b" becomes ["a","","b"]
	ARR_SPLIT_EMPTY_ITEMS_EXCEPT_FIRST  = 0x03,     //!< Split empty items except first, e.g. ",a,,b" becomes ["a","","b"]
	ARR_SPLIT_EMPTY_ITEMS_EXCEPT_LAST   = 0x05,     //!< Split empty items except last, e.g. "a,,b," becomes ["a","","b"]
	ARR_SPLIT_EMPTY_ITEMS_EXCEPT_ENDS   = 0x07,     //!< Split empty items except ends, e.g. ",a,,b," becomes ["a","","b"]
};

enum {
	ARR_JOIN_PREFIX                     = 0x01,     //!< Prepend delimiter as prefix
	ARR_JOIN_SUFFIX                     = 0x02,     //!< Append delimiter as suffix
};

enum {
	ARR_SORT_INDEX                      = 0x01,     //!< Sort integer keys
	ARR_SORT_KEYS                       = 0x02,     //!< Sort string keys
	ARR_SORT_CASE_INSENSITIVE           = 0x04,     //!< Sort string keys with case insensitivity
	ARR_SORT_KEYS_CASE                  = 0x06,     //!< Same as (ARR_SORT_KEYS | ARR_SORT_KEYS_CASE)
};

/*
#define q_array_new                      q_a_new
#define q_array_free                     q_a_free
#define q_array_dup                      q_a_dup
#define q_array_is_vector                q_a_isvec
#define q_array_is_hashtable             q_a_isht
#define q_array_set                      q_a_set
#define q_array_set_bool                 q_a_setb
#define q_array_set_int                  q_a_seti
#define q_array_set_float                q_a_setf
#define q_array_set_decimal              q_a_setd
#define q_array_set_array                q_a_seta
#define q_array_set_pointer              q_a_setp
#define q_array_push                     q_a_push
#define q_array_push_bool                q_a_pushb
#define q_array_push_int                 q_a_pushi
#define q_array_push_float               q_a_pushf
#define q_array_push_decimal             q_a_pushd
#define q_array_push_array               q_a_pusha
#define q_array_push_pointer             q_a_pushp
#define q_array_put                      q_a_put
#define q_array_put_bool                 q_a_putb
#define q_array_put_int                  q_a_puti
#define q_array_put_float                q_a_putf
#define q_array_put_decimal              q_a_putd
#define q_array_put_array                q_a_puta
#define q_array_put_pointer              q_a_putp
#define q_array_replace                  q_a_repl
#define q_array_replace_bool             q_a_replb
#define q_array_replace_int              q_a_repli
#define q_array_replace_float            q_a_replf
#define q_array_replace_decimal          q_a_repld
#define q_array_replace_array            q_a_repla
#define q_array_replace_pointer          q_a_replp
#define q_array_index                    q_a_ind
#define q_array_get                      q_a_get
#define q_array_fetch                    q_a_fetch
#define q_array_remove_index             q_a_rmi
#define q_array_remove                   q_a_rm
#define q_array_split                    q_a_split
#define q_array_join                     q_a_join
#define q_array_sort                     q_a_sort
#define q_array_reverse                  q_a_reverse
#define q_array_reset                    q_a_reset
#define q_array_min                      q_a_min
#define q_array_max                      q_a_max
#define q_array_first                    q_a_first
#define q_array_last                     q_a_last
#define q_array_previous                 q_a_prev
#define q_array_next                     q_a_next
#define q_array_each                     q_a_each
#define q_array_each_type                q_a_eacht
#define q_array_each_bool(arr,v)         q_a_eacht((arr),ARR_BOOLEAN,(v))
#define q_array_each_int(arr,v)          q_a_eacht((arr),ARR_INTEGER,(v))
#define q_array_each_float(arr,v)        q_a_eacht((arr),ARR_FLOAT,(v))
#define q_array_each_decimal(arr,v)      q_a_eacht((arr),ARR_DECIMAL,(v))
#define q_array_each_string(arr,v)       q_a_eacht((arr),ARR_STRING,(v))
#define q_array_each_array(arr,v)        q_a_eacht((arr),ARR_ARRAY,(v))
#define q_array_each_pointer(arr,v)      q_a_eacht((arr),ARR_POINTER,(v))
#define q_array_each_r                   q_a_eachr
#define q_array_each_r_type              q_a_eachrt
#define q_array_each_r_bool(arr,v)       q_a_eachrt((arr),ARR_BOOLEAN,(v))
#define q_array_each_r_int(arr,v)        q_a_eachrt((arr),ARR_INTEGER,(v))
#define q_array_each_r_float(arr,v)      q_a_eachrt((arr),ARR_FLOAT,(v))
#define q_array_each_r_decimal(arr,v)    q_a_eachrt((arr),ARR_DECIMAL,(v))
#define q_array_each_r_string(arr,v)     q_a_eachrt((arr),ARR_STRING,(v))
#define q_array_each_r_array(arr,v)      q_a_eachrt((arr),ARR_ARRAY,(v))
#define q_array_each_r_pointer(arr,v)    q_a_eachrt((arr),ARR_POINTER,(v))
#define q_array_foreach                  q_a_for
#define q_array_foreach_type             q_a_fort
#define q_array_foreach_bool(arr,f)      q_a_fort((arr),ARR_BOOLEAN,(f))
#define q_array_foreach_int(arr,f)       q_a_fort((arr),ARR_INTEGER,(f))
#define q_array_foreach_float(arr,f)     q_a_fort((arr),ARR_FLOAT,(f))
#define q_array_foreach_decimal(arr,f)   q_a_fort((arr),ARR_DECIMAL,(f))
#define q_array_foreach_string(arr,f)    q_a_fort((arr),ARR_STRING,(f))
#define q_array_foreach_array(arr,f)     q_a_fort((arr),ARR_ARRAY,(f))
#define q_array_foreach_pointer(arr,f)   q_a_fort((arr),ARR_POINTER,(f))
#define q_array_foreach_r                q_a_forr
#define q_array_foreach_r_type           q_a_forrt
#define q_array_foreach_r_bool(arr,f)    q_a_forrt((arr),ARR_BOOLEAN,(f))
#define q_array_foreach_r_int(arr,f)     q_a_forrt((arr),ARR_INTEGER,(f))
#define q_array_foreach_r_float(arr,f)   q_a_forrt((arr),ARR_FLOAT,(f))
#define q_array_foreach_r_decimal(arr,f) q_a_forrt((arr),ARR_DECIMAL,(f))
#define q_array_foreach_r_string(arr,f)  q_a_forrt((arr),ARR_STRING,(f))
#define q_array_foreach_r_array(arr,f)   q_a_forrt((arr),ARR_ARRAY,(f))
#define q_array_foreach_r_pointer(arr,f) q_a_forrt((arr),ARR_POINTER,(f))
#define q_array_key                      q_a_key
#define q_array_next_key                 q_a_next_key
#define q_array_key_is_int               q_a_kisi
#define q_array_key_is_string            q_a_kiss
#define q_array_value                    q_a_val
#define q_array_next_value               q_a_next_val
#define q_array_value_is_null            q_a_visn
#define q_array_value_is_bool            q_a_visb
#define q_array_value_is_int             q_a_visi
#define q_array_value_is_float           q_a_visf
#define q_array_value_is_decimal         q_a_visd
#define q_array_value_is_string          q_a_viss
#define q_array_value_is_array           q_a_visa
#define q_array_value_is_pointer         q_a_visp
#define q_array_print                    q_a_print
#define q_array_print_table              q_a_print_table
*/

typedef struct _QArray *QArray;
typedef struct _QArrayIter *QArrayIter;


/** Create a newQArray object
 * @param c Initial capacity for array
 * @param st Style flags for array, e.g. ARR_ST_-enum values
 * @return A newly allocated QArray object */
QArray q_array_new(size_t c,QStyle st);

/** Free array according to the style flags.
 * By default strings are allocated and freed, but not pointer and array values.
 * @param arr QArray object */
void q_array_free(QArray arr);

/** Duplicate QArray object, according to style flags.
 * Internally allocated keys and values are also duplicated. Order is maintained.
 * @param arr QArray object
 * @return Duplicated QArray object */
QArray q_array_dup(const QArray arr);

/** Size of array, number of values
 * @param arr QArray object
 * @return Number of values */
size_t q_array_size(const QArray arr);

/** Capacity of array
 * @param arr QArray object
 * @return Maximum amount of values before resizing is required */
size_t q_array_capacity(const QArray arr);

/** Does array contain values with integer keys
 * @param arr 
 * @return Boolean */
int q_array_is_vector(const QArray arr);

/** Does array contain values with string keys
 * @param arr 
 * @return Boolean */
int q_array_is_hashtable(const QArray arr);

void q_array_set(QArray arr,int k,const char *v);
void q_array_set_bool(QArray arr,int k,int v);
void q_array_set_int(QArray arr,int k,int v);
void q_array_set_float(QArray arr,int k,double v);
void q_array_set_decimal(QArray arr,int k,QDecimal v);
void q_array_set_array(QArray arr,int k,QArray v);
void q_array_set_pointer(QArray arr,int k,void *v);

void q_array_push(QArray arr,const char *v);
void q_array_push_bool(QArray arr,int v);
void q_array_push_int(QArray arr,int v);
void q_array_push_float(QArray arr,double v);
void q_array_push_decimal(QArray arr,QDecimal v);
void q_array_push_array(QArray arr,QArray v);
void q_array_push_pointer(QArray arr,void *v);

void q_array_put(QArray arr,const char *k,const char *v);
void q_array_put_bool(QArray arr,const char *k,int v);
void q_array_put_int(QArray arr,const char *k,int v);
void q_array_put_float(QArray arr,const char *k,double v);
void q_array_put_decimal(QArray arr,const char *k,QDecimal v);
void q_array_put_array(QArray arr,const char *k,QArray v);
void q_array_put_pointer(QArray arr,const char *k,void *v);

void q_array_replace(QArray arr,const char *v);
void q_array_replace_bool(QArray arr,int v);
void q_array_replace_int(QArray arr,int v);
void q_array_replace_float(QArray arr,double v);
void q_array_replace_decimal(QArray arr,QDecimal v);
void q_array_replace_array(QArray arr,QArray v);
void q_array_replace_pointer(QArray arr,void *v);

QType q_array_index(const QArray arr,int k);
int q_array_index_bool(QArray arr,int k);
int q_array_index_int(QArray arr,int k);
double q_array_index_float(QArray arr,int k);
QDecimal q_array_index_decimal(QArray arr,int k);
QArray q_array_index_array(QArray arr,int k);
void *q_array_index_pointer(QArray arr,int k);

QType q_array_get(const QArray arr,const char *k);
int q_array_get_bool(QArray arr,const char *k);
int q_array_get_int(QArray arr,const char *k);
double q_array_get_float(QArray arr,const char *k);
QDecimal q_array_get_decimal(QArray arr,const char *k);
QArray q_array_get_array(QArray arr,const char *k);
void *q_array_get_pointer(QArray arr,const char *k);

/** Fetch the value of the given key, reaching into sub-arrays.
 * The key should be the name of the field, or numerical index (starting with zero)
 * in an array in which case it should start with a hash-sign '#'. In nested arrays,
 * separate keys with a colon ':'.
 * Example: key "array:#2" for parsed json {"array":["abc","def","ghi"]} will return "ghi".
 * @param arr Array object
 * @param k Key to search for
 * @param t Pointer to type (can be NULL), is set to the type of the returned value (or if not found ARR_UNDEFINED).
 * @return Value of given key if found or 0. */
QType q_array_fetch(const QArray arr,const char *k);
int q_array_fetch_bool(QArray arr,const char *k);
int q_array_fetch_int(QArray arr,const char *k);
double q_array_fetch_float(QArray arr,const char *k);
QDecimal q_array_fetch_decimal(QArray arr,const char *k);
QArray q_array_fetch_array(QArray arr,const char *k);
void *q_array_fetch_pointer(QArray arr,const char *k);

int q_array_remove_index(QArray arr,int k);
int q_array_remove(QArray arr,const char *k);

void q_array_split(QArray arr,const char *str,const char *delim,int st);
char *q_array_join(const QArray arr,const char *delim,int st);

void q_array_sort(QArray arr,int st);
void q_array_reverse(QArray arr);

void q_array_reset(QArray arr);
int q_array_min(QArray arr,QType *v);
int q_array_max(QArray arr,QType *v);
int q_array_first(QArray arr,QType *v);
int q_array_last(QArray arr,QType *v);
int q_array_previous(QArray arr,QType *v);
int q_array_next(QArray arr,QType *v);
int q_array_each(QArray arr,QType *v);
int q_array_each_type(QArray arr,int type,QType *v);
int q_array_each_r(QArray arr,QType *v);
int q_array_each_r_type(QArray arr,int type,QType *v);
void q_array_foreach(QArray arr,void (*f)(QType,void *),void *data);
void q_array_foreach_type(QArray arr,int type,void (*f)(QType,void *),void *data);
void q_array_foreach_r(QArray arr,void (*f)(QType,void *),void *data);
void q_array_foreach_r_type(QArray arr,int type,void (*f)(QType,void *),void *data);

QType q_array_key(const QArray arr);
QType q_array_previous_key(const QArray arr);
QType q_array_next_key(const QArray arr);
int q_array_key_is_int(const QArray arr);
int q_array_key_is_string(const QArray arr);

QType q_array_value(const QArray arr);
QType q_array_previous_value(const QArray arr);
QType q_array_next_value(const QArray arr);
int q_array_value_is_null(const QArray arr);
int q_array_value_is_bool(const QArray arr);
int q_array_value_is_int(const QArray arr);
int q_array_value_is_float(const QArray arr);
int q_array_value_is_decimal(const QArray arr);
int q_array_value_is_string(const QArray arr);
int q_array_value_is_array(const QArray arr);
int q_array_value_is_pointer(const QArray arr);

QArrayIter q_array_get_iter(QArray arr);
void q_array_set_iter(QArray arr,QArrayIter iter);

void q_array_print(FILE *fp,const QArray arr);
void q_array_print_table(FILE *fp,const QArray arr);

#endif /* _LIBQ_ARRAY_H_ */


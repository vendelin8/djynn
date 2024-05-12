
#include "libq.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "array.h"
#include "_array.h"
#include "error.h"
#include "json.h"
#include "string.h"


Q_ERR(__FILE__)


int arr_value_bool(QType t) {
	switch(t.t) {
		case ARR_BOOLEAN:
		case ARR_INTEGER:return t.i!=0;
		case ARR_FLOAT:return t.f!=0.0;
		case ARR_DECIMAL:return t.d!=0;
		case ARR_STRING:return t.s!=NULL && *t.s!='\0';
		case ARR_ARRAY:return t.a!=NULL && t.a->size>0;
		case ARR_POINTER:return t.v!=NULL;
	}
	return 0;
}

int arr_value_int(QType t) {
	switch(t.t) {
		case ARR_BOOLEAN:
		case ARR_INTEGER:return t.i;
		case ARR_FLOAT:return (int)round(t.f);
		case ARR_DECIMAL:return q_dec_toint(t.d);
		case ARR_STRING:return t.s==NULL? 0 : (int)strtol(t.s,NULL,0);
		case ARR_ARRAY:return t.a==NULL? 0 : t.a->size;
	}
	return 0;
}

double arr_value_float(QType t) {
	switch(t.t) {
		case ARR_BOOLEAN:
		case ARR_INTEGER:return (double)t.i;
		case ARR_FLOAT:return t.f;
		case ARR_DECIMAL:return q_dec_tofloat(t.d);
		case ARR_STRING:return t.s==NULL? 0.0 : (double)strtod(t.s,NULL);
	}
	return 0.0;
}

QDecimal arr_value_decimal(QType t) {
	switch(t.t) {
		case ARR_BOOLEAN:
		case ARR_INTEGER:return q_dec_int64((int64_t)t.i);break;
		case ARR_FLOAT:return q_dec_float(t.f);break;
		case ARR_DECIMAL:return t.d;
		case ARR_STRING:return q_dec_str(t.s,NULL);
	}
	return 0;
}

QArray arr_value_array(QType t) {
	if(t.t==ARR_ARRAY) return t.a;
	return NULL;
}

void *arr_value_pointer(QType t) {
	switch(t.t) {
		case ARR_STRING:return (void *)t.s;
		case ARR_ARRAY:return (void *)t.a;
		case ARR_POINTER:return t.v;
	}
	return NULL;
}


/* Resize the table and re-index nodes. */
static void arr_resize(QArray arr,size_t c) {
	size_t i,m;
	QArrayNode n1,n2,*t;
//	owl_var_print(h);
	if(c<=arr->cap) {
		if(c<=0) c = arr->cap*2;
		else c = arr->cap+c;
	}
//fprintf(stderr,"arr_resize(%d=>%d, %d)\n",(int)arr->cap,(int)c,(int)(sizeof(QArrayNode)*c));
//fflush(stderr);
	t = (QArrayNode *)q_malloc(sizeof(QArrayNode)*c);
	if(t==NULL) {
		q_err(ERR_MALLOC,NULL);
		return;
	}
	memset(t,0,sizeof(QArrayNode)*c);
	if(arr->size>0) for(i=0; i<arr->cap; ++i)
		for(n1=arr->table[i]; n1!=NULL; n1=n2) {
			n2 = n1->table,m = n1->hash%c,n1->table = t[m],t[m] = n1;
//fprintf(stderr,"node(\"%s\",index=%d,hash=%lu[m=%d],n=%p)\n",n1->key.t==ARR_STRING? n1->key.s : "int",(int)i,(uint64_t)n1->hash,(int)m,t[m]);
//fflush(stderr);
		}
	arr->cap = c;
	q_free(arr->table);
	arr->table = t;

//q_array_print_table(stderr,arr);
}

/* Generate a list of nodes, that can be used for sorting algorithms etc. */
static QArrayNode *arr_list(QArray arr,QArrayNode list[]) {
	int i;
	QArrayNode n;
	for(i=0,n=arr->first; n!=NULL; ++i,n=n->next) list[i] = n;
	list[i] = NULL;
	return list;
}

/* Re-link nodes to the order of supplied list. */
static void arr_link(QArray arr,QArrayNode list[]) {
	int i;
	QArrayNode n;
	arr->first = list[0];
	arr->last = list[arr->size-1];
	for(i=0; i<arr->size; ++i) {
		n = list[i];
		n->back = i==0? NULL : list[i-1];
		n->next = i+1==arr->size? NULL : list[i+1];
	}
}

/* Find minimum and maximum key-values for nodes with integer keys. */
static void arr_minmax(QArray arr) {
	QArrayNode n;
	arr->min = NULL;
	arr->max = NULL;
	for(n=arr->first; n!=NULL; n=n->next)
		if(n->key.t==ARR_INTEGER) {
			if(arr->min==NULL || n->key.i<arr->min->key.i) arr->min = n;
			if(arr->max==NULL || n->key.i>arr->max->key.i) arr->max = n;
		}
}

QArray q_array_new(size_t c,QStyle st) {
	QArray arr = (QArray)q_malloc(sizeof(_QArray));
	if(arr==NULL) {
		q_err(ERR_MALLOC,NULL);
		return NULL;
	}
	memset(arr,0,sizeof(_QArray));
	if(c<=0) c = 4;
	if(!(st&(ARR_ST_STRING_POINTER|ARR_ST_STRING_COPY))) st |= ARR_ST_STRING_COPY;
	if(st&ARR_ST_STRING_COPY) st |= ARR_ST_STRING_FREE;
	arr->style = st;
	arr->size = 0;
	arr->cap = c;
	arr->min = NULL;
	arr->max = NULL;
	arr->first = NULL;
	arr->last = NULL;
	arr->iter = NULL;
	arr->table = (QArrayNode *)q_malloc(sizeof(QArrayNode)*c);
	if(arr->table==NULL) q_err(ERR_MALLOC,NULL);
	memset(arr->table,0,sizeof(QArrayNode)*c);
	return arr;
}

void arr_free_node(QArray arr,QArrayNode n) {

//if(n->key.t==ARR_STRING) fprintf(stderr,"arr_free_node(key=%s",n->key.s);
//else fprintf(stderr,"arr_free_node(key=%ld",n->key.i);
//fprintf(stderr,",value=%ld,key.t=%d,value.t=%d,style=%x,flags=%x)\n",n->value.i,n->key.t,n->value.t,arr->style,n->flags);

	if(n->key.t==ARR_STRING && n->key.s &&
			((arr->style&ARR_ST_STRING_FREE) ||
				(n->flags&NODE_FREE_KEY))) /*{*/q_free(n->key.s);/*fprintf(stderr,"free: string key\n");}*/
	if(n->value.t==ARR_STRING && n->value.s &&
			((arr->style&ARR_ST_STRING_FREE) ||
				(n->flags&NODE_FREE_VALUE))) /*{*/q_free(n->value.s);/*fprintf(stderr,"free: string value\n");}*/
	else if(n->value.t==ARR_ARRAY && n->value.a &&
			((arr->style&ARR_ST_ARRAY_FREE) ||
				(n->flags&NODE_FREE_VALUE))) q_array_free(n->value.a);
	q_free(n);
}

void q_array_free(QArray arr) {
	size_t i;
	QArrayNode n1,n2;
	for(i=0; i<arr->cap; i++)
		for(n1=arr->table[i]; n1!=NULL; n1=n2) {
			n2 = n1->table;
			arr_free_node(arr,n1);
		}
	q_free(arr->table);
	q_free(arr);
}

QArray q_array_dup(const QArray arr) {
	if(arr->size==0) return q_array_new(0,arr->style);
	else {
		size_t i,j = 0,m;
		QArray arr2;
		QArrayNode l1[arr->size+1],l2[arr->size+1],n1,n2;
		arr2 = q_array_new(arr->cap,arr->style);
		if(arr2==NULL) goto array_dup_err_malloc;
		arr2->size = arr->size;
		arr_list(arr,l1);
		for(i=0; i<arr->size; ++i) {
			n1 = l1[i];
			n2 = (QArrayNode)q_malloc(sizeof(_QArrayNode));
			if(n2==NULL) goto array_dup_err_malloc;
			if(n1==arr->iter) arr2->iter = n1;

			n2->hash = n1->hash;
			n2->flags = 0;
			l2[j++] = n2;
			if(n1->key.t==ARR_STRING && (arr->style&ARR_ST_STRING_COPY)) {
				n2->key = (QType){ t:ARR_STRING, s:q_strdup(n1->key.s) };
				if(n2->key.s==NULL) goto array_dup_err_malloc;
				n2->flags |= NODE_FREE_KEY;
			} else n2->key = n1->key;
			if(n1->value.t==ARR_STRING && (arr->style&ARR_ST_STRING_COPY)) {
				n2->value = (QType){ t:ARR_STRING, s:q_strdup(n1->value.s) };
				if(n2->value.s==NULL) goto array_dup_err_malloc;
				n2->flags |= NODE_FREE_VALUE;
			} else n2->value = n1->value;

			m = n2->hash%arr2->cap;
			n2->table = arr2->table[m];
			arr2->table[m] = n2;
		}

		if(0) {
array_dup_err_malloc:
			q_err(ERR_MALLOC,NULL);
			for(; j>0; --j) arr_free_node(arr,l2[j-1]);
			return NULL;
		}

		arr_link(arr2,l2);
		arr_minmax(arr2);
		return arr2;
	}
}

size_t q_array_size(const QArray arr) { return arr!=NULL? arr->size : 0; }
size_t q_array_capacity(const QArray arr) { return arr!=NULL? arr->cap : 0; }

int q_array_is_vector(const QArray arr) { return !!(arr->style&ARR_ST_VECTOR); }
int q_array_is_hashtable(const QArray arr) { return !!(arr->style&ARR_ST_HASHTABLE); }

/* Set value at index. */
static void arr_set(QArray arr,int k,QType v) {
	size_t m = ((QHash)k)%arr->cap;
//fprintf(pm_out,"q_arr_put(%s,%d[%d])\n",k,hash,m);
//fflush(pm_out);
	QArrayNode n;

	for(n=arr->table[m]; n!=NULL; n=n->table)
		if(n->key.t==ARR_INTEGER && n->key.i==(long)k) break;

	if(n!=NULL) {
		if(n->value.t==ARR_STRING && n->value.s &&
				((arr->style&ARR_ST_STRING_FREE) ||
					(n->flags&NODE_FREE_VALUE))) q_free(n->value.s);
		else if(n->value.t==ARR_ARRAY && n->value.a &&
				((arr->style&ARR_ST_ARRAY_FREE) ||
					(n->flags&NODE_FREE_VALUE))) q_array_free(n->value.a);
		if(n->flags&NODE_FREE_VALUE) n->flags ^= NODE_FREE_VALUE;
	} else {
		if(arr->size>=arr->cap/2) {
			arr_resize(arr,0);
			m = ((QHash)k)%arr->cap;
		}
		n = (QArrayNode)q_malloc(sizeof(_QArrayNode));
		if(n==NULL) q_err(ERR_MALLOC,NULL);

		n->back = arr->last;
		if(n->back!=NULL) n->back->next = n;
		n->next = NULL;
		if(arr->first==NULL) arr->first = n;
		arr->last = n;

		if(arr->min==NULL || (long)k<arr->min->key.i) arr->min = n;
		if(arr->max==NULL || (long)k>arr->max->key.i) arr->max = n;

		n->table = arr->table[m];
		n->hash = (QHash)k;
		n->key = (QType){ t:ARR_INTEGER, i:k };
		arr->table[m] = n;
		arr->size++;
//fprintf(pm_out,"#");
	}
	if((v.t==ARR_STRING || v.t==ARR_ARRAY || v.t==ARR_POINTER) && v.v==NULL) v.t = ARR_NULL;
	if(v.t==ARR_STRING && (arr->style&ARR_ST_STRING_COPY)) {
		n->value = (QType){ t:ARR_STRING, s:q_strdup(v.s) };
		if(n->value.s==NULL) q_err(ERR_MALLOC,NULL);
		n->flags |= NODE_FREE_VALUE;
	} else {
		n->value = v;
		if(v.t==ARR_ARRAY)
//			v.a->style =
//				(v.a->style&(ARR_ST_VECTOR|ARR_ST_HASHTABLE))|
//				(arr->style&(ARR_ST_CASE_INSENSITIVE|ARR_ST_KEY_MULTIPLES|ARR_ST_ARRAY_FREE));
			v.a->style =
				(v.a->style&(ARR_ST_VECTOR|ARR_ST_HASHTABLE))|
				(arr->style&~(ARR_ST_VECTOR|ARR_ST_HASHTABLE));
	}

	if((arr->style&ARR_ST_VECTOR)==0) arr->style |= ARR_ST_VECTOR;
}

void q_array_set(QArray arr,int k,const char *v) { arr_set(arr,k,(QType){ t:ARR_STRING, s:(char *)v }); }
void q_array_set_bool(QArray arr,int k,int v) { arr_set(arr,k,(QType){ t:ARR_BOOLEAN, i:(long)v }); }
void q_array_set_int(QArray arr,int k,int v) { arr_set(arr,k,(QType){ t:ARR_INTEGER, i:(long)v }); }
void q_array_set_float(QArray arr,int k,double v) { arr_set(arr,k,(QType){ t:ARR_FLOAT, f:v }); }
void q_array_set_decimal(QArray arr,int k,QDecimal v) { arr_set(arr,k,(QType){ t:ARR_DECIMAL, d:v }); }
void q_array_set_array(QArray arr,int k,QArray v) { arr_set(arr,k,(QType){ t:ARR_ARRAY, a:v }); }
void q_array_set_pointer(QArray arr,int k,void *v) { arr_set(arr,k,(QType){ t:ARR_POINTER, v:v }); }

/* Push value to end of array. */
static void arr_push(QArray arr,QType v) {
	if(arr!=NULL) {
		int i = 0;
		if(arr->size>0 && arr->max!=NULL) i = (int)arr->max->key.i+1;
		arr_set(arr,i,v);
	}
}

void q_array_push(QArray arr,const char *v) { arr_push(arr,(QType){ t:ARR_STRING, s:(char *)v }); }
void q_array_push_bool(QArray arr,int v) { arr_push(arr,(QType){ t:ARR_BOOLEAN, i:(long)v }); }
void q_array_push_int(QArray arr,int v) { arr_push(arr,(QType){ t:ARR_INTEGER, i:(long)v }); }
void q_array_push_float(QArray arr,double v) { arr_push(arr,(QType){ t:ARR_FLOAT, f:v }); }
void q_array_push_decimal(QArray arr,QDecimal v) { arr_push(arr,(QType){ t:ARR_DECIMAL, d:v }); }
void q_array_push_array(QArray arr,QArray v) { arr_push(arr,(QType){ t:ARR_ARRAY, a:v }); }
void q_array_push_pointer(QArray arr,void *v) { arr_push(arr,(QType){ t:ARR_POINTER, v:v }); }

/* Put value with string key. */
static void arr_put(QArray arr,const char *k,QType v) {
	QHash hash = q_crc32(k,!!(arr->style&ARR_ST_CASE_INSENSITIVE));
	size_t m = hash%arr->cap;
	QArrayNode n;

//fprintf(stderr,"q_arr_put(k=%s,hash=%lu[m=%lu])\n",k,(uint64_t)hash,(uint64_t)m);
//fflush(stderr);
	for(n=arr->table[m]; n!=NULL; n=n->table)
		if(n->key.t==ARR_STRING && n->hash==hash &&
			((arr->style&ARR_ST_CASE_INSENSITIVE)==0?
				strcmp(n->key.s,k) :
				q_stricmp(n->key.s,k))==0) break;

	if(n!=NULL) {
		if(n->value.t==ARR_STRING && n->value.s!=NULL &&
				((arr->style&ARR_ST_STRING_FREE) ||
					(n->flags&NODE_FREE_VALUE))) q_free(n->value.s);
		else if(n->value.t==ARR_ARRAY && n->value.a!=NULL &&
				((arr->style&ARR_ST_ARRAY_FREE) ||
					(n->flags&NODE_FREE_VALUE))) q_array_free(n->value.a);
		if(n->flags&NODE_FREE_VALUE) n->flags ^= NODE_FREE_VALUE;
	} else {
		if(arr->size>=arr->cap/2) {
			arr_resize(arr,0);
			m = hash%arr->cap;
		}
		n = (QArrayNode)q_malloc(sizeof(_QArrayNode));
		if(n==NULL) q_err(ERR_MALLOC,NULL);

		n->back = arr->last;
		if(n->back!=NULL) n->back->next = n;
		n->next = NULL;
		if(arr->first==NULL) arr->first = n;
		arr->last = n;

		n->table = arr->table[m];
		n->hash = hash;
		n->flags = 0;
		if(arr->style&ARR_ST_STRING_COPY) {
			n->key.s = q_strdup(k);
			if(n->key.s==NULL) q_err(ERR_MALLOC,NULL);
			n->flags |= NODE_FREE_KEY;
		} else n->key.s = (char *)k;
		n->key.t = ARR_STRING;
		arr->table[m] = n;
		arr->size++;
//fprintf(stderr,"#\n");
//fflush(stderr);
	}
	if((v.t==ARR_STRING || v.t==ARR_ARRAY || v.t==ARR_POINTER) && v.v==NULL) v.t = ARR_NULL;
	if(v.t==ARR_STRING && (arr->style&ARR_ST_STRING_COPY)) {
		n->value = (QType){ t:ARR_STRING, s:q_strdup(v.s) };
		if(n->value.s==NULL) q_err(ERR_MALLOC,NULL);
		n->flags |= NODE_FREE_VALUE;
	} else {
		n->value = v;
		if(v.t==ARR_ARRAY)
			v.a->style =
				(v.a->style&(ARR_ST_VECTOR|ARR_ST_HASHTABLE))|
				(arr->style&~(ARR_ST_VECTOR|ARR_ST_HASHTABLE));
	}

//fprintf(stderr,"q_arr_put(key=%s,value=%ld,key.t=%d,value.t=%d,style=%x,flags=%x)\n",
//n->key.s,n->value.i,n->key.t,n->value.t,arr->style,n->flags);

	if((arr->style&ARR_ST_HASHTABLE)==0) arr->style |= ARR_ST_HASHTABLE;
}

void q_array_put(QArray arr,const char *k,const char *v) { arr_put(arr,k,(QType){ t:ARR_STRING, s:(char *)v }); }
void q_array_put_bool(QArray arr,const char *k,int v) { arr_put(arr,k,(QType){ t:ARR_BOOLEAN, i:(long)v }); }
void q_array_put_int(QArray arr,const char *k,int v) { arr_put(arr,k,(QType){ t:ARR_INTEGER, i:v }); }
void q_array_put_float(QArray arr,const char *k,double v) { arr_put(arr,k,(QType){ t:ARR_FLOAT, f:v }); }
void q_array_put_decimal(QArray arr,const char *k,QDecimal v) { arr_put(arr,k,(QType){ t:ARR_DECIMAL, d:v }); }
void q_array_put_array(QArray arr,const char *k,QArray v) { arr_put(arr,k,(QType){ t:ARR_ARRAY, a:v }); }
void q_array_put_pointer(QArray arr,const char *k,void *v) { arr_put(arr,k,(QType){ t:ARR_POINTER, v:v }); }

/* Replace value at index. */
static void arr_replace(QArray arr,QType v) {
//fprintf(pm_out,"arr_replace(%s,%d[%d])\n",k,hash,m);
//fflush(pm_out);
	if(arr->iter!=NULL) {
		QArrayNode n = arr->iter;
		short f = n->flags;

		if(n->value.t==v.t && (
			((v.t==ARR_STRING || v.t==ARR_ARRAY || v.t==ARR_POINTER) && n->value.v==v.v) ||
			(v.t==ARR_INTEGER && n->value.i==v.i) ||
			(v.t==ARR_FLOAT && n->value.f==v.f) || (v.t==ARR_DECIMAL && n->value.d==v.d)
		)) return;

		if((v.t==ARR_STRING || v.t==ARR_ARRAY || v.t==ARR_POINTER) && v.v==NULL) v.t = ARR_NULL;
		if(v.t==ARR_STRING && (arr->style&ARR_ST_STRING_COPY)) {
			v = (QType){ t:ARR_STRING, s:q_strdup(v.s) };
			if(v.s==NULL) q_err(ERR_MALLOC,NULL);
			f |= NODE_FREE_VALUE;
		} else {
			if(v.t==ARR_ARRAY)
//				v.a->style =
//					(v.a->style&(ARR_ST_VECTOR|ARR_ST_HASHTABLE))|
//					(arr->style&(ARR_ST_CASE_INSENSITIVE|ARR_ST_KEY_MULTIPLES|ARR_ST_ARRAY_FREE));
				v.a->style =
					(v.a->style&(ARR_ST_VECTOR|ARR_ST_HASHTABLE))|
					(arr->style&~(ARR_ST_VECTOR|ARR_ST_HASHTABLE));
		}

		if(n->value.t==ARR_STRING && n->value.s &&
				((arr->style&ARR_ST_STRING_FREE) ||
					(n->flags&NODE_FREE_VALUE))) q_free(n->value.s);
		else if(n->value.t==ARR_ARRAY && n->value.a &&
				((arr->style&ARR_ST_ARRAY_FREE) ||
					(n->flags&NODE_FREE_VALUE))) q_array_free(n->value.a);
		if(n->flags&NODE_FREE_VALUE) n->flags ^= NODE_FREE_VALUE;

		n->value = v;
		n->flags = f;
	}
}

void q_array_replace(QArray arr,const char *v) { arr_replace(arr,(QType){ t:ARR_STRING, s:(char *)v }); }
void q_array_replace_bool(QArray arr,int v) { arr_replace(arr,(QType){ t:ARR_BOOLEAN, i:(long)v }); }
void q_array_replace_int(QArray arr,int v) { arr_replace(arr,(QType){ t:ARR_INTEGER, i:(long)v }); }
void q_array_replace_float(QArray arr,double v) { arr_replace(arr,(QType){ t:ARR_FLOAT, f:v }); }
void q_array_replace_decimal(QArray arr,QDecimal v) { arr_replace(arr,(QType){ t:ARR_DECIMAL, d:v }); }
void q_array_replace_array(QArray arr,QArray v) { arr_replace(arr,(QType){ t:ARR_ARRAY, a:v }); }
void q_array_replace_pointer(QArray arr,void *v) { arr_replace(arr,(QType){ t:ARR_POINTER, v:v }); }

QType q_array_index(const QArray arr,int k) {
	if(arr!=NULL && arr->table!=NULL && arr->size>0) {
		size_t m = ((QHash)k)%arr->cap;
		QArrayNode n;

		for(n=arr->table[m]; n!=NULL; n=n->table)
			if(n->key.t==ARR_INTEGER && n->key.i==(long)k) break;

		arr->iter = n;
		if(n!=NULL) return n->value;
	}
	return (QType){ t:ARR_UNDEFINED, i:0 };
}

int q_array_index_bool(QArray arr,int k) { return arr_value_bool(q_array_index(arr,k)); }
int q_array_index_int(QArray arr,int k) { return arr_value_int(q_array_index(arr,k)); }
double q_array_index_float(QArray arr,int k) { return arr_value_float(q_array_index(arr,k)); }
QDecimal q_array_index_decimal(QArray arr,int k) { return arr_value_decimal(q_array_index(arr,k)); }
QArray q_array_index_array(QArray arr,int k) { return arr_value_array(q_array_index(arr,k)); }
void *q_array_index_pointer(QArray arr,int k) { return arr_value_pointer(q_array_index(arr,k)); }


QType q_array_get(const QArray arr,const char *k) {
	if(arr!=NULL && arr->table!=NULL && arr->size>0) {
		QHash hash = q_crc32(k,!!(arr->style&ARR_ST_CASE_INSENSITIVE));
		size_t m = hash%arr->cap;
		QArrayNode n;

//fprintf(stderr,"q_array_get(key=%s,hash=%lu[m=%lu])\n",k,(uint64_t)hash,(uint64_t)m);
//fflush(stderr);
		for(n=arr->table[m]; n!=NULL; n=n->table)
			if(n->key.t==ARR_STRING && n->hash==hash &&
				((arr->style&ARR_ST_CASE_INSENSITIVE)==0?
					strcmp(n->key.s,k) : q_stricmp(n->key.s,k))==0) break;

//fprintf(stderr,"q_array_get(n=%p)\n",n);
//fflush(stderr);
		arr->iter = n;
		if(n!=NULL) return n->value;
//q_array_print_table(stderr,arr);
	}
	return (QType){ t:ARR_UNDEFINED, i:0 };
}

int q_array_get_bool(QArray arr,const char *k) { return arr_value_bool(q_array_get(arr,k)); }
int q_array_get_int(QArray arr,const char *k) { return arr_value_int(q_array_get(arr,k)); }
double q_array_get_float(QArray arr,const char *k) { return arr_value_float(q_array_get(arr,k)); }
QDecimal q_array_get_decimal(QArray arr,const char *k) { return arr_value_decimal(q_array_get(arr,k)); }
QArray q_array_get_array(QArray arr,const char *k) { return arr_value_array(q_array_get(arr,k)); }
void *q_array_get_pointer(QArray arr,const char *k) { return arr_value_pointer(q_array_get(arr,k)); }

QType q_array_fetch(const QArray arr,const char *k) {
	if(k!=NULL && *k!='\0') {
		int n = q_strnchr(k,':');
		if(n==0) { // If k doesn't contain any colon:
			if(*k=='#') return q_array_index(arr,atoi(&k[1]));
			else return q_array_get(arr,k);
		} else {
			QArray a = (QArray)arr;
			char s[strlen(k)+1],*p,*l[n+2];
			QType v1 = { t:ARR_UNDEFINED, i:0 };
			strcpy(s,k);
//fprintf(stderr,"q_json_get(n=%d, s=%s)\n",n,s);
			// Split s into an array of keys:
			for(p=s,l[n=0]=p; (p=strchr(p,':'))!=NULL; *p++='\0',l[++n]=p);
			l[++n] = NULL;
//fprintf(stderr,"get_array:");
			for(p=l[n=0]; p!=NULL; p=l[++n]) {
				if(*p=='#') v1 = q_array_index(a,atoi(&p[1]));
				else v1 = q_array_get(a,p);
//fprintf(stderr," %s[%p,%d]",p,(void *)v1,t1);
				if(v1.t==ARR_UNDEFINED) break;
				if(l[n+1]==NULL) { // If no more keys in array:
//fprintf(stderr," [found: %p]\n",(void *)v1);
					return v1;
				}
				if(v1.t==ARR_ARRAY) a = v1.a;
				else break;
			}
		}
	}
//fprintf(stderr," [not found]\n");
	return (QType){ t:ARR_UNDEFINED, i:0 };
}

int q_array_fetch_bool(QArray arr,const char *k) { return arr_value_bool(q_array_fetch(arr,k)); }
int q_array_fetch_int(QArray arr,const char *k) { return arr_value_int(q_array_fetch(arr,k)); }
double q_array_fetch_float(QArray arr,const char *k) { return arr_value_float(q_array_fetch(arr,k)); }
QDecimal q_array_fetch_decimal(QArray arr,const char *k) { return arr_value_decimal(q_array_fetch(arr,k)); }
QArray q_array_fetch_array(QArray arr,const char *k) { return arr_value_array(q_array_fetch(arr,k)); }
void *q_array_fetch_pointer(QArray arr,const char *k) { return arr_value_pointer(q_array_fetch(arr,k)); }


void arr_remove_node(QArray arr,QArrayNode n) {
	size_t m = n->hash%arr->cap;
	if(n==arr->table[m]) arr->table[m] = n->table;
	else {
		QArrayNode n1;
		for(n1=arr->table[m]; n1!=NULL; n1=n1->table)
			if(n1->table==n) {
				n1->table = n->table;
				break;
			}
	}
	if(n->back!=NULL) n->back->next = n->next;
	if(n->next!=NULL) n->next->back = n->back;
	if(n==arr->first) arr->first = n->next;
	if(n==arr->last) arr->last = n->back;
	--arr->size;
	if(arr->size==0) {
		arr->min = NULL;
		arr->max = NULL;
		arr->first = NULL;
		arr->last = NULL;
	} else if(arr->min==n || arr->max==n)
		arr_minmax(arr);
	if(arr->iter==n) arr->iter = n->back;
}

int q_array_remove_index(QArray arr,int k) {
	int r = 0;
	if(arr!=NULL && arr->size>0) {
		size_t m = ((QHash)k)%arr->cap;
		QArrayNode n;
		for(n=arr->table[m]; n!=NULL; n=n->table) {
			if(n->key.t==ARR_INTEGER && n->key.i==(long)k) {
				arr_remove_node(arr,n);
				arr_free_node(arr,n);
				++r;
				break;
			}
		}
	}
	return r;
}

int q_array_remove(QArray arr,const char *k) {
	int r = 0;
	if(arr!=NULL && arr->size>0) {
		QArrayNode n;
		if(k==NULL) {
			if(arr->iter!=NULL) {
				n = arr->iter;
				arr_remove_node(arr,n);
				arr_free_node(arr,n);
				++r;
			}
		} else {
			QHash hash = q_crc32(k,!!(arr->style&ARR_ST_CASE_INSENSITIVE));
			size_t m = hash%arr->cap;
			for(n=arr->table[m]; n!=NULL; n=n->table)
				if(n->key.t==ARR_STRING && n->hash==hash &&
					((arr->style&ARR_ST_CASE_INSENSITIVE)==0?
						strcmp(n->key.s,k) : q_stricmp(n->key.s,k))==0) {
					arr_remove_node(arr,n);
					arr_free_node(arr,n);
					++r;
					break;
				}
		}
	}
	return r;
}

void q_array_split(QArray arr,const char *str,const char *delim,int st) {
	if(arr!=NULL && str!=NULL) {
		char *s = q_strdup(str),*p1,*p2;
		if(s==NULL) q_err(ERR_MALLOC,NULL);
		else {
			p1 = s;
			if(delim!=NULL && *delim!='\0') {
				int dlen = strlen(delim);
				while(*p1!='\0') {
					p2 = strstr(p1,delim);
					if(p2==NULL) break;
					*p2 = '\0',p2 += dlen;
					if(*p1!='\0' ||
						((st&ARR_SPLIT_EMPTY_ITEMS) &&
							(arr->size>0 || (st&ARR_SPLIT_EMPTY_ITEMS_EXCEPT_FIRST)!=ARR_SPLIT_EMPTY_ITEMS_EXCEPT_FIRST))
					) q_array_push(arr,p1);
					p1 = p2;
				}
			}
			if(*p1!='\0' ||
				((st&ARR_SPLIT_EMPTY_ITEMS) &&
					(st&ARR_SPLIT_EMPTY_ITEMS_EXCEPT_LAST)!=ARR_SPLIT_EMPTY_ITEMS_EXCEPT_LAST)
			) q_array_push(arr,p1);
			q_free(s);
		}
	}
}

char *q_array_join(const QArray arr,const char *delim,int st) {
	char *s = NULL;
	if(arr!=NULL && arr->size>0) {
		QString str = q_string_new();
		QArrayNode n;
		if(arr->first!=NULL && (st&ARR_JOIN_PREFIX)) q_string_append(str,delim);
		for(n=arr->first; n!=NULL; n=n->next) {
			if(n!=arr->first) q_string_append(str,delim);
			if(n->value.t==ARR_INTEGER) q_string_append_int(str,(int)n->value.i);
			else if(n->value.t==ARR_STRING) q_string_append(str,n->value.s);
			else continue;
		}
		if(arr->first!=NULL && (st&ARR_JOIN_SUFFIX)) q_string_append(str,delim);
		s = str->ptr;
		q_free(str);
	}
	return s;
}

/* Compare nodes with integer keys. Nodes with string keys are not sorted and placed after nodes with integer keys. */
static int arr_cmpind(const void *a,const void *b) {
	QArrayNode n1 = *(QArrayNode *)a,n2 = *(QArrayNode *)b;
	if(n1->key.t!=n2->key.t) return n1->key.t>n2->key.t? 1 : -1;
	if(n1->key.t==ARR_INTEGER) return (int)(n1->key.i-n2->key.i);
	return 0;
}

/* Compare nodes with string keys. Nodes with integer keys are not sorted and placed before nodes with string keys. */
static int arr_cmpkey(const void *a,const void *b) {
	QArrayNode n1 = *(QArrayNode *)a,n2 = *(QArrayNode *)b;
	if(n1->key.t!=n2->key.t) return n1->key.t>n2->key.t? 1 : -1;
	if(n1->key.t==ARR_STRING) {
		char *s1 = n1->key.s,*s2 = n2->key.s;
		if(*s1=='\0' || *s2=='\0') return *s1=='\0'? (*s2=='\0'? 0 : 1) : -1;
		if(*s1>='a' && *s1<='z' && *s2>='A' && *s2<='Z') return -1;
		if(*s1>='A' && *s1<='Z' && *s2>='a' && *s2<='z') return 1;
		if(*s1!=*s2) return (int)(*s1-*s2);
		return strcmp(s1,s2);
	}
	return 0;
}

/* Compare nodes with string keys. Nodes with integer keys are not sorted and placed before nodes with string keys. Case insensitive string comparison. */
static int arr_icmpkey(const void *a,const void *b) {
	QArrayNode n1 = *(QArrayNode *)a,n2 = *(QArrayNode *)b;
	if(n1->key.t!=n2->key.t) return n1->key.t>n2->key.t? 1 : -1;
	if(n1->key.t==ARR_STRING) {
		char *s1 = n1->key.s,*s2 = n2->key.s;
		if(*s1=='\0' || *s2=='\0') return *s1=='\0'? (*s2=='\0'? 0 : 1) : -1;
		return q_stricmp(s1,s2);
	}
	return 0;
}

/* Compare nodes so that both integer and string keys are sorted, integer before string. */
static int arr_compare(const void *a,const void *b) {
	QArrayNode n1 = *(QArrayNode *)a,n2 = *(QArrayNode *)b;
	if(n1->value.t!=n2->value.t) return n1->value.t>n2->value.t? 1 : -1;
	if(n1->value.t==ARR_INTEGER) return (int)(n1->value.i-n2->value.i);
	if(n1->value.t==ARR_STRING) {
		char *s1 = n1->value.s,*s2 = n2->value.s;
		if(*s1=='\0' || *s2=='\0') return *s1=='\0'? (*s2=='\0'? 0 : 1) : -1;
		if(*s1>='a' && *s1<='z' && *s2>='A' && *s2<='Z') return -1;
		if(*s1>='A' && *s1<='Z' && *s2>='a' && *s2<='z') return 1;
		if(*s1!=*s2) return (int)(*s1-*s2);
		return strcmp(s1,s2);
	}
	return 0;
}

/* Compare nodes so that both integer and string keys are sorted, integer before string. Case insensitive string comparison. */
static int arr_icompare(const void *a,const void *b) {
	QArrayNode n1 = *(QArrayNode *)a,n2 = *(QArrayNode *)b;
	if(n1->value.t!=n2->value.t) return n1->value.t>n2->value.t? 1 : -1;
	if(n1->value.t==ARR_INTEGER) return (int)(n1->value.i-n2->value.i);
	if(n1->value.t==ARR_STRING) {
		char *s1 = n1->value.s,*s2 = n2->value.s;
		if(*s1=='\0' || *s2=='\0') return *s1=='\0'? (*s2=='\0'? 0 : 1) : -1;
		return q_stricmp(s1,s2);
	}
	return 0;
}

void q_array_sort(QArray arr,int st) {
	if(arr!=NULL && arr->size>1) {
		QArrayNode list[arr->size+1];
		int all = ARR_SORT_INDEX|ARR_SORT_KEYS;
		int (*cmp)(const void *,const void *);
		arr_list(arr,list);
		if((st&all)==all || (st&all)==0)
			cmp = (st&ARR_SORT_CASE_INSENSITIVE)? arr_icompare : arr_compare;
		else if(st&ARR_SORT_INDEX)
			cmp = arr_cmpind;
		else if(st&ARR_SORT_KEYS)
			cmp = (st&ARR_SORT_CASE_INSENSITIVE)? arr_icmpkey : arr_cmpkey;
		qsort((void *)list,arr->size,sizeof(QArrayNode),cmp);
		arr_link(arr,list);
	}
}

void q_array_reverse(QArray arr) {
	if(arr!=NULL && arr->size>1) {
		QArrayNode list[arr->size+1],n1;
		int i,n = arr->size/2;
		arr_list(arr,list);
		for(i=0; i<n; ++i) {
			n1 = list[i];
			list[i] = list[arr->size-1-i];
			list[arr->size-1-i] = n1;
		}
		arr_link(arr,list);
	}
}

void q_array_reset(QArray arr) {
	if(arr!=NULL) arr->iter = NULL;
}

int q_array_min(QArray arr,QType *v) {
	if(arr==NULL || arr->min==NULL) return 0;
	arr->iter = arr->min;
	if(v!=NULL) *v = arr->iter->value;
	return 1;
}

int q_array_max(QArray arr,QType *v) {
	if(arr==NULL || arr->max==NULL) return 0;
	arr->iter = arr->max;
	if(v!=NULL) *v = arr->iter->value;
	return 1;
}

int q_array_first(QArray arr,QType *v) {
	if(arr==NULL || arr->first==NULL) return 0;
	arr->iter = arr->first;
	if(v!=NULL) *v = arr->iter->value;
	return 1;
}

int q_array_last(QArray arr,QType *v) {
	if(arr==NULL || arr->last==NULL) return 0;
	arr->iter = arr->last;
	if(v!=NULL) *v = arr->iter->value;
	return 1;
}

int q_array_previous(QArray arr,QType *v) {
	if(arr==NULL) return 0;
	if(arr->iter!=NULL) arr->iter = arr->iter->back;
	if(arr->iter==NULL) return 0;
	if(v!=NULL) *v = arr->iter->value;
	return 1;
}

int q_array_next(QArray arr,QType *v) {
	if(arr==NULL) return 0;
	if(arr->iter!=NULL) arr->iter = arr->iter->next;
	if(arr->iter==NULL) return 0;
	if(v!=NULL) *v = arr->iter->value;
	return 1;
}

int q_array_each(QArray arr,QType *v) {
	if(arr==NULL) return 0;
//fprintf(stdout,"q_array_each(arr: %p, iter: %p, next: %p, value: %p)\n",
//arr,arr->iter,arr->iter==NULL? NULL : arr->iter->next,arr->iter==NULL? NULL : arr->iter->value.v);
	arr->iter = arr->iter==NULL? arr->first : arr->iter->next;
	if(arr->iter==NULL) return 0;
	if(v!=NULL) *v = arr->iter->value;
	return 1;
}

int q_array_each_type(QArray arr,int type,QType *v) {
	if(arr==NULL) return 0;
	arr->iter = arr->iter==NULL? arr->first : arr->iter->next;
	while(arr->iter!=NULL && arr->iter->value.t!=type) arr->iter = arr->iter->next;
	if(arr->iter==NULL) return 0;
	if(v!=NULL) *v = arr->iter->value;
	return 1;
}

int q_array_each_r(QArray arr,QType *v) {
	if(arr==NULL) return 0;
	arr->iter = arr->iter==NULL? arr->last : arr->iter->back;
	if(arr->iter==NULL) return 0;
	if(v!=NULL) *v = arr->iter->value;
	return 1;
}

int q_array_each_r_type(QArray arr,int type,QType *v) {
	if(arr==NULL) return 0;
	arr->iter = arr->iter==NULL? arr->last : arr->iter->back;
	while(arr->iter!=NULL && arr->iter->value.t!=type) arr->iter = arr->iter->back;
	if(arr->iter==NULL) return 0;
	if(v!=NULL) *v = arr->iter->value;
	return 1;
}

void q_array_foreach(QArray arr,void (*f)(QType,void *),void *data) {
	if(arr!=NULL && arr->first!=NULL && f!=NULL)
		for(arr->iter=arr->first; arr->iter!=NULL; arr->iter=arr->iter->next)
			(*f)(arr->iter->value,data);
}

void q_array_foreach_type(QArray arr,int type,void (*f)(QType,void *),void *data) {
	if(arr!=NULL && arr->first!=NULL && f!=NULL)
		for(arr->iter=arr->first; arr->iter!=NULL; arr->iter=arr->iter->next)
			if(arr->iter->value.t==type) (*f)(arr->iter->value,data);
}

void q_array_foreach_r(QArray arr,void (*f)(QType,void *),void *data) {
	if(arr!=NULL && arr->last!=NULL && f!=NULL)
		for(arr->iter=arr->last; arr->iter!=NULL; arr->iter=arr->iter->back)
			(*f)(arr->iter->value,data);
}

void q_array_foreach_r_type(QArray arr,int type,void (*f)(QType,void *),void *data) {
	if(arr!=NULL && arr->last!=NULL && f!=NULL)
		for(arr->iter=arr->last; arr->iter!=NULL; arr->iter=arr->iter->back)
			if(arr->iter->value.t==type) (*f)(arr->iter->value,data);
}

QType q_array_key(const QArray arr) { return arr!=NULL && arr->iter!=NULL? arr->iter->key : (QType){ t:ARR_UNDEFINED, i:0 }; }
QType q_array_previous_key(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->back!=NULL? arr->iter->back->key : (QType){ t:ARR_UNDEFINED, i:0 }; }
QType q_array_next_key(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->next!=NULL? arr->iter->next->key : (QType){ t:ARR_UNDEFINED, i:0 }; }
int q_array_key_is_int(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->key.t==ARR_INTEGER? 1 : 0; }
int q_array_key_is_string(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->key.t==ARR_STRING? 1 : 0; }

QType q_array_value(const QArray arr) { return arr!=NULL && arr->iter!=NULL? arr->iter->value : (QType){ t:ARR_UNDEFINED, i:0 }; }
QType q_array_previous_value(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->back!=NULL? arr->iter->back->value : (QType){ t:ARR_UNDEFINED, i:0 }; }
QType q_array_next_value(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->next!=NULL? arr->iter->next->value : (QType){ t:ARR_UNDEFINED, i:0 }; }
int q_array_value_is_null(const QArray arr) { return arr==NULL || arr->iter==NULL || arr->iter->value.t==ARR_NULL? 1 : 0; }
int q_array_value_is_bool(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->value.t==ARR_BOOLEAN? 1 : 0; }
int q_array_value_is_int(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->value.t==ARR_INTEGER? 1 : 0; }
int q_array_value_is_float(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->value.t==ARR_FLOAT? 1 : 0; }
int q_array_value_is_decimal(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->value.t==ARR_DECIMAL? 1 : 0; }
int q_array_value_is_string(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->value.t==ARR_STRING? 1 : 0; }
int q_array_value_is_array(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->value.t==ARR_ARRAY? 1 : 0; }
int q_array_value_is_pointer(const QArray arr) { return arr!=NULL && arr->iter!=NULL && arr->iter->value.t==ARR_POINTER? 1 : 0; }

QArrayIter q_array_get_iter(QArray arr) {
	return arr!=NULL? (QArrayIter)arr->iter : NULL;
}

void q_array_set_iter(QArray arr,QArrayIter iter) {
	if(arr!=NULL) arr->iter = (QArrayNode)iter;
}

/* Print table header. */
static void arr_print_header(FILE *fp,const QArray arr) {
	fprintf(fp,"Table (size=%lu,capacity=%lu,min=%ld,max=%ld):\n",
		(unsigned long)arr->size,(unsigned long)arr->cap,
		arr->min!=NULL? arr->min->key.i : 0,arr->max!=NULL? arr->max->key.i : 0);
}

/* Print array recursively. */
static void arr_print(FILE *fp,const QArray arr,int ind) {
	int j;
	QArrayNode n;
	if(arr==NULL || arr->first==NULL) {
		fprintf(fp,"(empty)");
		return;
	}
	for(n=arr->first; n!=NULL; n=n->next) {
		for(j=0; j<ind; j++) fputc('\t',fp);
		if(n->key.t==ARR_INTEGER) fprintf(fp,"%ld: ",n->key.i);
		else fprintf(fp,"\"%s\": ",n->key.s);
		if(n->value.t==ARR_NULL) fprintf(fp,"(null)");
		else if(n->value.t==ARR_BOOLEAN) fprintf(fp,"%s",n->value.i? "true" : "false");
		else if(n->value.t==ARR_INTEGER) fprintf(fp,"%ld",n->value.i);
		else if(n->value.t==ARR_FLOAT) fprintf(fp,"%g",n->value.f);
		else if(n->value.t==ARR_DECIMAL) fprintf(fp,"%s",q_dec_tostr(n->value.d));
		else if(n->value.t==ARR_STRING) fprintf(fp,"\"%s\"",n->value.s);
		else if(n->value.t==ARR_ARRAY) {
			fprintf(fp,"[\n");
			arr_print(fp,n->value.a,ind+1);
			for(j=0; j<ind; j++) fputc('\t',fp);
			fputc(']',fp);
		} else fprintf(fp,"%p",n->value.v);

		fprintf(fp," {node: %p, back: %p, next: %p}\n",n,n->back,n->next);
//		fputc('\n',fp);
	}
	fflush(fp);
}

void q_array_print(FILE *fp,const QArray arr) {
	arr_print_header(fp,arr);
	arr_print(fp,arr,0);
	fflush(fp);
}

void q_array_print_table(FILE *fp,const QArray arr) {
	int i;
	QArrayNode n;
	q_array_print(fp,arr);
	fprintf(fp,"Table:\n");
	for(i=0; i<arr->cap; ++i) {
		fprintf(fp,"[%d]:",i);
		if(arr->table[i]!=NULL) {
			for(n=arr->table[i]; n!=NULL; n=n->table) {
				if(n->key.t==ARR_STRING) fprintf(fp," [\"%s\"",n->key.s);
				else fprintf(fp," [%ld",n->key.i);
				fprintf(fp,"|%" PRIu64 ", %p]",(uint64_t)n->hash,n->table);
			}
		}
		fputc('\n',fp);
	}
	fflush(fp);
}


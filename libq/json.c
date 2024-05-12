
#include "libq.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "json.h"
#include "_json.h"
#include "string.h"
#include "array.h"
#include "_array.h"


Q_ERR(__FILE__)

#ifndef USE_JSON_HASH_COMMENTS
#define USE_JSON_HASH_COMMENTS
#endif

static const char *json_escape_chars = "\\/\"\n\r\t\x08\x0c";


/*static const char *types[] = {
   "undefined",
   "null",
   "boolean",
   "integer",
   "double",
   "string",
   "array",
   "object",
   "function"
};*/

/*static int arr_to_json_types[] = {
	JSON_UNDEFINED,
	JSON_NULL,
	JSON_BOOLEAN,
	JSON_INTEGER,
	JSON_DOUBLE,
	JSON_DOUBLE,
	JSON_STRING,
	JSON_ARRAY,
	JSON_NULL
};*/


void q_json_free(QJson json) {
	if(json->value.t==JSON_ARRAY || json->value.t==JSON_OBJECT)
		q_array_free(json->value.a);
	q_free(json);
}

/*static const char *match_nested_brackets(const char *s) {
	char c,b1 = *s,b2 = b1=='('? ')' : (b1=='{'? '}' : (b1=='['? ']' : '\0'));
	if(b2!='\0') {
		int n;
		for(c=*++s,n=1; c!='\0'; c=*++s) {
			if(c==b1) ++n;
			else if(c==b2 && (--n)==0) break;
		}
		if(c==b2) ++s;
	}
	return s;
}*/

static const char *skip_comment(const char *s) {
//fprintf(stdout,"json: skip_comment(%5s)\n",s);
	if(*s=='/') {
		if(s[1]=='*') {
			for(s+=2; *s!='\0'; ++s)
				if(*s=='*' && s[1]=='/') { s += 2;break; }
		} else if(s[1]=='/') {
			for(s+=2; *s!='\0'; ++s)
				if(*s=='\n') break;
		}
#ifdef USE_JSON_HASH_COMMENTS
	} else if(*s=='#') {
		for(s+=2; *s!='\0'; ++s)
			if(*s=='\n') break;
#endif
	}
	return s;
}

static const char *skip_whitespace(const char *s) {
//fprintf(stdout,"json: skip_whitespace(%5s)\n",s);
	for(; *s!='\0'; ++s) {
		if(*s=='/'
#ifdef USE_JSON_HASH_COMMENTS
			|| *s=='#'
#endif
		) s = skip_comment(s);
		if(strchr(q_whitespace,*s)==NULL) break;
	}
	return s;
}

static const char *skip_string(const char *s) {
	char q = *s++,c;
	if(q_isquote(q)) {
		for(c=*s; c!='\0' && c!=q; c=*++s)
			if(c=='\\') {
				c = *++s;
				if(c=='x') s += 2;
				else if(c=='u') s += 4;
				else if(c=='U') s += 8;
			}
		if(c==q) ++s;
	}
	return s;
}

static const char *skip_function(const char *s) {
	return s;
}


static char *decode_string(char *s);
static QArray decode_array(const char **s);
static QArray decode_object(const char **s);

static char *decode_key(const char **s) {
	const char *s1 = skip_whitespace(*s),*s2;
	char c = *s1,*k = NULL;
	if(q_isquote(c)) {
		s2 = skip_string(s1);
		k = q_substr(s1,1,s2-s1-2);
	} else {
		for(s2=s1; *s2!='\0' && *s2!=':'; ++s2)
			if(strchr(q_whitespace,*s2)!=NULL) break;
		k = q_substr(s1,0,s2-s1);
	}
	*s = skip_whitespace(s2);
	return k;
}

static QType decode_value(const char **s) {
	QType v = { t:JSON_UNDEFINED, i:0 };
	const char *s1 = skip_whitespace(*s),*s2 = s1;
	char c = *s1;
	if(c=='-' || c=='+') {
		s2 = s1+1;
		v = decode_value(&s2);
		if(c=='-') {
			if(v.t==JSON_INTEGER) v.i = -v.i;
			else if(v.t==JSON_DOUBLE) v.d = -v.d;
			else v.t = JSON_UNDEFINED,v.i = 0,s2 = s1+1;
		}
	} else if(c=='\0') {
		v = (QType){ t:JSON_NULL, v:NULL };
	} else if(c=='n' && strncmp(s1,"null",4)==0) {
		s2 = s1+4;
		v = (QType){ t:JSON_NULL, v:NULL };
	} else if(c=='t' && strncmp(s1,"true",4)==0) {
		s2 = s1+4;
		v = (QType){ t:JSON_BOOLEAN, i:1 };
	} else if(c=='f' && strncmp(s1,"false",5)==0) {
		s2 = s1+5;
		v = (QType){ t:JSON_BOOLEAN, i:0 };
	} else if(q_isdigit(c)) {
		for(s2=s1,c=*++s2; q_isdigit(c); c=*++s2);
		if(c=='.') {
			v = (QType){ t:JSON_DOUBLE, d:q_dec_str(s1,&s2) };
		} else {
			v = (QType){ t:JSON_INTEGER, i:strtol(s1,(char **)&s2,0) };
		}
	} else if(q_isquote(c)) {
		s2 = skip_string(s1);
		v = (QType){ t:JSON_STRING, s:(s2-s1-2==0? q_strdup("") : decode_string(q_substr(s1,1,s2-s1-2))) };
//fprintf(stderr,"decode_value(offset=%d, len=%d)\n",1,(int)(s2-s1-2));
	} else if(c=='[') {
		s2 = s1;
		v = (QType){ t:JSON_ARRAY, a:decode_array(&s2) };
	} else if(c=='{') {
		s2 = s1;
		v = (QType){ t:JSON_OBJECT, a:decode_object(&s2) };
	} else if(c=='f' && strncmp(s1,"function",8)==0) {
		s2 = skip_function(s1);
		v = (QType){ t:JSON_FUNCTION, s:q_substr(s1,0,s2-s1) };
	}
	*s = skip_whitespace(s2);
	return v;
}

static char *decode_string(char *s) {
	static const char esc[] = {
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,		// 0-15
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,		// 16-31
		0,   0,   '"', 0,   0,   0,   0,   '\'',0,   0,   0,   0,   0,   0,   0,   '/',	// 32-47
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,		// 48-63
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,		// 64-79
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   '\\',0,   0,   0,		// 80-95
		0,   0,   '\b',0,   0,   0,   '\f',0,   0,   0,   0,   0,   0,   0,   '\n',0,		// 96-111
		0,   0,   '\r',0,   '\t',0,   '\v',0,   0,   0,   0,   0,   0,   0,   0,   0,		// 112-127
	};
	char *s1 = s,*s2 = s1;
	for(; *s2!='\0'; ++s1,++s2) {
		if(*s2=='\\') *s1 = esc[(int)*++s2];
		else *s1 = *s2;
	}
	*s1 = '\0';
	return s;
}

static QArray decode_array(const char **s) {
	int i;
	const char *s1 = *s;
	QArray a = q_array_new(0,ARR_ST_STRING_POINTER|ARR_ST_STRING_FREE|ARR_ST_ARRAY_POINTER|ARR_ST_ARRAY_FREE|ARR_ST_VECTOR);
	QType v;
	for(i=0; *s1!='\0' && *s1!=']'; ++i) {
		++s1;
		v = decode_value(&s1);
		if(v.t==JSON_UNDEFINED || v.t==JSON_NULL) q_array_set_pointer(a,i,NULL);
		else if(v.t==JSON_BOOLEAN) q_array_set_bool(a,i,v.i);
		else if(v.t==JSON_INTEGER) q_array_set_int(a,i,v.i);
		else if(v.t==JSON_DOUBLE) q_array_set_decimal(a,i,v.d);
		else if(v.t==JSON_STRING || v.t==JSON_FUNCTION) q_array_set(a,i,v.s);
		else if(v.t==JSON_ARRAY || v.t==JSON_OBJECT) q_array_set_array(a,i,v.a);
		if(*s1!=',' && *s1!=']') {
			*s = s1;
			break;
		}
	}
	*s = s1+1;
	return a;
}

static QArray decode_object(const char **s) {
	int i;
	const char *s1 = *s;
	char *k;
	QArray a = q_array_new(0,ARR_ST_STRING_POINTER|ARR_ST_STRING_FREE|ARR_ST_ARRAY_POINTER|ARR_ST_ARRAY_FREE|ARR_ST_HASHTABLE);
	QType v;
	for(i=0; *s1!='\0' && *s1!='}'; ++i) {
		++s1;
		k = decode_key(&s1);
//fprintf(stderr,"decode_object(k=%s)\n",k);
		if(*s1!=':') {
			*s = s1;
			break;
		}
		++s1;
		v = decode_value(&s1);
		if(v.t==JSON_UNDEFINED || v.t==JSON_NULL) q_array_put_pointer(a,k,NULL);
		else if(v.t==JSON_BOOLEAN) q_array_put_bool(a,k,v.i);
		else if(v.t==JSON_INTEGER) q_array_put_int(a,k,v.i);
		else if(v.t==JSON_DOUBLE) q_array_put_decimal(a,k,v.d);
		else if(v.t==JSON_STRING || v.t==JSON_FUNCTION) q_array_put(a,k,v.s);
		else if(v.t==JSON_ARRAY || v.t==JSON_OBJECT) q_array_put_array(a,k,v.a);
		if(*s1!=',' && *s1!='}') {
			*s = s1;
			break;
		}
	}
	*s = s1+1;
	return a;
}


QJson q_json_decode(const char *json) {
	if(json==NULL) return NULL;
	else {
		QJson j = (QJson)q_malloc(sizeof(struct _QJson));
		j->value = decode_value(&json);
		return j;
	}
}

static void encode_value(QString buf,QType v,int ind,int style);

static void encode_array(QString buf,QArray a,int ind,int style) {
	const char *quote = (style&JSON_ESCAPED)? "\\\"" : ((style&JSON_SQL)? "\"\"" : "\"");
	QArrayNode n;
	QType v;
	int ht = q_array_is_hashtable(a),i;
	q_string_append(buf,(style&JSON_PRETTY)? (ht? "{\n" : "[\n") : (ht? "{" : "["));
	for(n=a->first; n!=NULL; n=n->next) {
		if(style&JSON_PRETTY)
			for(i=ind+1; i>0; --i) q_string_append_char(buf,'\t');
		if(ht) {
			if(n->key.t==ARR_INTEGER) {
				q_string_append_int(buf,(int)n->key.i);
			} else {
				q_string_append(buf,quote);
				q_string_append(buf,n->key.s);
				q_string_append(buf,quote);
			}
			q_string_append_char(buf,':');
			if(style&JSON_PRETTY) q_string_append_char(buf,' ');
		}
		v = n->value;
		encode_value(buf,v,ind+1,style);
		if(n->next!=NULL || (style&JSON_PRETTY))
			q_string_append(buf,(style&JSON_PRETTY)? (n->next==NULL? "\n" : ",\n") : ",");
	}
	if(style&JSON_PRETTY)
		for(i=ind; i>0; --i) q_string_append_char(buf,'\t');
	q_string_append_char(buf,ht? '}' : ']');
}

static char *encode_string(const char *str,int style) {
	static const char esc[] = {
		0,   0,   0,   0,   0,   0,   0,   0,   'b', 't', 'n', 'v', 'f', 'r', 0,   0,    // 0-15
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    // 16-31
		0,   0,   '"', 0,   0,   0,   0,   '\'',0,   0,   0,   0,   0,   0,   0,   '/',  // 32-47
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    // 48-63
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    // 64-79
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   '\\',0,   0,   0,    // 80-95
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    // 96-111
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    // 112-127
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                 // 128-159
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                 // 160-191
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                 // 192-223
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                 // 224-255
	};
	char *s = NULL,*p,c;
	int i,n;
	for(s=(char *)str,i=0,n=0; *s!='\0'; ++s,++i)
		if((c=esc[(int)*s])) {
			++n;
			if((style&JSON_SQL) && (c=='\'' || c=='"')) n += 2;
		}
	s = (char *)q_malloc(i+n+1);
	if(s==NULL) q_err(ERR_MALLOC,NULL);
	else {
		for(p=s; *str!='\0'; ++str) {
			if(!(c=esc[(int)*str])) *p++ = *str;
			else {
				*p++ = '\\',*p++ = c;
				if((style&JSON_SQL) && (c=='\'' || c=='"')) *p++ = '\\',*p++ = c;
			}
		}
		*p = '\0';
	}
	return s;
}

static void encode_value(QString buf,QType v,int ind,int style) {
	const char *quote = (style&JSON_ESCAPED)? "\\\"" : ((style&JSON_SQL)? "\"\"" : "\"");
	switch(v.t) {
		case JSON_UNDEFINED:q_string_append(buf,"undefined");break;
		case JSON_NULL:q_string_append(buf,"null");break;
		case JSON_BOOLEAN:q_string_append(buf,v.i? "true" : "false");break;
		case JSON_INTEGER:q_string_append_int(buf,v.i);break;
		case JSON_DOUBLE:q_string_append(buf,q_dec_tostr(v.d));break;
		case JSON_STRING:
		{
			q_string_append(buf,quote);
			if(strpbrk(v.s,json_escape_chars)!=NULL) {
				char *s = encode_string(v.s,style);
				if(s!=NULL) {
					q_string_append(buf,s);
					q_free(s);
				}
			} else q_string_append(buf,v.s);
			q_string_append(buf,quote);
			break;
		}
		case JSON_ARRAY:
		case JSON_OBJECT:encode_array(buf,v.a,ind,style);break;
		case JSON_FUNCTION:q_string_append(buf,v.s);break;
	}
}

char *q_json_encode(const QJson json,int style) {
	QString buf = q_string_new();
	char *ret;
	encode_value(buf,json->value,0,style);
	ret = buf->ptr;
	q_free(buf);
	return ret;
}

char *q_json_encode_string(const char *str,int style) {
	if(strpbrk(str,json_escape_chars)!=NULL) return encode_string(str,style);
	return q_strdup(str);
}

char *q_json_encode_array(QArray arr,int style) {
	QString buf = q_string_new();
	char *ret;
	encode_array(buf,arr,0,style);
	ret = buf->ptr;
	q_free(buf);
	return ret;
}

QType q_json_get(QJson json,const char *key) {
	if(key!=NULL && *key!='\0' && (json->value.t==JSON_ARRAY || json->value.t==JSON_OBJECT)) {
		QType v = q_array_fetch(json->value.a,key);
		if(v.t==ARR_ARRAY) v.t = q_array_is_hashtable(v.a)? JSON_OBJECT : JSON_ARRAY;
		return v;
	}
	return json->value;
}

QJson q_json_read(const char *fn) {
	long n = 0;
	QJson j = NULL;;
	FILE *fp = fopen(fn,"r");
	if(fp==NULL) q_err(ERR_NULL_POINTER,NULL);
	else {
		fseek(fp,0,SEEK_END);
		n = ftell(fp);
		if(n>0) {
			char buf[n+1];
			fseek(fp,0,SEEK_SET);
			fread(buf,1,n,fp);
			buf[n] = '\0';
			j = q_json_decode(buf);
		}
		fclose(fp);
	}
	return j;
}

void q_json_write(const char *fn,const QJson json,int style) {
	FILE *fp = fopen(fn,"w");
	if(fp==NULL) q_err(ERR_NULL_POINTER,NULL);
	else {
		char *str = q_json_encode(json,style);
		fprintf(fp,"%s\n",str);
		q_free(str);
		fclose(fp);
	}
}


static void print_value(FILE *fp,QType v,int ind);

static void print_array(FILE *fp,QArray a,int ind) {
	QArrayNode n;
	int ht = !!(a->style&ARR_ST_HASHTABLE),i;
	fprintf(fp,"%s\n",ht? "[object]{" : "[array][");
	for(n=a->first; n!=NULL; n=n->next) {
		for(i=0; i<=ind; ++i) fputc('\t',fp);
		if(ht) {
			if(n->key.t==ARR_INTEGER) fprintf(fp,"%ld=>",n->key.i);
			else fprintf(fp,"\"%s\"=>",n->key.s);
		}
		print_value(fp,n->value,ind+1);
		if(n->next!=NULL) fputc(',',fp);
		fputc('\n',fp);
	}
	for(i=0; i<ind; ++i) fputc('\t',fp);
	fputc(ht? '}' : ']',fp);
}

static void print_value(FILE *fp,QType v,int ind) {
	switch(v.t) {
		case JSON_UNDEFINED:fprintf(fp,"[undefined]");break;
		case JSON_NULL:fprintf(fp,"[null]");break;
		case JSON_BOOLEAN:fprintf(fp,"[bool]%s",v.i? "true" : "false");break;
		case JSON_INTEGER:fprintf(fp,"[int]%ld",v.i);break;
		case JSON_DOUBLE:fprintf(fp,"[double]%s",q_dec_tostr(v.d));break;
		case JSON_STRING:fprintf(fp,"[str]\"%s\"",v.s);break;
		case JSON_ARRAY:
		case JSON_OBJECT:print_array(fp,v.a,ind);break;
		case JSON_FUNCTION:fprintf(fp,"[function]%s",v.s);break;
	}
}

void q_json_print(FILE *fp,const QJson json) {
	print_value(fp,json->value,0);
}




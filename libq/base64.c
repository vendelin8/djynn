
#include "libq.h"
#include <stdlib.h>
#include <string.h>
#include "base64.h"

#define __ -1

static const char base64_code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const int base64_index[256] = {
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,62, __,__,__,63, /* '+','/' */
    52,53,54,55, 56,57,58,59, 60,61,__,__, __, 0,__,__, /* '0'-'9', '=' */
    __, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14, /* 'A'-'O' */
    15,16,17,18, 19,20,21,22, 23,24,25,__, __,__,__,__, /* 'P'-'Z' */
    __,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40, /* 'a'-'o' */
    41,42,43,44, 45,46,47,48, 49,50,51,__, __,__,__,__, /* 'p'-'z' */
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
};

long q_base64_encoded_size(long len) {
	return ((len+2)/3)*4;
}

long q_base64_decoded_size(long len) {
	return (len/4)*3;
}

char *q_base64_encode(const void *ptr,long len,long *elen) {
	const unsigned char *p = (const unsigned char *)ptr;
	char *dst = NULL,*d;
	long i,l = -1;
	if(p!=NULL) {
		l = q_base64_encoded_size(len);
		dst = (char *)q_malloc((l+4)*sizeof(char));
		if(dst!=NULL) {
			for(i=0,l=0,d=dst; i<len; p+=3,i+=3,l+=4,d+=4) {
				d[0] = (char)base64_code[(int)(p[0]>>2)];
				d[1] = (char)base64_code[(int)(((p[0]&0x03)<<4)|((p[1]&0xf0)>>4))];
				d[2] = (char)(len-i>1? base64_code[(int)(((p[1]&0x0f)<<2)|((p[2]&0xc0)>>6))] : '=');
				d[3] = (char)(len-i>2? base64_code[(int)(p[2]&0x3f)] : '=');
			}
			*d = '\0';
		}
	}
	if(elen!=NULL) *elen = l;
	return (void *)dst;
}

void *q_base64_decode(const char *b64,long len,long *dlen) {
	const unsigned char *p = (const unsigned char *)b64;
	unsigned char *dst = NULL,*d;
	long i,l = -1;
	if(p!=NULL) {
		if(len<=0) len = strlen(b64);
		l = q_base64_decoded_size(len);
		dst = (unsigned char *)q_malloc((l+4)*sizeof(unsigned char));
		if(dst!=NULL) {
			register int q,x,y,z;
			for(i=0,l=0,d=dst; i<len; p+=4,i+=4,l+=3,d+=3) {
				q = base64_index[p[0]];
				x = base64_index[p[1]];
				y = base64_index[p[2]];
				z = base64_index[p[3]];
				if(q==__ || x==__ || y==__ || z==__) break;
				d[0] = (unsigned char)(q<<2|x>>4);
				d[1] = (unsigned char)(x<<4|y>>2);
				d[2] = (unsigned char)(((y<<6)&0xc0)|z);
				if(p[2]=='=') { p += 4,d += 2,l += 2;break; }
			}
			*d = '\0';
		}
	}
	if(dlen!=NULL) *dlen = l;
	return (void *)dst;
}



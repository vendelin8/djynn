
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "base58.h"
#include "string.h"

#define __ -1

static const char base58_code[]="123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static const int base58_index[256] = {
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __, 0, 1, 2,  3, 4, 5, 6,  7, 8,__,__, __,__,__,__, // '0'-'9'
    __, 9,10,11, 12,13,14,15, 16,__,17,18, 19,20,21,__, // 'A'-'O'
    22,23,24,25, 26,27,28,29, 30,31,32,__, __,__,__,__, // 'P'-'Z'
    __,33,34,35, 36,37,38,39, 40,41,42,43, __,44,45,46, // 'a'-'o'
    47,48,49,50, 51,52,53,54, 55,56,57,__, __,__,__,__, // 'p'-'z'
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
    __,__,__,__, __,__,__,__, __,__,__,__, __,__,__,__,
};


static char divmod58(char number[],int start,int len) {
	int i;
	uint32_t digit256,temp,remainder = 0;
	for(i=start; i<len; i++) {
		digit256 = (uint32_t)(number[i]&0xFF);
		temp = remainder*256+digit256;
		number[i] = (char)(temp/58);
		remainder = temp % 58;
	}
	return (char)remainder;
}

static char divmod256(char number58[],int start,int len) {
	int i;
	uint32_t digit58,temp,remainder = 0;
	for(i=start; i<len; i++) {
		digit58 = (uint32_t)(number58[i]&0xFF);
		temp = remainder*58+digit58;
		number58[i] = (char)(temp/256);
		remainder = temp % 256;
	}
	return (char)remainder;
}

long q_base58_encoded_size(long len) {
	return ((len+4)/5)*7;
}

long q_base58_decoded_size(long len) {
	return (len/4)*3;
}

char *q_base58_encode(const void *ptr,long len,long *elen) {
	char *dst = NULL;
	long l = -1;
	if(ptr!=NULL) {
		if(len>0) {
			long tlen= len*2,j = tlen,zc = 0,start,mod;
			char *copy = q_substr((const char *)ptr,0,len);
			char temp[tlen];

			while(zc<len && copy[zc]=='\0') ++zc;

			start = zc;
			while(start<len) {
				mod = divmod58(copy,start,len);
				if(copy[start]==0) ++start;
				temp[--j] = base58_code[mod];
			}

			while(j<tlen && temp[j]==base58_code[0]) ++j;

			while(--zc>=0) temp[--j] = base58_code[0];

			q_free(copy);
			if(tlen-j>0) {
				l = tlen-j;
				dst = (char *)q_malloc(l*sizeof(char));
				memcpy(dst,&temp[j],l);
				dst[l] = '\0';
			}
		}
	}
	if(elen!=NULL) *elen = l;
	return dst;
}

void *q_base58_decode(const char *b58,long len,long *dlen) {
	unsigned char *dst = NULL;
	long l = -1;
	if(len<=0) len = strlen(b58);
	if(len>0) {
		long i,j = len,c,digit58,zc = 0,start,mod;
		char input58[len];
		char temp[len];

		for(i=0; i<len; ++i) {
			c = b58[i];
			digit58 = -1;
			if(c>=0 && c<128) digit58 = base58_index[c];
			if(digit58<0) {
				fprintf(stderr,"Illegal character '%c' at %ld.\n",(char)c,i);
				goto base58_decode_end;
			}
			input58[i] = (char)digit58;
		}

		while(zc<len && input58[zc]==0) ++zc;
		start = zc;
		while(start<len) {
			mod = divmod256(input58,start,len);
			if(input58[start]==0) ++start;
			temp[--j] = mod;
		}

		while(j<len && temp[j]==0) ++j;

		if(len-(j-zc)>0) {
			l = len-(j-zc);
			dst = (unsigned char *)q_malloc(l*sizeof(unsigned char));
			memcpy(dst,&temp[j-zc],l);
			dst[l] = '\0';
		}
	}
base58_decode_end:
	if(dlen!=NULL) *dlen = l;
	return (void *)dst;
}



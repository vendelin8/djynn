
#include "libq.h"
#include <stdlib.h>
#include <stdint.h>
#include "rle.h"


long q_rle_compressed_size(const void *ptr,long len) {
	const uint8_t *p = (const uint8_t *)ptr;
	long i,n,l = -1;
	if(p!=NULL) {
		for(i=0,n=1,l=0; i<len; ++i,++p)
			if(i<len-1 && *p==p[1] && n<=0x100) ++n;
			else l += n==1? 1 : 3,n = 1;
	}
	return l;
}

long q_rle_extracted_size(const void *ptr,long len) {
	const uint8_t *p = (const uint8_t *)ptr;
	long i,l = -1;
	if(p!=NULL) {
		for(i=0,l=0; i<len; )
			if(*p!=p[1] || i==len-1) ++i,++l,++p;
			else i += 3,l += ((long)p[2])+2,p += 3;
	}
	return l;
}

void *q_rle_compress(const void *ptr,long len,long *clen) {
	const uint8_t *p = (const uint8_t *)ptr;
	uint8_t *dst = NULL,*d;
	long i,n,l = -1;
	if(p!=NULL) {
		l = q_rle_compressed_size(p,len);
		dst = (uint8_t *)q_malloc((l+4)*sizeof(uint8_t));
		if(dst!=NULL) {
			for(i=0,n=1,l=0,d=dst; i<len; ++i,++p)
				if(i<len-1 && *p==p[1] && n<=0x100) ++n;
				else if(n==1) *d++ = *p,++l,n = 1;
				else d[0] = d[1] = *p,d[2] = (unsigned char)(n-2),d += 3,l += 3,n = 1;
			*d = 0;
		}
	}
	if(clen!=NULL) *clen = l;
	return (void *)dst;
}

void *q_rle_extract(const void *ptr,long len,long *elen) {
	const uint8_t *p = (const uint8_t *)ptr;
	uint8_t *dst = NULL,*d,c;
	long i = 0,n,l = -1;
	if(p!=NULL) {
		l = q_rle_extracted_size(p,len);
		dst = (uint8_t *)q_malloc((l+4)*sizeof(uint8_t));
		if(dst!=NULL) {
			for(i=0,l=0,d=dst; i<len; )
				if(*p!=p[1] || i==len-1) *d++ = *p,++i,++l,++p;
				else for(c=*p,n=((size_t)p[2])+2,i+=3,l+=n,p+=3; n>0; --n) *d++ = c;
			*d = 0;
		}
	}
	if(elen!=NULL) *elen = l;
	return (void *)dst;
}




#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "type.h"
#include "string.h"
#include "lzw.h"
#include "bytes.h"

typedef struct _QLzw QLzw;
typedef struct _QLzwHeader QLzwHeader;
typedef struct _QLzwNode QLzwNode;
typedef struct _QLzwWord QLzwWord;

struct _QLzw {
	int bits;
	int index;
	int size;
	int cap;
	QLzwNode *dict;
	QLzwNode **table;
	QLzwWord *map;
};

struct _QLzwHeader {
	char header[4];
	uint32_t uncompressed;
	uint32_t compressed;
};

struct _QLzwNode {
	int index;
	int pos;
	int len;
	QHash hash;
	QLzwNode *table;
};

struct _QLzwWord {
	int pos;
	int len;
};

//static int mem_index;
//static int mem[1<<16];

/*// Print data with non-printable characters printed in cyan-hex (debug purposes):
void print_data(const unsigned char *str,int l,int t) {
	int i,n,c,c1,c2;
	for(i=0,n=0; i<l; ++i,++n,++str) {
		c = *str;
		if(c<0x20 || c>0x7f) {
			c1 = c>>4,c2 = c&0xf,++n;
			printf("\x1b[0;36m%c%c\x1b[0m",c1<=9? '0'+c1 : 'A'-10+c1,c2<=9? '0'+c2 : 'A'-10+c2);
		} else fputc(c,stdout);
	}
	for(; n<t; ++n) fputc(' ',stdout);
}*/


static void resize_table(QLzw *lzw,int n) {
	int i,m;
	QLzwNode **t = (QLzwNode **)q_malloc(n*sizeof(QLzwNode *));
	memset(t,0,n*sizeof(QLzwNode *));
	if(lzw->table!=NULL) {
		QLzwNode *node;
		q_free(lzw->table);
		for(i=0; i<lzw->index-0x100; ++i) {
			node = &lzw->dict[i];
			m = node->hash%n;
			node->table = t[m];
			if(node->table==node) node->table = NULL;
			t[m] = node;
		}
	}
	lzw->table = t;
	lzw->cap = n;

//printf("resize_table(bits: %d, cap: %d, index: %d)\n",lzw->bits,lzw->cap,lzw->index);

}

static void resize_dict(QLzw *lzw,int n) {
	int i;
	QLzwNode *node;
	if(lzw->dict==NULL) {
		lzw->dict = (QLzwNode *)q_malloc(n*sizeof(QLzwNode));
		lzw->index = 0x100;
	} else lzw->dict = (QLzwNode *)q_realloc(lzw->dict,n*sizeof(QLzwNode));
	for(i=0; i<n; ++i) {
		node = &lzw->dict[i];
		node->index = i+0x100;
		node->table = NULL;
	}
	lzw->size = n;

//printf("resize_dict(bits: %d, size: %d, index: %d)\n",lzw->bits,lzw->size,lzw->index);

	resize_table(lzw,lzw->size*2);

}

static int add_word(QLzw *lzw,const uint8_t *src,int pos,int len) {
	int m;
	QLzwNode *node;
	if(lzw->index-0x100==lzw->size || lzw->dict==NULL)
		resize_dict(lzw,lzw->size*2);
	node = &lzw->dict[lzw->index-0x100];
	node->pos = pos;
	node->len = len;
	node->hash = (QHash)q_crc32n((const char *)src,(long)pos,(long)len,0);
	m = node->hash%lzw->cap;
	node->table = lzw->table[m];
	if(node->table==node) node->table = NULL;
	lzw->table[m] = node;
	lzw->index++;

/*{char _w[len+1];
memcpy(_w,&src[pos],len);
_w[len] = '\0';
printf("%d = add_word([%d]\"%s\" [%d] hash: %x)\n",node->index,pos,_w,len,node->hash);}*/

	return node->index;
}

static int find_word(QLzw *lzw,const uint8_t *src,int pos,int len) {
	int ret = -1;
//QHash hash = 0;
//QLzwNode *node = NULL;

	if(len==1) return (int)src[pos];
	if(lzw->table!=NULL) {
		QHash hash = (QHash)q_crc32n((const char *)src,(long)pos,(long)len,0);
		QLzwNode *node = lzw->table[hash%lzw->cap];
		for(node=lzw->table[hash%lzw->cap]; node!=NULL; node=node->table)
			if(node->hash==hash && node->len==len && memcmp(&src[node->pos],&src[pos],len)==0) {
				ret = node->index;
				break;
			}
	}

/*{char _w[len+1];
memcpy(_w,&src[pos],len);
_w[len] = '\0';
printf("%d = find_word([%d]\"%s\" [%d] hash: %x)\n",ret,pos,_w,len,hash);}*/

	return ret;
}

void *q_lzw_compress(const void *ptr,long len,int hdr,long *clen) {
	QLzw lzw = {
		9,
		0x100,
		256,
		0,
		NULL,
		NULL,
		NULL
	};
	QLzwHeader header = {
		"LZW",
		len,
		0
	};
	long l = 0;
	long w = 0;
	long k = 2;
	long i;
	long m = 0x400;
	const uint8_t *src = (const uint8_t *)ptr;
	uint8_t *dst = (uint8_t *)q_malloc(m+9);
	uint8_t *d = dst;
	int o = 0;
	int s = (int)*src;
	int c;
	int b = (1<<lzw.bits)-1;
	uint64_t out = 0;
	resize_dict(&lzw,lzw.size);

	if(dst==NULL) goto lzw_compress_error;
/*printf("input: %s\n\
compress:\tbits\tindex\tcode\tw+k\n",src);
mem_index = 0;*/

	if(hdr) l += sizeof(QLzwHeader),d += l;

	for(i=0; i<len; ++i) {
		c = find_word(&lzw,src,w,k);
		if(c!=-1) {
			s = c;
			++k;
		} else {

/*// Print compress data...
{char _w[k+1]; memcpy(_w,&src[w],k);
_w[k] = '\0';
if(s>=0x20 && s<=0x7f) printf("\
compress:\t%d\t%d\t'%c'",lzw.bits,lzw.index,(char)s);
else printf("\
compress:\t%d\t%d\t%4d",lzw.bits,lzw.index,s);
printf("\t[%d]",(int)k);
print_data((const unsigned char *)_w,k,16);
printf("\n");}*/
//mem[mem_index++] = s;

			out |= ((uint64_t)s)<<o;
			o += lzw.bits;
			add_word(&lzw,src,w,k);
			w += k-1;
			k = 2;
			s = (int)src[w];

			if(lzw.index>b) {
				++lzw.bits;
				b = (1<<lzw.bits)-1;
			}

			if(o>=40) {
				if(l>=m) {
					m <<= 1;
					dst = (uint8_t *)q_realloc(dst,m+9);
					d = &dst[l];
				}
				if(dst==NULL) goto lzw_compress_error;
				while(o>=8) {
					*d++ = (uint8_t)(out&0xff);
					++l;
					out >>= 8;
					o -= 8;
				}
			}
		}
	}
	if(c!=-1) {
		out |= ((uint64_t)c)<<o;
		o += lzw.bits;
	}
	while(o>0) {
		*d++ = (uint8_t)(out&0xff);
		++l;
		out >>= 8;
		o -= 8;
	}
	*d = 0;

	if(hdr) {
		header.compressed = l;
#if __BYTE_ORDER == __BIG_ENDIAN
		header.uncompressed = bswap32(hdr.uncompressed);
		header.compressed = bswap32(hdr.compressed);
#endif
		memcpy(dst,&header,sizeof(QLzwHeader));
	}

	goto lzw_compress_finalize;
lzw_compress_error:
	if(dst!=NULL) q_free(dst);
	dst = NULL;
	l = -1;
lzw_compress_finalize:
	if(lzw.dict!=NULL) q_free(lzw.dict);
	if(lzw.table!=NULL) q_free(lzw.table);
	if(clen!=NULL) *clen = l;
	return (void *)dst;
}

static void resize_map(QLzw *lzw,int n) {
	if(lzw->map==NULL) lzw->map = (QLzwWord *)q_malloc(n*sizeof(QLzwWord));
	else lzw->map = (QLzwWord *)q_realloc(lzw->map,n*sizeof(QLzwWord));
	lzw->size = n;

//printf("resize_map(bits: %d, size: %d, index: %d)\n",lzw->bits,lzw->size,lzw->index);

}

void *q_lzw_extract(const void *ptr,long len,int hdr,long *elen) {
	QLzw lzw = {
		9,
		0x100,
		256,
		0,
		NULL,
		NULL,
		NULL
	};
	QLzwHeader header;
	QLzwWord *word;
	long i;
	long l = 0;
	long w0 = 0;
	long w1 = 0;
	long k0 = 0;
	long k1 = 0;
	long m = 0x400;
	const uint8_t *src = (const uint8_t *)ptr;
	const uint8_t *s = src;
	uint8_t *dst = NULL;
	uint8_t *d;
	int o = 0;
	int c;
	uint64_t out = 0;
	uint64_t b = (uint64_t)((1<<lzw.bits)-1);
	resize_map(&lzw,lzw.size);

int i1;
uint8_t *d1,*d2;

//mem_index = 0;
/*printf("\
extract:\tbits\tindex\tcode\tw+k\n");*/

	if(hdr) {
		memcpy(&header,s,sizeof(QLzwHeader));
		if(strcmp(header.header,"LZW")!=0)
			goto lzw_extract_error;
#if __BYTE_ORDER == __BIG_ENDIAN
		header.uncompressed = bswap32(hdr.uncompressed);
		header.compressed = bswap32(hdr.compressed);
#endif
		s += sizeof(QLzwHeader);
		len = header.compressed-sizeof(QLzwHeader);
		m = header.uncompressed;
	}

	dst = (uint8_t *)q_malloc(m+1);
	d = dst;
	if(dst==NULL) goto lzw_extract_error;

	for(i=0; i<len || o>0; ) {
		if(o<24 && i<len) {
			while(o<40 && i<len) {
				out |= ((uint64_t)(*s))<<o;
				o += 8;
				++s;
				++i;
			}
		}

		c = (int)(out&b);
		out >>= lzw.bits;
		o -= lzw.bits;

		if(k0+1>1) {
			if(lzw.index-0x100==lzw.size)
				resize_map(&lzw,lzw.size<<1);
			word = &lzw.map[lzw.index-0x100];
			word->pos = w0;
			word->len = k0+1;

			if(c==lzw.index) {
//printf("symbol [%d] equals index\n",(int)c);
				//memcpy(d,&dst[w0],k0+1); // <- Generates overlap errors
				for(d1=d,d2=&dst[w0],i1=k0+1; i1>0; ++d1,++d2,--i1) *d1 = *d2;
			}

			++lzw.index;

			if(lzw.index==b) {
				++lzw.bits;
				b = (uint64_t)((1<<lzw.bits)-1);
			}
		}

//if(mem[mem_index++]!=c) printf("mem index mismatch %d!=%d\n",mem[mem_index-1],c);
/*if(c>=lzw.index) {
printf("symbol [%d] exceeds index [%d]\n",(int)c,lzw.index);
break;}*/

		if(c<=0xff) k1 = 1;
		else {
			word = &lzw.map[c-0x100];
			k1 = word->len;
		}

		if(l+k1>=m) {
			m <<= 1;
			dst = (uint8_t *)q_realloc(dst,m+1);
			d = &dst[l];
		}
		if(dst==NULL) goto lzw_extract_error;

		if(c<=0xff) *d++ = (uint8_t)c;
		else {
			//memcpy(d,&dst[word->pos],k1); // <- Generates overlap errors
			for(d1=d,d2=&dst[word->pos],i1=k1; i1>0; ++d1,++d2,--i1) *d1 = *d2;
			d += k1;
		}
		l += k1;

/*// Print extract data...
{char _w[k0+2];
memcpy(_w,dst+w0,k0+1);
_w[k0+1] = '\0';
if(c>=0x20 && c<=0x7f)
printf("\
extract:\t%d\t%d\t'%c'",lzw.bits,lzw.index-1,(char)c);
else printf("\
extract:\t%d\t%d\t%4d",lzw.bits,lzw.index-1,c);
*d = '\0';
printf("\t[%d]",(int)k0+1);
print_data((const unsigned char *)_w,(int)k0+1,16);
printf("\t\"");
print_data(dst+(l<64? 0 : l-64),l<64? l : 64,0);
printf("\"\n");
fflush(stdout);}*/

		w0 = w1;
		k0 = k1;
		w1 = w0+k0;
		k1 = 0;
	}

//if(dst!=NULL) *d = '\0';
//printf("result: \"%s\"\n",dst);

	if(hdr) {
		/* Accept a margin of two extra bytes compared to header: */
		if(l-header.uncompressed<0 || l-header.uncompressed>2) 
			printf("Extracted data does not match size in header. [%ld!=%ld]\n",(long)header.uncompressed,(long)l);
		l = header.uncompressed;
	}

	goto lzw_extract_finalize;
lzw_extract_error:
	if(dst!=NULL) q_free(dst);
	dst = NULL;
	l = -1;
lzw_extract_finalize:
	if(lzw.map!=NULL) q_free(lzw.map);
	if(elen!=NULL) *elen = l;
	return (void *)dst;
}




#include "libq.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "random.h"
#include "string.h"


#define GENERATE_NUMBER(n,i) (n=(1812433253*((n^(n>>30))+(i++))))
#define RANDOM_NUMBER (r->num=(r->table[(r->index++)&0xff]^=(r->table[r->num&0xff]+r->num)))

static const double n2p32 = 1.0/4294967295.0;


typedef struct _QRandom {
	uint64_t seed;					// The initial seed from which the random sequence is generated.
	uint64_t num;					// The last random number in the sequence.
	uint64_t index;					// The index in the table.
	uint64_t table[256];			// The table used for random number generation.
} _QRandom;


QRandom q_rnd_new() {
	QRandom r = (QRandom)q_malloc(sizeof(_QRandom));
	memset(r,0,sizeof(_QRandom));
	q_rnd_seed(r,time(0));
	return r;
}

void q_rnd_free(QRandom r) {
	if(r!=NULL) q_free(r);
}

QRandom q_rnd_new_seed(uint64_t n) {
	QRandom r = (QRandom)q_malloc(sizeof(_QRandom));
	memset(r,0,sizeof(_QRandom));
	q_rnd_seed(r,n);
	return r;
}


void q_rnd_seed(QRandom r,uint64_t n) {
	int i;
	r->seed = n,r->num = n,r->index = 1;
	for(i=0; i<256; i++) {
		r->table[i] = GENERATE_NUMBER(r->num,r->index);
// printf("table[%3d] = %8lx\n",i,table[i]);
	}
	r->index = 0;
}

void q_rnd_seedn(QRandom r,const uint64_t n[],size_t l) {
	size_t i,j;
	uint64_t h = q_crcn((const char *)n,0,l*sizeof(uint64_t),0);
	r->seed = 0,r->num = 0,r->index = 1;
	for(i=0,j=0; i<256; ++i,++j) {
		if(j==l) j = 0;
		r->num ^= h^n[j];
		r->table[i] = GENERATE_NUMBER(r->num,r->index);
// printf("n[%3d] = %8lx\ttable[%3d] = %8lx\n",j,n[j],i,table[i]);
	}
	r->index = 0;
}

void q_rnd_seed_string(QRandom r,const char *s) {
	size_t t = sizeof(uint64_t),i = strlen(s),l = (i/t)+((i%t)==0? 0 : 1);
	char n[l*t];
	memset(n,0,l*t);
	memcpy(n,s,i);
	q_rnd_seedn(r,(uint64_t *)n,l);
}


uint32_t q_rnd_uint32(QRandom r) {
	return (uint32_t)RANDOM_NUMBER;
}

uint32_t q_rnd_uintN(QRandom r,int n) {
	return (n<=0 || n>32? 0 : (q_rnd_uint32(r)&(0xffffffff>>(32-n))));
}

int32_t q_rnd_int32(QRandom r) {
	return (int32_t)q_rnd_uint32(r);
}

uint64_t q_rnd_uint64(QRandom r) {
#if _WORDSIZE == 64
	return (uint64_t)RANDOM_NUMBER;
#else
	uint64_t n = (uint64_t)RANDOM_NUMBER;
	n = (n<<32)|(uint64_t)RANDOM_NUMBER;
	return n;
#endif
}

int64_t q_rnd_int64(QRandom r) {
	return (int64_t)q_rnd_uint64(r);
}

double q_rnd_real64(QRandom r) {
	return (double)q_rnd_uint32(r)*n2p32;
}

char q_rnd_alpha(QRandom r) {
	int i = q_rnd_uint32(r)%52;
	return i<26? 'A'+i : 'a'-26+i;
}

char q_rnd_alphanum(QRandom r) {
	int i = q_rnd_uint32(r)%62;
	return i<10? '0'+i : (i<36? 'A'-10+i : 'a'-36+i);
}

void q_rnd_write(FILE *fp,QRandom r) {
	fwrite(r,sizeof(_QRandom),1,fp);
}


void q_rnd_read(FILE *fp,QRandom r) {
	int i = fread(r,sizeof(_QRandom),1,fp);
	(void)i;
}




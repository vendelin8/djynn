
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "bytes.h"
#include "decimal.h"
#include "error.h"
#include "float.h"
#include "string.h"


Q_ERR(__FILE__)

//9223372036854775807
//92233720.36854775807

#define STRBUF_SIZE              16                             // Size of string buffer
#define STRDEC_SIZE              25                             // Maximum size in chars of a QDecimal written to string, including minus, decimal sign & final '\0'

#define _NUM_TOT                 19                             // Total number of digits in a QDecimal, including integer and decimal
#define _NUM_DEC                 11                             // Maximum number of decimals in a QDecimal (including extra decimal for rounding the final)
#define _MAX_DEC						9000000000000000000            // Maximum accepted number of a QDecimal, which is actually minus _MAX_DEC to plus _MAX_DEC
#define _INT_MUL                        100000000000            // Integer multiplier, for conversion between integer and QDecimal
#define _INT_MOD2                             100000            // Inteegr modula, used for avoiding overflow in multiplication

static const int NUM_TOT         = _NUM_TOT;
//static const int NUM_DEC         = _NUM_DEC;
static const int NUM_INT         = (_NUM_TOT-_NUM_DEC);         // Maximum number of integer digits (8)
static const QDecimal MAX_DEC    = _MAX_DEC;
static const QDecimal INT_MUL    = _INT_MUL;
static const QDecimal MAX_INT    = (_MAX_DEC/_INT_MUL);         // Maximum integer value
static const QDecimal INT_MOD2   = _INT_MOD2;
static const QDecimal INT_MUL2   = (_INT_MUL/_INT_MOD2);        // Used in multiplication for avoiding overflow
static const double DBL_MUL      = _INT_MUL;                    // Double multiplier, for converting between QDecimal and double
static const double MAX_DBL      = ((double)_MAX_DEC/_INT_MUL); // Maximum double value

static int STR_DEC_MIN           = 1;                           // Internal value for minimum number of decimal digits in string output
static int STR_DEC_MAX           = 10;                          // Internal value for maximum number of decimal digits in string output
static char STR_MINUS_SIGN       = '-';
static char STR_MIL_SIGN         = ',';
static char STR_DEC_SIGN         = '.';

static char strbuf[STRBUF_SIZE][STRDEC_SIZE];                   // String buffer for writing to string
static int strbufn               = -1;                          // Index of string buffer


static QDecimal dec_outofrange(const char *s,int ln) {
	if(s==NULL) {
		q_err_line(ERR_OUTOFRANGE,"Decimal number out of range.",ln);
	} else {
		char buf[257];
		sprintf(buf,"Decimal number out of range \"%s\".",s);
		q_err_line(ERR_OUTOFRANGE,buf,ln);
	}
	return 0;
}

const char *dec_tostr(char *buf,QDecimal d,int m) {
	int i;
	uint8_t q[NUM_TOT];
	char *p = buf;

	if(d<0) d = -d,*p++ = STR_MINUS_SIGN;

//printf("dec_tostr(%ld)\n",d);

	memset(q,0,NUM_TOT);

	// Convert decimal to int-array of 0-9 each:
	for(i=0; i<NUM_TOT && d>0; ++i,d=d/10) q[NUM_TOT-i-1] = d%10;

//printf("dec_tostr(");
//for(i=0; i<NUM_TOT; ++i) putc('0'+q[i],stdout);
//printf(")\n");

	// Round decimal:
	if(q[i=NUM_INT+STR_DEC_MAX]>=5) 
		for(q[i]=10; i>0 && q[i]==10; --i) ++q[i-1],q[i] = 0;

	// Find first positive int value in array:
	for(i=0; i<NUM_INT && q[i]==0; ++i); 

	// Write integer value:
	if(i==NUM_INT) *p++ = '0';
	else {
		if(!m || (NUM_INT-i)<3) {
			for(; i<NUM_INT; ++i) *p++ = '0'+q[i];
		} else {
			int n = i;
			for(; i<NUM_INT; ++i) {
				if(i>n && (NUM_INT-i)%3==0) *p++ = STR_MIL_SIGN;
				*p++ = '0'+q[i];
			}
		}
	}

	*p++ = STR_DEC_SIGN; // Decimal

	for(i=0; i<STR_DEC_MAX; ++i) *p++ = '0'+q[NUM_INT+i];
	while(i<STR_DEC_MIN) *p++ = '0',++i;;
	*p-- = '\0';

	for(; i>STR_DEC_MIN && p-1>buf && *p=='0' && p[-1]!=STR_DEC_SIGN; --i) *p-- = '\0'; // Remove trailing zeros
	if(*p==STR_DEC_SIGN && STR_DEC_MIN==0) *p = '\0';

	return buf;
}


QDecimal q_dec_int32(int32_t n) {
	if(n>MAX_INT) return dec_outofrange(NULL,__LINE__);
	return (QDecimal)(n*INT_MUL);
}

QDecimal q_dec_uint32(uint32_t n) {
	if(n>MAX_INT) return dec_outofrange(NULL,__LINE__);
	return (QDecimal)(n*INT_MUL);
}

QDecimal q_dec_int64(int64_t n) {
	if(n>MAX_INT) return dec_outofrange(NULL,__LINE__);
	return (QDecimal)(n*INT_MUL);
}

QDecimal q_dec_uint64(uint64_t n) {
	if(n>MAX_INT) return dec_outofrange(NULL,__LINE__);
	return (QDecimal)(n*INT_MUL);
}

QDecimal q_dec_float(double n) {
	if(n>MAX_DBL) return dec_outofrange(NULL,__LINE__);
	return (QDecimal)(n*DBL_MUL);
}

QDecimal q_dec_str(const char *n,const char **r) {
	int s = 1;
	QDecimal d = 0;

	if(n==NULL) return 0;
	if(r!=NULL) *r = n;

//fprintf(stdout,"q_dec_str(%s)\n",n);
//fflush(stdout);
	while(*n==' ') ++n;

	if(*n=='-') s = -1,++n;
	else if(*n=='+') ++n;

	if((*n<'0' || *n>'9') && *n!=STR_DEC_SIGN) return 0;

	// If string starts with '0' and not "0.":
	if(*n=='0' && n[1]!=STR_DEC_SIGN) {
		++n;
		if(*n=='x' || *n=='X') { // Hexadecimal number
			uint64_t m,c;
			int i;

//printf("q_dec_str(n=%s,l=%d,\"",n,(int)l);
			for(i=0,m=0; i<16; ++i) {
				c = q_x(n[i]);
				m = (m<<4)|c;
			}
			if(i==16) return dec_outofrange(n,__LINE__);
			n += i;
#if __BYTE_ORDER == __BIG_ENDIAN
			d = m;
#else
			d = bswap_64(m);
#endif
//printf("\")\n");
		}
	} else {
		int i,qi;
		const char *pi,*pd;
		uint8_t q[NUM_TOT];

		memset(q,0,NUM_TOT);

		for(i=0,pi=n; *pi>='0' && *pi<='9'; ++i,++pi);
//printf("q_dec_str(i=%d,NUM_INT=%d)\n",i,NUM_INT);
		if(i>NUM_INT) return dec_outofrange(n,__LINE__);
		for(pd=pi,--i,--pi,qi=NUM_INT-1; i>=0; --i,--pi,--qi) q[qi] = *pi-'0';
		if(*pd==STR_DEC_SIGN)
			for(qi=NUM_INT,++pd; qi<NUM_TOT && *pd>='0' && *pd<='9'; ++qi,++pd) q[qi] = *pd-'0';
		for(qi=0; qi<NUM_TOT; ++qi) d = d*10+q[qi];
		if(*pd=='e' || *pd=='E') { // Exponentials:
			int e = atoi(++pd);
			if(e>0 && e<=NUM_TOT) for(; e>0; --e,d*=10);
			else if(e<0 && e>=-NUM_TOT) for(; e<0; ++e,d/=10);
		}
		n = pd;
	}

	if(d>MAX_DEC) return dec_outofrange(n,__LINE__);
	if(s==-1) d = -d;

//fprintf(stdout,"%ld, %s\n",d,q_dec_tostr(d));

	if(r!=NULL) *r = n;
	return d;
}

QDecimal q_dec_inc(QDecimal d,uint64_t n) { return d+n*INT_MUL; }
QDecimal q_dec_dec(QDecimal d,uint64_t n) { return d-n*INT_MUL; }
QDecimal q_dec_add(QDecimal d,QDecimal a) { return d+a; }
QDecimal q_dec_sub(QDecimal d,QDecimal s) { return d-s; }

QDecimal q_dec_mul(QDecimal d,QDecimal m) {
	int s = 1;
	uint64_t d2,d3,m2;
	QDecimal r;

	if(d==0 || m==0) return 0;
	else if(d==INT_MUL) return m;
	else if(m==INT_MUL) return d;

	if(d<0 && m<0) d = -d,m = -m;
	else if(d<0) d = -d,s = -1;
	else if(m<0) m = -m,s = -1;

	d2 = d%INT_MUL;
	d3 = d2%INT_MOD2;
	m2 = m%INT_MUL;
	r = (d/INT_MUL)*m+(m/INT_MUL)*d2;
	d2 = d2/INT_MOD2;
	r += d2*m2/INT_MUL2;
	r += d3*m2/INT_MUL;
	r = s==-1? -r : r;
//printf("q_dec_mul(%s)\n",q_dec_tostr(r));
	return r;
}

QDecimal q_dec_div(QDecimal d,QDecimal n) {
/*	QDecimal r = 0;

	if(n==0) q_warning(ERR_ZERO_DIVISION,NULL);
	else {
		uint64_t x = 10000000000000000000llu;
		uint64_t y = n<0? -n : n;
		uint64_t z = x;

		z = x/(y/1000);
		n = n<0? -(QDecimal)z : (QDecimal)z;

		r = q_dec_mul(d,n);

printf("q_dec_div(x=%" PRIu64 ", y=%" PRIu64 ", z=%" PRIu64 ", d=%s, n=%s, r=%s)\n",x,y,z,q_dec_tostr(d),q_dec_tostr(n),q_dec_tostr(r));

	}
	return r;*/

	double d1 = q_dec_tofloat(d);
	double n1 = q_dec_tofloat(n);
	d = q_dec_float(d1/n1);
//printf("q_dec_div(%s)\n",q_dec_tostr(d));
	return d;
}

int q_dec_cmpi32(QDecimal d,int32_t n) {
	QDecimal c = n*INT_MUL;
	return d==c? 0 : (d<c? -1 : 1);
}

int q_dec_cmpu32(QDecimal d,uint32_t n) {
	QDecimal c = n*INT_MUL;
	return d==c? 0 : (d<c? -1 : 1);
}

int q_dec_cmpi64(QDecimal d,int64_t n) {
	QDecimal c = n*INT_MUL;
	return d==c? 0 : (d<c? -1 : 1);
}

int q_dec_cmpu64(QDecimal d,uint64_t n) {
	QDecimal c = n*INT_MUL;
	return d==c? 0 : (d<c? -1 : 1);
}

QDecimal q_dec_abs(QDecimal d) {
	return d<0? -d : d;
}


void q_dec_precision(int dmin,int dmax) {
	if(dmin>dmax) dmin = dmax;
	if(dmax<dmin) dmax = dmin;
	STR_DEC_MIN = dmin<0? 0 : (dmin>10? 10 : dmin);
	STR_DEC_MAX = dmax<0? 0 : (dmax>10? 10 : dmax);
}


int q_dec_toint(QDecimal d) {
	return d/INT_MUL;
}

double q_dec_tofloat(QDecimal d) {
	double n = d;
	return n/DBL_MUL;
}

const char *q_dec_tostr(QDecimal d) {
	++strbufn;
	if(strbufn==16) strbufn = 0;
	return dec_tostr(strbuf[strbufn],d,0);
}

const char *q_dec_fixed(QDecimal d,int n) {
	int d1 = STR_DEC_MIN,d2 = STR_DEC_MAX;
	const char *str;
	q_dec_precision(n,n);
	str = q_dec_tostr(d);
	STR_DEC_MIN = d1,STR_DEC_MAX = d2;
	return str;
}

const char *q_dec_tolocale(QDecimal d) {
	++strbufn;
	if(strbufn==16) strbufn = 0;
	return dec_tostr(strbuf[strbufn],d,1);
}





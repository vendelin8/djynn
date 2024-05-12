#ifndef _LIBQ_DECIMAL_H_
#define _LIBQ_DECIMAL_H_

/**
 * @file libq/decimal.h  
 * @author Per Löwgren
 * @date Modified: 2015-07-25
 * @date Created: 2014-06-23
 */ 

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include "libq.h"

enum {
	ERR_OUTOFRANGE = (__LIBQ_DECIMAL__<<16),
	ERR_ZERO_DIVISION
};

typedef int64_t QDecimal;

/*
#define q_dec_int32           q_d_i32
#define q_dec_uint32          q_d_u32
#define q_dec_int64           q_d_i64
#define q_dec_uint64          q_d_u64
#define q_dec_float           q_d_f
#define q_dec_str             q_d_s
#define q_dec_inc             q_d_inc
#define q_dec_dec             q_d_dec
#define q_dec_add             q_d_add
#define q_dec_sub             q_d_sub
#define q_dec_mul             q_d_mul
#define q_dec_div             q_d_div
#define q_dec_cmpi32          q_d_cmpi32
#define q_dec_cmpu32          q_d_cmpu32
#define q_dec_cmpi64          q_d_cmpi64
#define q_dec_cmpu64          q_d_cmpu64
#define q_dec_abs             q_d_abs
#define q_dec_precision       q_d_prc
#define q_dec_toint           q_d_toi
#define q_dec_tofloat         q_d_tof
#define q_dec_tostr           q_d_tos
#define q_dec_fixed           q_d_fix
*/

QDecimal q_dec_int32(int32_t n);     //!< Set d to n and return.
QDecimal q_dec_uint32(uint32_t n);   //!< Set d to n and return.
QDecimal q_dec_int64(int64_t n);     //!< Set d to n and return.
QDecimal q_dec_uint64(uint64_t n);   //!< Set d to n and return.
QDecimal q_dec_float(double n);      //!< Set d to n and return.
QDecimal q_dec_str(const char *n,const char **r);   //!< Set d to n and return.

QDecimal q_dec_inc(QDecimal d,uint64_t n);   //!< Increase d with n.
QDecimal q_dec_dec(QDecimal d,uint64_t n);   //!< Decrease d with n.

/** Add a to n and return.
 * @param d 
 * @param a */
QDecimal q_dec_add(QDecimal d,QDecimal a);

/** Subtract s from n and return.
 * @param d 
 * @param s */
QDecimal q_dec_sub(QDecimal d,QDecimal s);

/** Multiply n with m.
 * @param d 
 * @param m */
QDecimal q_dec_mul(QDecimal d,QDecimal m);

/** Divide d with n.
 * @param d Dividend.
 * @param n Value to divide with. */
QDecimal q_dec_div(QDecimal d,QDecimal n);

int q_dec_cmpi32(QDecimal d,int32_t n);    //!< Compare d with n. @return 0 if equal, <0 if less, and >0 if bigger.
int q_dec_cmpu32(QDecimal d,uint32_t n);   //!< Compare d with n. @return 0 if equal, <0 if less, and >0 if bigger.
int q_dec_cmpi64(QDecimal d,int64_t n);    //!< Compare d with n. @return 0 if equal, <0 if less, and >0 if bigger.
int q_dec_cmpu64(QDecimal d,uint64_t n);   //!< Compare d with n. @return 0 if equal, <0 if less, and >0 if bigger.


/** Sets the Integer sign to Plus. */
QDecimal q_dec_abs(QDecimal d);

/** Set decimal precision for converting decimal to string
 * @param dmin Minimum decimals (0-10)
 * @param dmax Maximum decimals (0-10) */
void q_dec_precision(int dmin,int dmax);

/** Convert to integer */
int q_dec_toint(QDecimal d);

/** Convert to double */
double q_dec_tofloat(QDecimal d);

/** Convert to string
 * Returns a pointer to a string containing the decimal (base 10). */
const char *q_dec_tostr(QDecimal d);

/** Convert to string with fixed number of decimals
 * Returns a pointer to a string containing the decimal (base 10).
 * @param n Number of decimals (0-10) */
const char *q_dec_fixed(QDecimal d,int n);

#endif /* _LIBQ_DECIMAL_H_ */


#ifndef _LIBQ_BASE58_H_
#define _LIBQ_BASE58_H_

/**
 * @file libq/base58.h  
 * @author Per Löwgren
 * @date Modified: 2015-07-25
 * @date Created: 2013-12-27
 */ 

/** Get length of encoded string.
 * Calculation is approximate, slightly larger than actual encoded size to account for margin.
 * @param len Length of a string
 * @return Approximate calculated length of base58-encoded string
 */
long q_base58_encoded_size(long len);

/** Get length of decoded string from encoded string-length.
 * Calculation is approximate, slightly larger than actual decoded size to account for margin.
 * @param len Length of encoded string
 * @return Approximate calculated length of decoded string
 */
long q_base58_decoded_size(long len);

char *q_base58_encode(const void *ptr,long len,long *elen);

void *q_base58_decode(const char *b58,long len,long *dlen);


#endif /* _LIBQ_BASE58_H_ */


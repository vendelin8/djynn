#ifndef _LIBQ_BASE64_H_
#define _LIBQ_BASE64_H_

/**
 * @file libq/base64.h  
 * @author Per Löwgren
 * @date Modified: 2015-07-25
 * @date Created: 2012-10-08
 */ 

long q_base64_encoded_size(long len);
long q_base64_decoded_size(long len);

char *q_base64_encode(const void *ptr,long len,long *elen);
void *q_base64_decode(const char *b64,long len,long *dlen);


#endif /* _LIBQ_BASE64_H_ */


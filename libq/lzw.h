#ifndef _LIBQ_LZW_H_
#define _LIBQ_LZW_H_

/**
 * @file libq/lzw.h  
 * @author Per Löwgren
 * @date Modified: 2015-09-11
 * @date Created: 2012-09-09
 */ 

void *q_lzw_compress(const void *ptr,long len,int hdr,long *clen);
void *q_lzw_extract(const void *ptr,long len,int hdr,long *elen);


#endif /* _LIBQ_LZW_H_ */


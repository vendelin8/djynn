#ifndef _LIBQ_RLE_H_
#define _LIBQ_RLE_H_

/**
 * @file libq/rle.h  
 * @author Per Löwgren
 * @date Modified: 2015-07-25
 * @date Created: 2012-10-08
 */ 

long q_rle_compressed_size(const void *ptr,long len);
long q_rle_extracted_size(const void *ptr,long len);

void *q_rle_compress(const void *ptr,long len,long *clen);
void *q_rle_extract(const void *ptr,long len,long *elen);


#endif /* _LIBQ_RLE_H_ */


#ifndef _LIBQ_RANDOM_H_
#define _LIBQ_RANDOM_H_

/**
 * @file libq/random.h  
 * @author Per Löwgren
 * @date Modified: 2015-07-25
 * @date Created: 2008-09-07
 */ 


#include <stdio.h>
#include <stdint.h>



/** The algorithm is inspired by mersenne twister, but differs in some ways. It is designed
 * to be fast, before generating perfect randomness, though the entropy values measured are
 * relatively good.
 * 
 * From the seed is generated a sequence of 256 integer numbers with a decent randomness and
 * place in the table. For generation of new random numbers are used two more variables; "num"
 * and "index". "Index" is an unsigned integer of the uint64_t-type that counts forward +1 for
 * every generated number. This ensures we avoid getting stuck in loops. "Num" is also an
 * unsigned integer of type uint64_t, it is always the value of the previously generated number.
 * When generating a new number we take two numbers in the table, add num to the second
 * number and xor them together. The result is placed in the address of the first number.
 * The code for this algorithm looks like this:
 * @code
table[index&0xff] ^= table[num&0xff] + num;
num = table[index&0xff];
index = index +1;
 * @endcode
 * It isn't more complicated than this, and yet it generates quite a good result compared to other
 * random number generators. A comparsion can be made here:
 * http://www.cacert.at/cgi-bin/rngresults
 * 
 * Actually, given a sufficently great enough amount of random data it seems to top the list,
 * but even a smaller amount of data places random at a good spot.
 * 
 * To ge a random number, simply call any of the given methods, depending on what type of value
 * you require. The most commonly used method is perhaps uint32(), that generates an unsigned
 * 32 bits integer. There are some methods specific for games, board games or role-playing games.
 * 
 * Here is one example of how to get a random int:
 * @code
random *r = q_rnd_new();
int i = q_rnd_int32(r);
q_rnd_free(r);
 * @endcode
 * 
 * Here's an example on how to store and restore an instance of Random:
 * @code
random *r = q_rnd_new();
FILE *fp = fopen("random.dat","wb");
q_rnd_write(fp);
fclose(fp);

... // Do romething with r.

fp = fopen("random.dat","rb");
q_rnd_read(fp,r); // Reads r to be exactly the same as when saved.
fclose(fp);
q_rnd_free(r);
 * @endcode
 * 
 * @ingroup btcx */

typedef struct _QRandom _QRandom;
typedef _QRandom *QRandom;

/** @name Constructors & destructor
 * The constructors for random.
 * @{ */
/** Constructor method. Sets seed to time(0) in time.h. */
QRandom q_rnd_new();

/** Constructor method. Sets seed to n.
 * @param n Sets seed to n. */
QRandom q_rnd_new_seed(uint64_t n);

/** Destructor method. */
void q_rnd_free(QRandom r);
/** @} */

/** @name Random engine
 * Methods to handle the enginge internally.
 * @{ */
/** Set seed and generate a new sequence of numbers.
 * @param n The seed. */
void q_rnd_seed(QRandom r,uint64_t n);

/** Set seed and generate a new sequence of numbers.
 * 
 * This method uses the entire list n as the seed.
 * @param n A list of seed numbers.
 * @param l Length of n. */
void q_rnd_seedn(QRandom r,const uint64_t n[],size_t l);

/** Set seed and generate a new sequence of numbers.
 * 
 * This method uses the entire list n as the seed.
 * @param s A string as seed. */
void q_rnd_seed_string(QRandom r,const char *s);

/** @name Random integer numbers
 * This group of methods generate various types of random integer numbers.
 * @{ */
/** Generate a random number in the range 0-0xffffffff.
 * @return A random unsigned 32 bit integer. */
uint32_t q_rnd_uint32(QRandom r);

/** Generates a number that is exactly n bits of randomness.
 * @param n
 * @return A random unsigned 32 bit integer. */
uint32_t q_rnd_uintN(QRandom r,int n);

/** @return A random signed 32 bit integer. */
int32_t q_rnd_int32(QRandom r);

/** @return A random unsigned 64 bit integer. */
uint64_t q_rnd_uint64(QRandom r);

/** @return A random signed 64 bit integer. */
int64_t q_rnd_int64(QRandom r);
/** @} */

/** @name Random real numbers
 * This group of methods generate values of the type double.
 * @{ */
/** @return A random double in the range 0.0-1.0. */
double q_rnd_real64(QRandom r);
/** @} */

/** @name Random chars
 * This group of methods generate random chars.
 * @{ */
/** @return A random char within the latin alphabet [a-zA-Z]. */
char q_rnd_alpha(QRandom r);

/** @return A random char within the latin alphabet or a digit [a-zA-Z0-9]. */
char q_rnd_alphanum(QRandom r);
/** @} */

/** @name IO
 * Methods for Input/Output.
 * @{ */
/** Write to file.
 * 
 * Write the internal data of the engine to file (it's 1036 byte or 2072 bytes depending on if it's a
 * 32 bit processor or a 64 bit).
 * @param fp File to write to. */
void q_rnd_write(FILE *fp,QRandom r);

/** Read from file.
 * 
 * Read stored data into the random engine to get an exact print of when it was stored.
 * @param fp File to read from. */
void q_rnd_read(FILE *fp,QRandom r);
/** @} */


#endif /* _LIBQ_RANDOM_H_ */


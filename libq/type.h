#ifndef _LIBQ_TYPE_H_
#define _LIBQ_TYPE_H_

/**
 * @file libq/type.h  
 * @author Per Löwgren
 * @date Modified: 2015-07-25
 * @date Created: 2008-09-07
 */ 


#include <stdint.h>


/** Enumeration used by the iteration class and subclasses
 * for extended iteration positions. */
enum {
	Q_ITER_EMPTY         = -4,  //!< Iteration points to an empty value.
	Q_ITER_START         = -3,  //!< Iteration points at the start of the Collection.
	Q_ITER_AFTER_LAST    = -2,  //!< Iteration points at one step after last value in the Collection.
	Q_ITER_BEFORE_FIRST  = -1,  //!< Iteration points at one step before first value in the Collection.
};

/** @name Type flags
 * Can be XOR:ed to any type.
 * @{ */
enum {
	Q_VALUE_NAN  = 0x20,   //!< Not a Number
	Q_VALUE_INF  = 0x40,   //!< Infinite (Generally a result of x/0)
	Q_VALUE_NEG  = 0x80,   //!< Negative
};
/** @} */

typedef struct _QType QType;
typedef struct _QArray *QArray;
typedef int64_t QDecimal;

struct _QType {
	uint8_t t;
	union {
		void *v;
		long i;
		double f;
		char *s;
		QDecimal d;
		QArray a;
	};
};

/** Is a variant, it may contain any value from the type-enumeration. */
//typedef uintptr_t QValue;

/** Contains information about how the collection should be handled. */
typedef uint32_t QStyle;

/** Is used by the Hashtable class to store a hash-values, and to get a hash-value from the Object-class. */
typedef uint32_t QHash;

#endif /* _LIBQ_TYPE_H_ */


#ifndef _LIBQ__ARRAY_H_
#define _LIBQ__ARRAY_H_

/**
 * @file libq/_array.h  
 * @author Per Löwgren
 * @date Modified: 2015-07-25
 * @date Created: 2014-05-30
 */ 

#include "array.h"
#include "decimal.h"

enum {
	NODE_FREE_KEY			= 1,
	NODE_FREE_VALUE		= 2,
};

typedef struct _QArray _QArray;
typedef struct _QArrayNode _QArrayNode;
typedef struct _QArrayNode *QArrayNode;
typedef struct _QArrayNode _QArrayIter;

struct _QArrayNode {
	QArrayNode back;           // Previous linked node
	QArrayNode next;           // Next linked node
	QHash hash;                // Hash value for hashtable
	QArrayNode table;          // Link to next node in hashtable with the same hash value % capacity
	short flags;               // Flags telling the kind of node, how it is stored
	QType key;                 // Index key, can be integer or string
	QType value;               // Value, can be QValue-type
};

struct _QArray {
	QStyle style;              // Style of array
	size_t size;               // Number of elements in array
	size_t cap;                // Capacity of table
	QArrayNode min;            // Minimum numerical key in array
	QArrayNode max;            // Maximum numerical key in array
	QArrayNode first;          // First value in linked list
	QArrayNode last;           // Last value in linked list
	QArrayNode iter;           // Iterator for array
	QArrayNode *table;         // Hashtable
};


#endif /* _LIBQ__ARRAY_H_ */


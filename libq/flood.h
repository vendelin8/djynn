#ifndef _LIBQ_FLOOD_H_
#define _LIBQ_FLOOD_H_

/**
 * @file libq/flood.h  
 * @author Per Löwgren
 * @date Modified: 2015-09-18
 * @date Created: 2008-09-07
 */ 

#include <stdint.h>
#include "geometry.h"

enum {
	Q_FLOOD_4DIRS       = 0x00,	//<! Flood fill 4 directions.
	Q_FLOOD_8DIRS       = 0x01,	//<! Flood fill 8 directions.
	Q_FLOOD_HWRAP       = 0x02,	//<! Flood fill wrap around edges horizontally.
	Q_FLOOD_VWRAP       = 0x04,	//<! Flood fill wrap around edges vertically.
};

/** Fill the vector area at location x,y with t.
 * Filling is done vertically and horizontally, and no diagonal search is done. The
 * value at x,y will be replaced by t.
 * @param area A 2D vector that should be filled. 
 * @param w Width of area.
 * @param h Height of area.
 * @param x X-Position in area to start flood-filling.
 * @param y Y-Position in area to start flood-filling.
 * @param t Value to replace with.
 * @param b Filling bounds (input and output). Can be NULL.
 * @param wr Flags for wrapping horizontally & vertically.
 * @return Number of values in area that has changed. */
int q_flood_fill8(uint8_t *area,int w,int h,int x,int y,int t,QRectangle *b,int st);


#endif /* _LIBQ_FLOOD_H_ */


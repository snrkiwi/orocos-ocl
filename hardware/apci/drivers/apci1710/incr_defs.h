/***************************************************************************
  tag: Peter Soetens  Mon Jan 10 15:59:16 CET 2005  incr_defs.h 

                        incr_defs.h -  description
                           -------------------
    begin                : Mon January 10 2005
    copyright            : (C) 2005 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be
 
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/
 
 
#ifndef APCI1710_INCR_DEFS_H
#define APCI1710_INCR_DEFS_H

/** identification of a INCR_COUNT module */
#define APCI_DEVICE_ID                  0x818f
#define APCI_INCREMENTAL_COUNTER        0x53430000

#ifdef __KERNEL__

#else

#define __KERNEL__
#include <asm/types.h>
#undef __KERNEL__

#endif
/** Various Modes */
// See technical documentation ADDICOUNT APCI-/CPCI-1710
// Incremental encoder / Pulse encoder

// quadruple mode 32 bit
// Bit 7 6 5 4 3 2 1 0
//     0 X 0 0 X 0 X 0
//
// #define APCI_INCR_QUAD32 0x00
//
// quadruple mode 16 bit
// Bit 7 6 5 4 3 2 1 0
//     0 0 0 1 0 0 0 0
//
// #define APCI1710__QUAD16 0x10

#define APCI1710_16BIT_COUNTER    0x10
#define APCI1710_32BIT_COUNTER    0x0

#define APCI1710_QUADRUPLE_MODE   0X0
#define APCI1710_DOUBLE_MODE    0x3
#define APCI1710_SIMPLE_MODE    0xf
#define APCI1710_DIRECT_MODE    0x80

#define APCI1710_HYSTERESIS_ON    0x60
#define APCI1710_HYSTERESIS_OFF   0x0

#define APCI1710_INCREMENT    0x60 // this is for 32bit mode
#define APCI1710_DECREMENT    0x0

// Interrupts

#define APCI1710_ENABLE_LATCH_INT  0x80
#define APCI1710_DISABLE_LATCH_INT  (~APCI1710_ENABLE_LATCH_INT)

// index stuff

#define APCI1710_ENABLE_INDEX   0x4
#define APCI1710_DISABLE_INDEX   (~APCI1710_ENABLE_INDEX)

#define APCI1710_ENABLE_INDEX_ACTION   0x20
#define APCI1710_DISABLE_INDEX_ACTION   (~APCI1710_ENABLE_INDEX_ACTION)

#define APCI1710_INDEX_LATCH_COUNTER  0x10
#define APCI1710_INDEX_CLEAR_COUNTER  (~APCI1710_INDEX_LATCH_COUNTER)

#define APCI1710_ENABLE_INDEX_AUTO_MODE  0x8
#define APCI1710_DISABLE_INDEX_AUTO_MODE  (~APCI1710_ENABLE_INDEX_AUTO_MODE)

#define APCI1710_ENABLE_INDEX_INT   0x1
#define APCI1710_DISABLE_INDEX_INT   (~APCI1710_ENABLE_INDEX_INT)

#endif

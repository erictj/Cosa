/**
 * @file Cosa/Types.h
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2012-2013, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * @section Description
 * Common data types and syntax abstractions.
 *
 * This file is part of the Arduino Che Cosa project.
 */

#ifndef __COSA_TYPES_H__
#define __COSA_TYPES_H__

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sfr_defs.h>
#include <util/delay_basic.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * Number of bits in a character.
 */
#define CHARBITS 8

/**
 * Standard floating point number, 32-bit.
 */
typedef float float32_t;

/**
 * Macro for number of elements in a vector.
 * @param[in] x vector
 * @return number of elements
 */
#define membersof(x) (sizeof(x)/sizeof(x[0]))

/**
 * Instruction clock cycles per micro-second. Assumes clock greater
 * or equal to 1 MHz.
 */
#define I_CPU (F_CPU / 1000000L)

/**
 * Macro for micro-second level delay. 
 * @param[in] us micro-seconds.
 */
#define DELAY(us) _delay_loop_2((us) * I_CPU / 4)

/**
 * Macro for sleep for number of seconds. Requires include of the
 * Watchdog. Allowed values are; 1, 2, 4, and 8 seconds.
 * @param[in] seconds.
 */
#define SLEEP(seconds) Watchdog::delay(seconds * 1024)

/**
 * Disable interrupts and return flags.
 * @return processor flags.
 */
inline uint8_t 
lock() 
{ 
  uint8_t key = SREG;
  cli();
  return (key);
}

/**
 * Restore processor flags and possible enable of interrupts.
 * @param[in] key processor flags.
 */
inline void 
unlock(uint8_t key)
{
  SREG = key;
}

/**
 * Syntactic sugar for synchronized block. Used in the form:
 * synchronized {
 *   ...
 *   synchronized_return(expr);
 *   ...
 *   synchronized_goto(label);
 *   ...
 * }
 * Interrupts are disabled in the block allowing secure update.
 */
#define synchronized							\
  for (uint8_t __key = lock(), i = 1; i != 0; i--, unlock(__key))
#define synchronized_return(expr)					\
  return (unlock(__key), expr)
#define synchronized_goto(label)					\
  do { unlock(__key); goto label; } while (0)

/**
 * Force compiler to store all values in memory at this point.
 * Alternative to volatile declaration.
 */
#define barrier() __asm__ __volatile__("nop" ::: "memory") 

/**
 * Buffer structure for vector write operations.
 */
struct iovec_t {
  uint8_t* buf;
  size_t size;
};

/**
 * Preprocessor tricks. Allow creating symbols.
 */
#define CONCAT(var, line) var ## line
#define MERGE(var, line) CONCAT(var, line)
#define UNIQUE(var) MERGE(var, __LINE__)

#endif


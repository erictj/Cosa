/**
 * @file Cosa/Time.hh
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2013, Mikael Patel
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
 * This file is part of the Arduino Che Cosa project.
 */

#ifndef __COSA_TIME_HH__
#define __COSA_TIME_HH__

#include "Cosa/Types.h"
#include "Cosa/BCD.h"
#include "Cosa/IOStream.hh"

/**
 * Number of seconds elapsed since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
 */
typedef uint32_t clock_t;

const uint32_t SECONDS_PER_DAY = 86400L;
const uint16_t SECONDS_PER_HOUR = 3600;
const uint16_t SECONDS_PER_MINUTE = 60;
const uint8_t DAYS_PER_WEEK = 7;

/**
 * Common date/time structure for real-time clocks. Data on devices
 * is stored in BCD (DS1307/DS3231).
 */
struct time_t {			// Range
  uint8_t seconds;		// 00-59 Seconds
  uint8_t minutes;		// 00-59 Minutes
  uint8_t hours;		// 00-23 Hours
  uint8_t day;			// 01-07 Day
  uint8_t date;			// 01-31 Date
  uint8_t month;		// 01-12 Month
  uint8_t year;			// 00-99 Year

  /**
   * Convert time to binary representation (from BCD). 
   * Apply after reading from device and before any calculation.
   */
  void to_binary()
  {
    ::to_binary(&seconds, sizeof(time_t));
  }

  /**
   * Convert time to BCD representation (from binary).
   * Apply after setting new value and writing to the device.
   */
  void to_bcd()
  {
    ::to_bcd(&seconds, sizeof(time_t));
  }

  /**
   * Default constructor.
   */
  time_t() {}

  /**
   * Construct time record (in BCD) from seconds from NTP Epoch.
   * @param[in] c clock.
   * @param[in] zone time (hours adjustment from UTC).
   */
  time_t(clock_t c, uint8_t zone = 0);

  /**
   * Convert time to clock representation (from bcd).
   * @return seconds from epoch.
   */
  clock_t to_clock();
};

/**
 * Print the date/time to the given stream with the format (YYYY-MM-DD
 * HH:MM:SS). The time structure values should be in BCD i.e. not
 * converted to binary.  
 * @param[in] outs output stream.
 * @param[in] t time structure.
 * @return iostream.
 */
IOStream& operator<<(IOStream& outs, time_t& t);

#endif


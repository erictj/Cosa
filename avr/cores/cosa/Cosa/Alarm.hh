/**
 * @file Cosa/Alarm.hh
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2014, Mikael Patel
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

#ifndef __COSA_ALARM_HH__
#define __COSA_ALARM_HH__

#include "Cosa/Types.h"
#include "Cosa/Time.hh"
#include "Cosa/Linkage.hh"
#include "Cosa/Periodic.hh"

/**
 * Cosa Alarm handler; Allows one-time or periodic activities to be
 * schedule with a seconds level resolution. Requires Watchdog with
 * timeout events and RTC for more accurate time-keeping.
 */
class Alarm : private Link {
public:
  /**
   * Construct alarm with given timeout period in seconds.
   * The alarm must be enabled to become active. 
   * @param[in] period seconds (max 65535 s, approx. 18 h).
   */
  Alarm(uint16_t period = 0) : 
    Link(),
    m_when(0L),
    m_period(period)
  {}

  /**
   * Returns current time in seconds. The current time is from startup
   * or latest set_time().
   * @return seconds.
   */
  static clock_t time()
  {
    return (s_ticks);
  }

  /**
   * Set the alarm manager clock with given time in seconds. The
   * Cosa/Time time_t data type may be used to map to calender date
   * and time. 
   * @param[in] now current time in seconds.
   */
  static void set_time(clock_t now)
  {
    s_ticks = now;
  }

  /**
   * Increment alarm manager ticks (seconds) counter and execute
   * all alarm handlers that have expired. This function must be
   * called by a RTC callback. The default scheduler is based on the
   * Watchdog and Periodic timeout events. 
   */
  static void tick();

  /**
   * Set alarm handler next timeout to the given number of seconds
   * from current time (delta). If the parameter is zero the alarm
   * handler periodic value is used as increment.
   * @param[in] seconds to alarm.
   */
  void next_alarm(uint16_t seconds = 0)
  { 
    if (seconds == 0) seconds = m_period;
    m_when = s_ticks + seconds; 
  }

  /**
   * Set alarm handler timeout to the given number of seconds from
   * current time (delta). If the parameter is zero the alarm handler
   * The alarm must be enabled to become active. 
   * periodic value is used as delta.
   * @param[in] seconds to alarm.
   */
  void set_alarm(clock_t time)
  { 
    m_when = time;
  }

  /**
   * Return alarm handler periodic timeout.
   * @return seconds.
   */
  uint16_t period()
  {
    return (m_period);
  }

  /**
   * Set alarm handler periodic timeout to the given number of
   * seconds. The alarm will be automatically rescheduled (enabled)
   * with the given period.
   * @param[in] seconds periodic timeout.
   */
  void set_period(uint16_t seconds)
  {
    m_period = seconds;
  }

  /**
   * Schedule the alarm handler. 
   */
  void enable();

  /**
   * Remove the alarm handler from the schedule. 
   */
  void disable()
  {
    detach();
  }

  /**
   * @override Alarm
   * Alarm action member function. Called when the alarm expires.
   * If the periodic timeout is non-zero the alarm is enabled again
   * otherwise a new timeout must be set and the alarm enabled.
   */
  virtual void run() = 0;

  /**
   * Watchdog based periodic(128 ms) scheduler. The setup function
   * must initiate the Watchdog to generate timeout event, 
   *   Watchdog::begin(ms, mode, Watchdog::push_timeout_events)
   * and the loop function must contain an event dispatcher,
   *   Event event;
   *   Event::queue.await(&event);
   *   event.dispatch();
   * The alarm manager will check for expired alarms and call the
   * respective alarm handlers. The RTC is used for more accurate
   * timekeeping.
   */
  class Scheduler : public Periodic {
  public:
    Scheduler() : Periodic(128), m_seconds(0L) {}
    virtual void run();
  private:
    uint32_t m_seconds;
  };
  
private:
  // Current time in seconds
  static clock_t s_ticks;

  // Alarm handler queue
  static Head s_queue;

  // Alarm timeout and period in seconds
  uint32_t m_when;
  uint16_t m_period;
};
#endif


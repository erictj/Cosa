/**
 * @file CosaThingSpeakChannel.ino
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
 * @section Description
 * ThingSpeak channel demonstration; post temperature and humidity
 * reading from DHT11 sensor to ThingSpeak server.
 *
 * This file is part of the Arduino Che Cosa project.
 */

// Uncomment to remove debug output
// #define NDEBUG

#include "Cosa/RTC.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/Driver/DHT.hh"
#include "Cosa/IoT/ThingSpeak.hh"
#include "Cosa/Socket/Driver/W5100.hh"

#ifndef NDEBUG
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#else
#define TRACE(x) x
#endif

// Digital humidity and temperature sensor
DHT11 sensor(Board::EXT0);

// Ethernet controller
static const char HOSTNAME[] __PROGMEM = "CosaThingSpeakChannel";
W5100 ethernet;

// Thingspeak client
static const char KEY[] __PROGMEM = "I3U14KLWM1R1SDPR";
ThingSpeak::Client client;

void setup()
{
#ifndef NDEBUG
  uart.begin(9600);
  trace.begin(&uart, PSTR("CosaThingSpeakChannel: started"));
#endif
  Watchdog::begin();
  RTC::begin();
  TRACE(ethernet.begin_P(HOSTNAME));
  TRACE(client.begin(ethernet.socket(Socket::TCP)));
}

void loop()
{
  ThingSpeak::Channel channel(&client, KEY);
  ThingSpeak::Entry update;
  sensor.sample();
  update.set_field(1, sensor.get_temperature(), 1);
  update.set_field(2, sensor.get_humidity(), 1);
  TRACE(channel.post(update));
  SLEEP(20);
}

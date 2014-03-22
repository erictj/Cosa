/**
 * @file CosaDNS.ino
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
 * W5100 Ethernet Controller device driver example code; DNS client.
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/Socket/Driver/W5100.hh"
#include "Cosa/INET.hh"
#include "Cosa/INET/DNS.hh"

#include "Cosa/Watchdog.hh"
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"
  
// Network configuration
#define MAC 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed
#define IP 192,168,0,100
#define SUBNET 255,255,255,0
#define SERVER 192,168,0,1
// #define SERVER 79,138,0,180
// #define SERVER 85,8,31,209

// W5100 Ethernet Controller with MAC-address
const uint8_t mac[6] __PROGMEM = { MAC };
W5100 ethernet(mac);

// Query configuration
// #define NAME PSTR("www.arduino.cc")
#define NAME PSTR("www.google.com")
// #define NAME PSTR("www.github.com")

void setup()
{
  uart.begin(9600);
  trace.begin(&uart, PSTR("CosaDNS: started"));
  Watchdog::begin();

  uint8_t ip[4] = { IP };
  uint8_t subnet[4] = { SUBNET };
  ASSERT(ethernet.begin(ip, subnet));
}

void loop()
{
  DNS dns;
  uint8_t server[4] = { SERVER };
  ASSERT(dns.begin(ethernet.socket(Socket::UDP), server));
  trace << PSTR("SERVER = ");
  INET::print_addr(trace, server, DNS::PORT);

  uint8_t host[4];
  ASSERT(dns.gethostbyname_P(NAME, host) == 0);
  trace << PSTR(":gethostbyname(") << NAME << PSTR(") = ");
  INET::print_addr(trace, host);
  trace.println();

  SLEEP(10);
}

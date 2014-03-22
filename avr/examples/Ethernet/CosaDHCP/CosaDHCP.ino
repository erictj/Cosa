/**
 * @file CosaDHCP.ino
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
 * W5100 Ethernet Controller device driver example code; DHCP client.
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/INET.hh"
#include "Cosa/INET/DHCP.hh"
#include "Cosa/Socket/Driver/W5100.hh"

#include "Cosa/Watchdog.hh"
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"
  
// Network configuration
#define MAC 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed
#define IP 0,0,0,0
#define SUBNET 255,255,255,0
static const uint8_t mac[6] __PROGMEM = { MAC };
static const char hostname[] __PROGMEM = "CosaDHCP";

// W5100 Ethernet Controller and DHCP client
W5100 ethernet(mac);
DHCP dhcp(hostname, mac);

void setup()
{
  uart.begin(9600);
  trace.begin(&uart, PSTR("CosaDHCP: started"));
  Watchdog::begin();
  ASSERT(ethernet.begin());
}

void loop()
{
  // Allocate a connection-less socket on the DHCP port (client)
  Socket* sock = ethernet.socket(Socket::UDP, DHCP::PORT);

  // Discover DHCP server and request a network address
  uint8_t ip[4], subnet[4], gateway[4];
  if (!dhcp.begin(sock)) return;
  if (dhcp.discover()) goto error;
  if (dhcp.request(ip, subnet, gateway)) goto error;
  dhcp.end();

  // Print dynamic configuration
  trace << PSTR("DHCP = "); 
  INET::print_addr(trace, dhcp.get_dhcp_addr());
  trace << endl;

  trace << PSTR("DNS = ");
  INET::print_addr(trace, dhcp.get_dns_addr());
  trace << endl;

  trace << PSTR("IP = "); 
  INET::print_addr(trace, ip); 
  trace << endl;

  trace << PSTR("GATEWAY = ");
  INET::print_addr(trace, gateway);
  trace << endl;

  trace << PSTR("SUBNET = "); 
  INET::print_addr(trace, subnet); 
  trace << endl;

  trace << PSTR("LEASE OBTAINED = ");
  trace << dhcp.get_lease_obtained();
  trace << endl;

  trace << PSTR("LEASE EXPIRES = ");
  trace << dhcp.get_lease_expires();
  trace << endl << endl;
  SLEEP(15);
  return;

 error:
  dhcp.end();
}

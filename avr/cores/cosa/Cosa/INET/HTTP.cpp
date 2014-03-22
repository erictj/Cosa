/**
 * @file Cosa/INET/HTTP.cpp
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

#include "Cosa/INET/HTTP.hh"
#include "Cosa/Watchdog.hh"
#include <ctype.h>

#define CRLF "\r\n"

bool
HTTP::Server::begin(Socket* sock)
{
  if (sock == NULL) return (false);
  m_sock = sock;
  return (!sock->listen());
}

int
HTTP::Server::request(uint32_t ms)
{
  // Wait for incoming connection requests
  uint32_t start = Watchdog::millis();
  int res;
  while (((res = m_sock->accept()) != 0) &&
	 ((ms == 0L) || (Watchdog::millis() - start < ms)))
    Watchdog::delay(16);
  if (res != 0) return (-2);

  // Wait for the HTTP request
  while ((res = m_sock->available()) == 0) Watchdog::delay(16);
  if (res < 0) goto error;

  // Read request, call handler and flush/send any buffered response
  char line[REQUEST_MAX];
  m_sock->gets(line, sizeof(line));
  on_request(line);
  m_sock->flush();
  
  // Disconnect the client and allow new connection requests
 error:
  m_sock->disconnect();
  m_sock->listen();
  return (res);
}

bool
HTTP::Server::end()
{
  if (m_sock == NULL) return (false);
  m_sock->close();
  m_sock = NULL;
  return (true);
}

bool 
HTTP::Client::begin(Socket* sock)
{
  if (sock == NULL) return (false);
  m_sock = sock;
  return (true);
}

bool 
HTTP::Client::end()
{
  if (m_sock == NULL) return (false);
  m_sock->close();
  m_sock = NULL;
  return (true);
}

int
HTTP::Client::get(const char* url, uint32_t ms)
{
  if (m_sock == NULL) return (-1);
  const uint8_t PREFIX_MAX = 7;
  uint16_t port = 80;
  char hostname[HOSTNAME_MAX];
  uint32_t start;
  uint8_t i;
  int res;
  char c;

  // Parse given url for hostname
  if (memcmp_P(url, PSTR("http://"), PREFIX_MAX) == 0) url += PREFIX_MAX;
  i = 0; 
  while (1) {
    c = *url;
    hostname[i] = c;
    if (c == 0) break;
    url += 1;
    if ((c == '/') || (c == ':')) break;
    i += 1;
    if (i == sizeof(hostname)) return (-2);
  }
  if (c != 0) hostname[i] = 0;

  // Parse url for port number
  if (c == ':') {
    char num[16];
    i = 0;
    while (isdigit(c = *url)) {
      url += 1;
      num[i++] = c;
      if (i == sizeof(num)) return (-2);
    }
    if (i == 0) return (-2);
    num[i] = 0;
    port = atoi(num);
    if (c == '/') url += 1;
  }

  // Connect to the server
  res = m_sock->connect((const char*) hostname, port);
  if (res != 0) goto error;
  while ((res = m_sock->isconnected()) == 0) Watchdog::delay(16);
  if (res == 0) res = -3;
  if (res < 0) goto error;
  
  // Send a HTTP request
  m_sock->puts_P(PSTR("GET /"));
  m_sock->puts(url);
  m_sock->puts_P(PSTR(" HTTP/1.1" CRLF "Host: "));
  m_sock->puts(hostname);
  m_sock->puts_P(PSTR(CRLF "Connection: close" CRLF CRLF));
  m_sock->flush();
  
  // Wait for the response
  start = Watchdog::millis();
  while (((res = m_sock->available()) == 0) &&
	 ((ms == 0L) || (Watchdog::millis() - start < ms)))
    Watchdog::delay(16);
  if (res == 0) res = -4;
  if (res < 0) goto error;
  on_response(hostname, url);
  res = 0;

  // Close the connect and reopen for additional get
 error:
  m_sock->disconnect();
  m_sock->close();
  m_sock->open(Socket::TCP, 0, 0);
  return (res);
}


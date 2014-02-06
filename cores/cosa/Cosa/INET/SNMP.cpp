/**
 * @file Cosa/INET/SNMP.cpp
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

#include "Cosa/INET/SNMP.hh"
#include "Cosa/Watchdog.hh"

IOStream& operator<<(IOStream& outs, SNMP::OID& oid)
{
  for (uint8_t i = 0; i < oid.length; i++) {
    if (i > 0) outs << '.';
    uint16_t lv = 0;
    uint8_t v;
    while ((v = oid.name[i]) & 0x80) {
      lv = (lv << 7) | (v & 0x7f);
      i += 1;
    }
    lv = (lv << 7) | v;
    outs << lv;
  }
  return (outs);
}

IOStream& operator<<(IOStream& outs, SNMP::PDU& pdu)
{
  outs << PSTR("dest = "); INET::print_addr(outs, pdu.dest, pdu.port); outs << endl;
  outs << PSTR("version = ") << pdu.version + 1 << endl;
  outs << PSTR("community = ") << pdu.community << endl;
  outs << PSTR("type = ") << hex << pdu.type << endl;
  outs << PSTR("request_id = ") << pdu.request_id << endl;
  outs << PSTR("error_status = ") << pdu.error_status << endl;
  outs << PSTR("error_index = ") << pdu.error_index << endl;
  outs << PSTR("oid = ") << pdu.oid << endl;
  outs << PSTR("value = ");
  outs.print(&pdu.value, pdu.value.length + 2, IOStream::hex);
  return (outs);
}

bool
SNMP::VALUE::encode(SYNTAX syn, const char* value, size_t size) 
{
  if ((syn == SYNTAX_OCTETS) || (syn == SYNTAX_OPAQUE)) {
    if (size < DATA_MAX) {
      length = size;
      syntax = syn;
      memcpy(data, value, size);
      return (true);
    }
  }
  return (false);
}

bool
SNMP::VALUE::encode(SYNTAX syn, const int16_t value) 
{
  if ((syn == SYNTAX_INT) || (syn == SYNTAX_OPAQUE)) {
    uint8_t *p = (uint8_t*) &value;
    length = sizeof(value);
    syntax = syn;
    data[0] = p[1];
    data[1] = p[0];
    return (true);
  }
  return (false);
}

bool
SNMP::VALUE::encode(SYNTAX syn, const int32_t value) 
{
  if ((syn == SYNTAX_INT32) || (syn == SYNTAX_OPAQUE)) {
    uint8_t *p = (uint8_t*) &value;
    length = sizeof(value);
    syntax = syn;
    data[0] = p[3];
    data[1] = p[2];
    data[2] = p[1];
    data[3] = p[0];
    return (true);
  }
  return (false);
}

bool
SNMP::VALUE::encode(SYNTAX syn, const uint32_t value) 
{
  if ((syn == SYNTAX_COUNTER) || 
      (syn == SYNTAX_TIME_TICKS) || 
      (syn == SYNTAX_GAUGE) || 
      (syn == SYNTAX_UINT32) || 
      (syn == SYNTAX_OPAQUE)) {
    uint8_t *p = (uint8_t*) &value;
    length = sizeof(value);
    syntax = syn;
    data[0] = p[3];
    data[1] = p[2];
    data[2] = p[1];
    data[3] = p[0];
    return (true);
  }
  return (false);
}

bool
SNMP::VALUE::encode(SYNTAX syn, const uint8_t* value)
{
  if ((syn == SYNTAX_IP_ADDRESS) || 
      (syn == SYNTAX_NSAPADDR) || 
      (syn == SYNTAX_OPAQUE)) {
    uint8_t *p = (uint8_t*) &value;
    length = sizeof(uint32_t);
    syntax = syn;
    data[0] = p[3];
    data[1] = p[2];
    data[2] = p[1];
    data[3] = p[0];
    return (true);
  }
  return (false);
}

bool
SNMP::VALUE::encode(SYNTAX syn, const bool value)
{
  if ((syn == SYNTAX_BOOL) || (syn == SYNTAX_OPAQUE)) {
    length = sizeof(uint8_t);
    syntax = syn;
    data[0] = value ? 0xff : 0x00;
    return (true);
  }
  return (false);
}

bool
SNMP::VALUE::encode(SYNTAX syn) 
{
  if ((syn == SYNTAX_NULL) || (syn == SYNTAX_OPAQUE)) {
    length = 0;
    syntax = syn;
    return (true);
  }
  return (false);
}

bool 
SNMP::read_byte(uint8_t& value)
{
  return (read(&value, sizeof(value)) == sizeof(value));
}

bool 
SNMP::read_tag(uint8_t expect, uint8_t& length)
{
  uint8_t buf[2];
  if (read(buf, sizeof(buf)) != sizeof(buf)) return (false);
  length = buf[1];
  return (buf[0] == expect);
}

bool 
SNMP::decode_null()
{
  uint8_t length;
  return (read_tag(SYNTAX_NULL, length) && (length == 0));
}

bool 
SNMP::decode_integer(int32_t& value)
{
  uint8_t length;
  value = 0L;
  if (!read_tag(SYNTAX_INT, length)) return (false);
  if (length > sizeof(value)) return (false);
  if (read(&value, length) != length) return (false);
  value = value << (32 - (length * CHARBITS));
  value = ntoh(value);
  return (true);
}

bool 
SNMP::decode_string(char* buf, size_t count)
{
  uint8_t length;
  if (!read_tag(SYNTAX_OCTETS, length)) return (false);
  if (length > count - 1) return (false);
  if (read(buf, length) != length) return (false);
  buf[length] = 0;
  return (true);
}

bool 
SNMP::decode_sequence(uint8_t& length)
{
  return (read_tag(SYNTAX_SEQUENCE, length));
}

bool 
SNMP::decode_oid(OID& oid)
{
  if (!read_tag(SYNTAX_OID, oid.length)) return (false);
  if (oid.length > OID::NAME_MAX) return (false);
  return (read(oid.name, oid.length) == oid.length);
}

bool 
SNMP::encode_null()
{
  uint8_t header[] = { SYNTAX_NULL, 0 };
  return (write(header, sizeof(header)) == sizeof(header));
}

bool 
SNMP::encode_integer(int32_t value)
{
  uint8_t header[] = { SYNTAX_INT, sizeof(value) };
  if (write(header, sizeof(header)) != sizeof(header)) return (false);
  value = hton(value);
  return (write(&value, sizeof(value)) == sizeof(value));
}

bool 
SNMP::encode_string(const char* buf)
{
  uint8_t count = strlen(buf);
  uint8_t header[] = { SYNTAX_OCTETS, count };
  if (write(header, sizeof(header)) != sizeof(header)) return (false);
  return (write(buf, count) == count);
}

bool 
SNMP::encode_sequence(int32_t count)
{
  uint8_t header[] = { SYNTAX_SEQUENCE, count };
  return (write(header, sizeof(header)) == sizeof(header));
}

bool 
SNMP::encode_oid(OID& oid)
{
  uint8_t header[] = { SYNTAX_OID, oid.length };
  if (write(header, sizeof(header)) != sizeof(header)) return (false);
  return (write(oid.name, oid.length) == oid.length);
}

bool 
SNMP::encode_pdu(uint8_t type, uint8_t size)
{
  return ((write(&type, sizeof(type)) == sizeof(type)) 
	  && (write(&size, sizeof(size)) == sizeof(size)));
}

bool 
SNMP::encode_value(VALUE& value)
{
  return (write(&value, value.length + 2) == value.length + 2);
}

bool 
SNMP::begin(Socket* sock)
{
  if (sock == NULL) return (false);
  m_sock = sock;
  return (true);
}

bool 
SNMP::end()
{
  m_sock->close();
  m_sock = NULL;
  return (true);
}

int 
SNMP::recv(PDU& pdu, uint32_t ms)
{
  uint8_t length;
  uint32_t start;
  uint8_t tag;
  int res;
  int err = -1;

  // Wait for an incoming request
  if (ms != 0L) start = Watchdog::millis();
  while (((res = m_sock->recv(&tag, sizeof(tag), pdu.dest, pdu.port)) < 0) &&
	 ((ms == 0L) || ((Watchdog::millis() - start) < ms)))
    Power::sleep(SLEEP_MODE_IDLE);
  if (res != sizeof(tag)) goto error;

  // Decode the packet and extract elements
  if (tag != SYNTAX_SEQUENCE) goto error;
  if (!read_byte(length)) goto error;
  if (!decode_integer(pdu.version)) goto error;
  if (!decode_string(pdu.community, PDU::COMMUNITY_MAX)) goto error;
  if (!read_byte(pdu.type)) goto error;
  if (!read_byte(length)) goto error;
  if (!decode_integer(pdu.request_id)) goto error;
  if (!decode_integer(pdu.error_status)) goto error;
  if (!decode_integer(pdu.error_index)) goto error;
  if (!decode_sequence(length)) goto error;
  if (!decode_sequence(length)) goto error;
  if (!decode_oid(pdu.oid)) goto error;

  // Check for value to be set (otherwise null)
  if (pdu.type == SNMP::PDU_SET) {
    length = length - (sizeof(pdu.oid.length) + pdu.oid.length + 1);
    if (length > sizeof(pdu.value)) goto error;
    if (read(&pdu.value, length) != length) goto error;
  }
  else {
    pdu.value.encode(SYNTAX_NULL);
  }
  err = 0;

  // Flush any remaining data (could be a sequence of OID:VALUE, ignored)
 error:
  uint8_t buf[32];
  while (available() > 0) read(buf, sizeof(buf));
  return (err);
}

int 
SNMP::send(PDU& pdu)
{
  uint8_t packet_size;
  uint8_t pdu_size;
  int32_t varbind_list_size;
  int32_t varbind_size;

  // Set default values
  pdu.type = SNMP::PDU_RESPONSE;
  if (pdu.value.length == 0 || pdu.error_status != NO_ERROR) {
    pdu.value.encode(SYNTAX_NULL);
  }

  // Calculate size of packet sections
  varbind_size = (pdu.value.length + 2) + (pdu.oid.length + 2);
  varbind_list_size = (varbind_size + 2);
  pdu_size = ((varbind_list_size) + 2) + (3*(sizeof(int32_t) + 2));
  packet_size = (pdu_size + 2) + (strlen(pdu.community) + 2) + (sizeof(int32_t) + 2);
  
  // Create the datagram with all encoded elements
  if (m_sock->datagram(pdu.dest, pdu.port) < 0) goto error;
  if (!encode_sequence(packet_size)) goto error;
  if (!encode_integer(pdu.version)) goto error;
  if (!encode_string(pdu.community)) goto error;
  if (!encode_pdu(pdu.type, pdu_size)) goto error;
  if (!encode_integer(pdu.request_id)) goto error;
  if (!encode_integer(pdu.error_status)) goto error;
  if (!encode_integer(pdu.error_index)) goto error;
  if (!encode_sequence(varbind_list_size)) goto error;
  if (!encode_sequence(varbind_size)) goto error;
  if (!encode_oid(pdu.oid)) goto error;
  if (!encode_value(pdu.value)) goto error;
  
 error:
  // Send the datagram
  return (m_sock->flush());
}

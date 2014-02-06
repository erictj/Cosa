/**
 * @file Cosa/INET/SNMP.hh
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

#ifndef __COSA_INET_SNMP_HH__
#define __COSA_INET_SNMP_HH__

#include "Cosa/Types.h"
#include "Cosa/Socket.hh"
#include "Cosa/IOStream.hh"

class SNMP {
public:
  // ASN.1 Basic Encoding Rules (BER) Tags
  enum {
    ASN_BER_BASE_UNIVERSAL = 0x0,
    ASN_BER_BASE_APPLICATION = 0x40,
    ASN_BER_BASE_CONTEXT = 0x80,
    ASN_BER_BASE_PUBLIC = 0xC0,
    ASN_BER_BASE_PRIMITIVE = 0x0,
    ASN_BER_BASE_CONSTRUCTOR = 0x20
  } __attribute__((packed));

  // SNMP Protocol Data Unit Operation Tags
  enum {
    PDU_GET = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 0,
    PDU_GET_NEXT = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 1,
    PDU_RESPONSE = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 2,
    PDU_SET = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 3,
    PDU_TRAP = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 4
  } __attribute__((packed));

  // SNMP Trap Tags
  enum {
    TRAP_COLD_START = 0,
    TRAP_WARM_START = 1,
    TRAP_LINK_DOWN = 2,
    TRAP_LINK_UP = 3,
    TRAP_AUTHENTICATION_FAIL = 4,
    TRAP_EGP_NEIGHBORLOSS = 5,
    TRAP_ENTERPRISE_SPECIFIC = 6
  } __attribute__((packed));

  // SNMP Value Tags (VALUE::syntax)
  enum SYNTAX {
    SYNTAX_SEQUENCE = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_CONSTRUCTOR | 0x10,
    SYNTAX_BOOL = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_PRIMITIVE | 1,
    SYNTAX_INT = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_PRIMITIVE | 2,
    SYNTAX_BITS = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_PRIMITIVE | 3,
    SYNTAX_OCTETS = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_PRIMITIVE | 4,
    SYNTAX_NULL = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_PRIMITIVE | 5,
    SYNTAX_OID = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_PRIMITIVE | 6,
    SYNTAX_INT32 = SYNTAX_INT,
    SYNTAX_IP_ADDRESS = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 0,
    SYNTAX_COUNTER = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 1,
    SYNTAX_GAUGE = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 2,
    SYNTAX_TIME_TICKS = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 3,
    SYNTAX_OPAQUE = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 4,
    SYNTAX_NSAPADDR = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 5,
    SYNTAX_COUNTER64 = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 6,
    SYNTAX_UINT32 = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 7,
  } __attribute__((packed));

  // Error codes (PDU::error_status)
  enum {
    NO_ERROR = 0,
    TOO_BIG = 1,
    NO_SUCH_NAME = 2,
    BAD_VALUE = 3,
    READ_ONLY = 4,
    GEN_ERR = 5
  };

  // Object Identity (PDU::oid)
  struct OID {
    static const size_t NAME_MAX = 32;
    uint8_t length;
    uint8_t name[NAME_MAX];
  };

  // Object Value in Basic Encoding Rule (ASN.1 BER)
  struct VALUE {
    static const uint8_t DATA_MAX = 64;
    uint8_t syntax;
    uint8_t length;
    uint8_t data[DATA_MAX];
    bool encode(SYNTAX syn, const char* value, size_t size);
    bool encode(SYNTAX syn, const int16_t value);
    bool encode(SYNTAX syn, const int32_t value);
    bool encode(SYNTAX syn, const uint32_t value);
    bool encode(SYNTAX syn, const uint8_t* value);
    bool encode(SYNTAX syn, const bool value);
    bool encode(SYNTAX syn);
  };

  // SNMP Protocol Data Unit (PDU)
  struct PDU {
    static const uint8_t COMMUNITY_MAX = 32;
    uint8_t dest[INET::IP_MAX];
    uint16_t port;
    int32_t version;
    char community[COMMUNITY_MAX];
    uint8_t type;
    int32_t request_id;
    int32_t error_status;
    int32_t error_index;
    OID oid;
    VALUE value;
  };
  
  /**
   * The SNMP Agent standard port.
   */
  static const uint16_t PORT = 161;
  
  /**
   * Start SNMP agent with the given socket (UDP::PORT). Returns true
   * if successful otherwise false.
   * @param[in] sock connection-less socket on SNMP::PORT.
   * @return bool
   */
  bool begin(Socket* sock);

  /**
   * Stop SNMP agent. Returns true if successful otherwise false.
   * @return bool
   */
  bool end();

  /**
   * Receive SNMP protocol data unit (PDU) request within given time
   * limit in milli-seconds. Returns zero and data in given PDU
   * otherwise a negative error code.
   * @param[in,out] pdu protocol unit. 
   * @param[in] ms time-out period in milli-seconds (Default BLOCK).
   * @return zero if successful otherwise a negative error code.
   */
  int recv(PDU& pdu, uint32_t ms = 0L);

  /**
   * Send SNMP protocol data unit (PDU) response. Application should
   * fill in error status and index, and value for PDU_GET on matching
   * OID. For PDU_SET the OID should be matched and the application
   * should set error status if READ_ONLY.
   * @param[in] pdu protocol unit. 
   * @return zero if successful otherwise a negative error code.
   */
  int send(PDU& pdu);

protected:
  bool read_byte(uint8_t& value);
  bool read_tag(uint8_t expect, uint8_t& length);

  bool decode_null();
  bool decode_integer(int32_t& res);
  bool decode_string(char* buf, size_t count);
  bool decode_sequence(uint8_t& length);
  bool decode_oid(OID& oid);

  bool encode_null();
  bool encode_integer(int32_t res);
  bool encode_string(const char* buf);
  bool encode_sequence(int32_t count);
  bool encode_oid(OID& oid);
  bool encode_pdu(uint8_t type, uint8_t size);
  bool encode_value(VALUE& value);

  int available() { return (m_sock->available()); }
  int read(void* buf, size_t size) { return (m_sock->read(buf, size)); }
  int write(const void* buf, size_t size) { return (m_sock->write(buf, size)); }
  int write_P(const void* buf, size_t size) { return (m_sock->write_P(buf, size)); }

  // Connection-less socket for incoming requests
  Socket* m_sock;
};

IOStream& operator<<(IOStream& outs, SNMP::OID& oid);
IOStream& operator<<(IOStream& outs, SNMP::PDU& pdu);

#endif
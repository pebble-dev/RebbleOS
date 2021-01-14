#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct __attribute__((__packed__)) {
  uint8_t byte0;
  uint8_t byte1;
  uint8_t byte2;
  uint8_t byte3;
  uint8_t byte4;
  uint8_t byte5;
  uint8_t byte6;
  uint8_t byte7;
  uint8_t byte8;
  uint8_t byte9;
  uint8_t byte10;
  uint8_t byte11;
  uint8_t byte12;
  uint8_t byte13;
  uint8_t byte14;
  uint8_t byte15;
} Uuid;

//! The minimum required length of a string used to hold a uuid (including null).
#define UUID_STRING_BUFFER_LENGTH (32 + 4 + 2 + 1)

#define UUID_SIZE 16

//! Make a Uuid object from sixteen bytes.
//! @return A Uuid structure representing the bytes p0 to p15.
#define UuidMake(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15) ((Uuid) {p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15})

//! Creates a Uuid from an array of bytes with 16 bytes in Big Endian order.
//! @return The created Uuid
#define UuidMakeFromBEBytes(b) ((Uuid) { b[0], b[1], b[2], b[3], \
                                         b[4], b[5], b[6], b[7], \
                                         b[8], b[9], b[10], b[11], \
                                         b[12], b[13], b[14], b[15] })

//! Creates a Uuid from an array of bytes with 16 bytes in Little Endian order.
//! @return The created Uuid
#define UuidMakeFromLEBytes(b) ((Uuid) { b[15], b[14], b[13], b[12], \
                                         b[11], b[10], b[9], b[8], \
                                         b[7], b[6], b[5], b[4], \
                                         b[3], b[2], b[1], b[0] })

//! Compares two UUIDs.
//! @return True if the two UUIDs are equal, false if they are not.
bool uuid_equal(const Uuid *uu1, const Uuid *uu2);

//! Writes UUID in a string form into buffer that looks like the following...
//! {12345678-1234-5678-1234-567812345678} or {NULL UUID} if NULL was passed.
//! @param uuid The Uuid to write into the buffer as human-readable string
//! @param buffer Memory to write the string to. Must be at least \ref UUID_STRING_BUFFER_LENGTH bytes long.
void uuid_to_string(const Uuid *uuid, char *buffer);

bool uuid_is_int(const Uuid *uuid, const uint8_t c);
bool uuid_null(const Uuid *uuid);

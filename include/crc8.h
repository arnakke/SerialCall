/*
 * Provides a simple CRC-8 checksum function based on
 * crcSlow by Michael Barr.
 */

#ifndef CRC8_H
#define CRC8_H

#include "inttypes.h"


uint8_t crc8(uint8_t* message, int nBytes);


#endif

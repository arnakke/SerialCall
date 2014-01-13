#include "include/crc8.h"

#define CRC8INIT  0xFF
#define CRC8POLY  0x31

/**
 * Calculate CRC-8 for a data block.
 * 
 * @message Pointer to the data that we want the checksum for.
 * @nBytes The size of the data block.
 * 
 * @returns The 8 bit CRC checksum
 */
uint8_t crc8(uint8_t* message, int nBytes) {

    // Set remainder to initial value
    uint8_t  remainder = CRC8INIT;

    // Perform modulo-2 division, a byte at a time.
    for (int byte = 0; byte < nBytes; byte++) {
      
        //Bring the next byte into the remainder.
        remainder ^= message[byte];

        // Perform modulo-2 division, a bit at a time.
        for (uint8_t bit = 8; bit; bit--) {
        
            // Divide only if leftmost bit is non-zero
            if (remainder & 0x80) {
                remainder = (remainder << 1) ^ CRC8POLY;
            } else {
                remainder = (remainder << 1);
            }
        }
    }
    // The final remainder is the CRC result.
    return remainder;
}


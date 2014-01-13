CRC8INIT =  0xFF
CRC8POLY =  0x31

def crc8(message):
    """
    Take a string and return the CRC-8.
    """
    
    # Set remainder to initial value
    remainder = CRC8INIT;
    
    # Perform modulo-2 division, a byte at a time.
    for byte in range(len(message)):
        
        # Bring the next byte into the remainder.
        remainder ^= ord(message[byte]);
        
        # Perform modulo-2 division, a bit at a time.
        for bit in range(8):
            
            # Divide only if leftmost bit is non-zero
            if remainder & 0x80:
                remainder = (remainder << 1 & 0xFF) ^ CRC8POLY;
            else:
                remainder = (remainder << 1 & 0xFF)
    
    # The final remainder is the CRC result.
    return remainder;

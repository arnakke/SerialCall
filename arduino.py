#!/usr/bin/python

"""
Implement commonly used arduino commands.
The commands will be sent to a sketch running on the arduino.
The goal is to be able to run arduino sketches through python with minimum
modification.
"""

import serial
import time

# Constants to create an arduino-like environment

# These apply to the UNO
A0 = 14
A1 = 15
A2 = 16
A3 = 17
A4 = 18
A5 = 19

LOW = INPUT = LSBFIRST = EXTERNAL = 0
HIGH = OUTPUT = MSBFIRST = DEFAULT = CHANGE = 1
FALLING = 2
RISING = INTERNAL = 3


def begin(serial_port = "", retry=10, baud=115200):
    global ser
    
    # Delete a previous instance of the serial object
    try:
      del ser
    except:
      pass
    
    if serial_port:
        ser = serial.Serial(serial_port, baud)
        return True
    else:
        # Try to open a connection on various serial ports
        # the solution is specific to certain linux distros
        for dev_num in range(8):
            serial_port = '/dev/ttyACM' + str(dev_num)
            try:
                ser = serial.Serial(serial_port, baud)
                print 'Connected to ' + serial_port
                return True
            except:
                pass
        serial_port = ""
    
    # We still don't have a serial connection
    # try once a second for retry seconds
    if retry > 0:
      print "Waiting for device"
      for i in range(retry):
        time.sleep(1)
        if begin(serial_port, -1, baud):
          return True
      
      print "No device found. Not connected."


def pinMode(pin, mode):
    ser.write(chr(1)+chr(pin*2+mode))

def digitalWrite(pin, value):
    ser.write(chr(2)+chr(pin*2+value))

def analogWrite(pin, value):
    ser.write(chr(3)+chr(pin)+chr(value))

def digitalRead(pin):
    ser.write(chr(4)+chr(pin))
    return ord(ser.read())

def analogRead(pin):
    ser.write(chr(5)+chr(pin))
    return ord(ser.read()) + ord(ser.read())*256

def setTimer1freq(setting):
    ser.write(chr(6)+chr(setting))

def setTimer2freq(setting):
    ser.write(chr(7)+chr(setting))

def analogReference(setting):
    ser.write(chr(8)+chr(setting))

def tone(pin, frequency):
    ser.write(chr(9)+chr(pin)+chr(frequency >> 8)
              +chr(frequency & 255))

def noTone(pin):
    ser.write(chr(10)+chr(pin))

def shiftOut(dataPin, clockPin, bitOrder, value):
    clockPin_bitOrder = clockPin | (bitOrder << 7)
    ser.write(chr(11)+chr(dataPin)+chr(clockPin_bitOrder)+
              chr(value))

def shiftIn(dataPin, clockPin, bitOrder):
    clockPin_bitOrder = clockPin | (bitOrder << 7)
    ser.write(chr(12)+chr(dataPin)+chr(clockPin_bitOrder))
    return ord(ser.read())

def invertTimer2B():
    ser.write(chr(13))

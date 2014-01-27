#!/usr/bin/python

"""
Implement commonly used arduino commands.
The commands will be sent to a sketch running on the arduino.
Goals:
* be able to run arduino sketches through python with minimum
modification
* try things out in an interactive environment (the shell)
* be able to set all registers in a manner that resembles how it is done
in the arduino environment
* debug: access global variables
* debug: call own functions
* 
"""

import serial
import time
import struct
import re
import os
from parse_constants import *
from crc8 import *

format_code_to_types = {
    'B': ["unsigned char", "uint8_t", "byte", "boolean"],
    'b': ["char", "signed char", "int8_t"],
    '?': ["bool"],
    'H': ["unsigned int", "uint16_t", "size_t", "void*"],
    'h': ["int", "signed int", "int16_t"],
    'Q': ["unsigned long long int", "unsigned long long", "uint64_t"],
    'q': ["long long int", "long long", "int64_t"],
    'I': ["unsigned long int", "unsigned long", "uint32_t"],
    'i': ["long int", "long", "signed long int", "signed long", "int32_t"],
    'f': ["float", "double"]
}

c_type_to_format_code = {T: f for
                         f, Ts in format_code_to_types.iteritems() for
                         T in Ts}

types = [T for f, Ts in format_code_to_types.iteritems() for T in Ts]

format_code_to_bytes = {'B': 1, 'b': 1, '?': 1, 'h': 2, 'H': 2,
                        'i': 4, 'I': 4, 'q': 8, 'Q': 8, 'f': 4}

#_registers = __registers

# Constants to create an arduino-like environment

# These apply to the UNO
#~ A0 = 14
#~ A1 = 15
#~ A2 = 16
#~ A3 = 17
#~ A4 = 18
#~ A5 = 19
#~ 
#~ LOW = INPUT = LSBFIRST = EXTERNAL = 0
#~ HIGH = OUTPUT = MSBFIRST = DEFAULT = CHANGE = 1
#~ FALLING = 2
#~ RISING = INTERNAL = 3

class CB:
    pass

class Arduino(object):
    functions = {}
    active_cb = ''
    id = 255;
    
    def verify_conn(self):
        #return True
        retry = 8
        while retry:
            try:
                self.id = self.getDevId()
                return True
            except:
                retry -= 1
        
        print "error, device didn't return an id"
        return False
    
    def import_const(self):
        print "Importing constants from io header file"
        io_h = self.getDevType()
        print "Got filename from device: ", io_h
        path = ["/usr/lib/",
                os.environ.get("ProgramFiles","")+"/arduino/hardware/tools/avr"]
        for p in path:
            f = p + "/avr/include/avr/" + io_h
            if os.path.isfile(f):
                print "Importing from ", f
                parse_h(f)
                return
        print "Error, io header file not found"          
        
    
    def open_conn(self, port):
        self.ser = serial.Serial(port, self.baud, timeout=0.5)
    
    def auto_connect(self):
        # Try to open a connection on various serial ports
        for port in get_ports():
            try:
                print "trying ", port
                self.open_conn(port[0])
                if self.verify_conn():
                    print "verified ", port
                    break
            except:
                pass
    
    
    def __init__(self, serial_port = "", baud=115200, load_def_cmds = True):
        self.baud = baud
        
        if load_def_cmds:
            self.load_def_cmds()
        
        if serial_port:
            self.open_conn(serial_port)
        else:
            self.auto_connect()
        
        
        if self.verify_conn():
            print "Device has id ", self.id
            self.import_const()
        
        # Print connected port etc.
        print "Connected to", self.ser.portstr
    
    
    def add(self, proto, cmd_id):
        CNAME = r"\b[a-zA-Z_][\w]*\b"
        #CTYPE = r"(?:const\s+)?(?:(?:un)?signed\s+)?(?:char|int|long long|long)"
        #TYPES = r"(?:{CTYPE}|float|double|void|size_t)\*?"
        regextypes = [t.replace("*",r"\*").replace(" ",r"\s+") for t in types]
        TYPE = "|".join(regextypes)
        value = r"-?\d+\.\d*|-?\d*\.\d+|-?\d+|true|false"

        f = {"cname":CNAME, "type":TYPE, "value":value}



        cfunction = r".*?\b({type}|void)\s+({cname})\s*\((.*?)\)".format(**f)
        functionarg = r"\s*(?:const\s+)?({type})(?:\s+({cname}))?(?:\s*=\s*({value}))?".format(**f)
        
        
        m = re.match(cfunction, proto)
        if m:
            cb = CB()
            cb.id = cmd_id
            ret = m.groups()[0]
            if ret == 'void':
                cb.ret = ''
            else:
                cb.ret = c_type_to_format_code[ret]
            name = m.groups()[1]
            args = m.groups()[2].split(',')
            cb.args = []
            if args[0]:
                for arg in args:
                    m = re.match(functionarg, arg)
                    cb.args.append(c_type_to_format_code[m.groups()[0]])
                    #cb.args = [c_type_to_format_code[arg.strip()] for arg in args]
            
            self.functions[name] = cb
    
    
    def call_callback(self, *args):
        cb = self.functions[self.active_cb]
        
        # Start the package with the cmd id
        package = chr(cb.id)
        
        # Add each of the callback args
        for i in range(len(cb.args)):
            form = '<' + cb.args[i]
            package += struct.pack(form, args[i])
        
        # Append the CRC-8 checksum
        package += chr( crc8(package) )
        
        # Send the package
        self.ser.write(package)
        
        # Get the return data (if relevant)
        if cb.ret:
            form = '<' + cb.ret
            size = format_code_to_bytes[cb.ret]
            return struct.unpack(form, self.ser.read(size))[0]


    def load_def_cmds(self):
        self.add("void pinMode(uint8_t, uint8_t);", 2)
        self.add("void digitalWrite(uint8_t, uint8_t);", 3)
        self.add("int digitalRead(uint8_t);", 5)
        self.add("int analogRead(uint8_t);", 6)
        self.add("void analogReference(uint8_t);", 7)
        self.add("void analogWrite(uint8_t, int);", 4)
        self.add("void tone(uint8_t, uint16_t, uint32_t)", 14)
        self.add("void noTone(uint8_t)", 15);
        self.add("void shiftOut(uint8_t, uint8_t, uint8_t, uint8_t)", 16);
        self.add("uint8_t shiftIn(uint8_t, uint8_t, uint8_t)", 17);
        
        self.add("uint8_t getByte(void*);", 0);
        self.add("void setByte(void*, uint8_t);", 1);
        self.add("uint16_t get2Bytes(void*);", 8);
        self.add("uint32_t get4Bytes(void*);", 9);
        self.add("void set2Bytes(void*, uint16_t);", 10);
        self.add("void set4Bytes(void*, uint32_t);", 11);
        
        self.add("void* getDevTypeAddr()", 12)
        self.add("uint8_t getDevId()", 13)
    
    
    def __getattr__(self, name):
        if name in self.functions:
            self.active_cb = name
            return self.call_callback
        
        if name in registers:
            return self.getByte(registers[name])
    

    def __setattr__(self, name, value):
        if name in registers:
            return self.setByte(registers[name], value)
        else:
            self.__dict__[name] = value
    
    def getDevType(self):
        addr = self.getDevTypeAddr()
        i = 0
        devType = ""
        while 1:
            char = chr(self.getByte(addr+i))
            if char == "\0":
                break
            devType += char
            i += 1
        return devType
            


def get_ports():
   """
   Return a list of available serial ports.
   Should work on most major operating systems.
   """
   
   available = []
   for i in range(256):
       try:
           s = serial.Serial(i)
           available.append( (i, s.portstr))
           s.close()
       except serial.SerialException:
           pass
   import glob
   usbttys = glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*")
   for i in usbttys:
       try:
           s = serial.Serial(i)
           available.append( (i, s.portstr))
           s.close()
       except serial.SerialException:
           pass
   return available

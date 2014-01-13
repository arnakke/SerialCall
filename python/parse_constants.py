import re
import __main__

registers = {}

def parse_h(filename):
  file = open(filename)
  for line in file:
    m = re.match(r"#define\s+(\w+)\s+(\d)+\b", line)
    if m:
      setattr(__main__, m.groups()[0], int(m.groups()[1]))
      #print m.groups()[0], m.groups()[1]
      
    m = re.match(r"#define\s+(\w+)\s+_SFR_(MEM|IO)8\((0x[0-9A-F]{2})\)", line)
    if m:
      addr = int(m.groups()[2], 16)
      # IO address space has offset of 0x20
      if m.groups()[1] == 'IO':
        addr += 0x20
      setattr(__main__, m.groups()[0], addr)
      registers[m.groups()[0]] = addr
      #print m.groups()[0], addr

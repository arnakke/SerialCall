#include "SerialCall.h"

/**
 * Create an instance of the SerialCall class.
 * 
 * @Stream Stream instance connected to the remote caller(s)
 * @id Device id used to identify this device
 */
SerialCall::SerialCall(Stream &serial_dev, uint8_t id, bool load_default_cmd_set) :
  serial(serial_dev),
  timeout(200),
  cmd_count(0),
  dev_id(id)
{
  
  // Zero initalize handlers array
  for (uint8_t i = 0; i < MAX_COMMANDS; i++) {
    handlers[i] = NULL;
  }
  
  if (load_default_cmd_set) {
    load_def_cmd_set();
  }
}

/**
 * Add a callback to the SerialCall instance.
 * 
 * @handlerf The hander function that calls the actual callback
 * @args_sz The sum of size of the arguments to the callback.
 * This is the amount of data that process_call will receive before
 * calling the handler.
 * @id The callback id that the will trigger this handler
 */
void SerialCall::add(CmdHandler handlerf, ArgsSz args_sz, CmdId id) {
  // Avoid overwriting manually id'ed handlers if auto-id'ing
  while (handlers[cmd_count] != NULL) {
    cmd_count++;
  }
  if (id == AUTO_ID) {
    id = cmd_count;
  }
  
  //serial.print("add argsz"); serial.print(args_sz);
  //serial.print("id "); serial.println(id);
  
  // Add our command handler to the array
  handlers[id] = handlerf;
  args_szs[id] = args_sz;
}

/**
 * Process incoming data on the Stream instance.
 * This function has to be called in a main loop, by a timer,
 * or by an interrupt triggered by incoming data.
 */
void SerialCall::process_call() {
  unsigned long start = millis();
  
  // Read id of the called command
  // -1 means that there is no data to read
  cmd_id = serial.read();
  
  // Only continue if command was read and is defined
  if (cmd_id > -1 && handlers[cmd_id]) {
    #ifdef debug
    serial.print("exec "); serial.println(cmd_id);
    serial.print("argssz "); serial.println(args_szs[cmd_id]);
    serial.flush();
    #endif
    
    // Get the args    
    for (uint8_t i = 0; i < args_szs[cmd_id]; i++) {
      // Return if the args are not received before timeout
      int temp;
      while ((temp = serial.read()) < 0) {
        if (millis() - start >= timeout) return;
      }
      args_buf[i] = temp;
      #ifdef debug
      serial.print("got arg "); serial.println((char)args_buf[i]);
      serial.flush();
      #endif
    }
    
    // Set argument index to zero
    args_i = args_szs[cmd_id]-1;
    
    #ifdef debug
    serial.print("haddr "); serial.println((uint16_t)handlers[cmd_id]);
    serial.print("faddr "); serial.println((uint16_t)org_funcs[cmd_id]);
    serial.flush();
    #endif
    // Execute the handler
    handlers[cmd_id](this, org_funcs[cmd_id]);
  }
}


/**
 * Return data.
 * 
 * @data Address of the data that should be returned
 * @bytes How many bytes to send.
 */
void SerialCall::return_data(void *data, uint8_t bytes) {
  #ifdef debug
  serial.print("returning: "); serial.println(bytes);
  serial.flush();
  #endif
  for (uint8_t i = 0; i < bytes; i++) {
    serial.write(((uint8_t*)data)[i]);
  }
}


/**
 * Add default callback functions.
 */
void SerialCall::load_def_cmd_set() {
  add(pinMode, 2);
  add(digitalWrite, 3);
  add(analogWrite, 4);
  add(digitalRead, 5);
  add(analogRead, 6);
  add(analogReference, 7);
  
  add(getByte, 0);
  add(setByte, 1);
  add(get2Bytes, 8);
  add(get4Bytes, 9);
  add(set2Bytes, 10);
  add(set4Bytes, 11);
  
  add(tone, 14);
  add(noTone, 15);
  add(shiftOut, 16);
  add(shiftIn, 17);
  
  add(getDevTypeAddr, 12);
  add(getDevId, 0, 13);
}


// Default callback functions for low level data access

uint8_t getByte(void* addr) {
  return *(uint8_t*)addr;
}

uint16_t get2Bytes(void* addr) {
  return *(uint16_t*)addr;
}

uint32_t get4Bytes(void* addr) {
  return *(uint32_t*)addr;
}

void setByte(void* addr, uint8_t data) {
  *(uint8_t*)addr = data;
}

void set2Bytes(void* addr, uint16_t data) {
  *(uint16_t*)addr = data;
}

void set4Bytes(void* addr, uint32_t data) {
  *(uint32_t*)addr = data;
}


// Default callback functions to get some information about the device
char devType[] = _AVR_IOXXX_H_;

void* getDevTypeAddr() {
  return devType;
}

void getDevId(SerialCall* p) {
  p->serial.write(p->dev_id);
}

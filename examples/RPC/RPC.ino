/*
 * Example of how to use the remote procedure call capabilities
 */


#include "SerialCall.h"

// Create a SerCall instance tied to the default hardware serial
SerialCall ser_call(Serial);

// A function that we want to be able to call remotely
float get_voltage(uint8_t pin) {
  return analogRead(pin) * 5.0 / 1023;
}

// Another function we want to call remotely
int somedata[3];
void set_data(int a, int b, int c) {
  somedata[0] = a;
  somedata[1] = b;
  somedata[2] = c;
}

void setup() {
  Serial.begin(115200);
  
  // Export get_voltage with the id 20
  // Most callback id's below 20 are used for arduino or low level functions
  ser_call.add(get_voltage, 15);
  
  // Export set_data with the id 21
  ser_call.add(set_data, 21);
}


void loop() {
  // See if there is any serial data to process
  ser_call.process_call();
}

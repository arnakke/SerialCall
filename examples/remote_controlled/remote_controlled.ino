#include "SerialCall.h"


// Create a SerialCall instance tied to the default hardware serial
// It will export common arduino functions along with some low
// level memory access functions.
SerialCall ser_call(Serial);

  
void setup() {
  // Initialize the serial interface
  Serial.begin(115200);
}


void loop() {
  // See if there is any serial data to process
  ser_call.process_call();
}

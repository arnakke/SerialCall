#ifndef SERIALCONTROLLED_H
#define SERIALCONTROLLED_H

#include "Arduino.h"
#include "include/handler_templates.h"
#include "include/crc8.h"

//#define debug

// The max number of callback functions that can be attached to a
// SerialCall instance. Larger number will eat more SRAM.
#define MAX_COMMANDS 30

// The size of the args_buf for storing arguments from the remote caller
#define MAX_ARGS_SZ 17

// The type for storing the return data from the from callback functions.
// A larger type means that callback functions can have larger return types,
// but at the cost of larger handler functions
typedef uint32_t RetData;

#define AUTO_ID 255


/**
 * Class to create remote procedure call functionality.
 * Each instance can have a number of callback functions attached.
 * Each instance is tied to a Stream instance that works as an
 * abstraction layer between the SerialCall instance and communication
 * hardware, such as a UART.
 */
class SerialCall {  
  public:
    // Pointer type of the handler functions that work as bridges between
    // the SerialCall class and the actual callback functions.
    typedef void (*CmdHandler)(HANDLER_ARGS);
    
    // The type used for the callback id's
    typedef unsigned char CmdId;
    typedef unsigned char ArgsSz;
    
    
    // Initializer
    SerialCall(Stream&, uint8_t id = 0, bool = true);
    
    
    // Data
    CmdHandler handlers[MAX_COMMANDS];  
    ArgsSz args_szs[MAX_COMMANDS];
    void (*org_funcs[MAX_COMMANDS])();
    
    Stream &serial;
    
    CmdId cmd_count;
    int cmd_id;
    unsigned int timeout;
    // This is used for checksum calculation and must be placed just
    // above args_buf
    uint8_t cmd_id_chk;
    uint8_t args_buf[MAX_ARGS_SZ];
    uint8_t args_i;
    uint8_t dev_id;
    
    
    // Functions
    void process_call();
    void return_data(void *data, uint8_t bytes);
    void load_def_cmd_set();
    
    
    // Function to add callback functions to the instance
    void add(CmdHandler, ArgsSz, CmdId = AUTO_ID);
    
    // This function is to add custom handler functions with a signature
    // like this: void my_handler(SerialCall*)
    inline void add(void (*func)(SerialCall*), ArgsSz args_sz, CmdId id = AUTO_ID) {
      add((CmdHandler)func, args_sz, id);
    }
    
    // Template function to add callbacks to the instance.
    // Will automatically create handlers for arbitrary
    // callback functions (limitations apply).
    
    /* The variadic template version is out for now,
     * until arduino decides to update the gcc version
     * shipped with the ide.
    template <typename R, typename ... Args>
    void add(R (*func) (Args ...), uint8_t id) {
      add(handle<R,Args...>, sizeof_args(func), id); 
      org_funcs[id] = (void (*)()) func;  
    }*/
    
    template <typename R>
    void add(R (*func) (), uint8_t id) {
      add(handle<R>, 0, id); 
      org_funcs[id] = (void (*)()) func;  
    }
    template <typename R, typename A1>
    void add(R (*func) (A1), uint8_t id) {
      add(handle<R,A1>, sizeof(A1), id); 
      org_funcs[id] = (void (*)()) func;  
    }
    template <typename R, typename A1, typename A2>
    void add(R (*func) (A1, A2), uint8_t id) {
      add(handle<R,A1,A2>, sizeof(A1)+sizeof(A2), id); 
      org_funcs[id] = (void (*)()) func;  
    }
    template <typename R, typename A1, typename A2, typename A3>
    void add(R (*func) (A1, A2, A3), uint8_t id) {
      add(handle<R,A1,A2,A3>, sizeof(A1)+sizeof(A2)+sizeof(A3), id); 
      org_funcs[id] = (void (*)()) func;  
    }
    template <typename R, typename A1, typename A2, typename A3, typename A4>
    void add(R (*func) (A1, A2, A3, A4), uint8_t id) {
      add(handle<R,A1,A2,A3,A4>, sizeof(A1)+sizeof(A2)+sizeof(A3)+sizeof(A4), id); 
      org_funcs[id] = (void (*)()) func;  
    }
    
    /**
     * Get an argument from args_buf.
     * Use: get_arg<type>()
     * To be used inside custom handler function to pass arguments to
     * the callback. Example:
     * void my_custom_handler(SerialCall* p) {
     *   my_callback(p->get_arg<float>(), p->get_arg<MyType>());
     * }
     * Note that when you call get_arg the first argument returned is
     * the rightmost argument of the callback. This is due to the calling
     * convention of passing the rightmost argument first.
     */
    template <typename T>
    T get_arg() {
      T arg = *(T*) &args_buf[args_i+1-sizeof(arg)];
      args_i -= sizeof(arg);
      return arg;
    }
    
};

// This must be included here as the templates have to be availbale
// in their entirety
#include "include/handler_templates.cpp"


uint8_t getByte(void* addr);
uint16_t get2Bytes(void* addr);
uint32_t get4Bytes(void* addr);
void setByte(void* addr, uint8_t data);
void set2Bytes(void* addr, uint16_t data);
void set4Bytes(void* addr, uint32_t data);


void* getDevTypeAddr();
void getDevId(SerialCall* p);


#endif

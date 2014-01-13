#include "SerialCall.h"

// Helpers for the GET_ARGS macro
#define ARGS_()
#define ARGS_A1(A1) *(A1*)p->args_buf
#define ARGS_1_OR_0(A1) ARGS_##A1(A1) // call appropriate macro for 0 or 1 arg
#define ARGS_2(A1, A2) *(A1*)p->args_buf, *(A2*)&p->args_buf[sizeof(A1)]
#define ARGS_3(A1, A2, A3) *(A1*)p->args_buf, *(A2*)&p->args_buf[sizeof(A1)], *(A3*)&p->args_buf[sizeof(A1)+sizeof(A2)]
#define ARGS_4(A1, A2, A3, A4) *(A1*)p->args_buf, *(A2*)&p->args_buf[sizeof(A1)], *(A3*)&p->args_buf[sizeof(A1)+sizeof(A2)], *(A4*)&p->args_buf[sizeof(A1)+sizeof(A2)+sizeof(A3)]
#define GET_MACRO(_1,_2,_3,_4,NAME,...) NAME

/**
 * Generate code to retrieve arguments received from the remote caller
 * stored in the arg_buf of the SerialCall instance.
 * 
 * @... List of the types that callback function accepts
 */
#define GET_ARGS(...) GET_MACRO(__VA_ARGS__, ARGS_4, ARGS_3, ARGS_2, ARGS_1_OR_0)(__VA_ARGS__)


// This is the template code for handlers to functions with n arguments.
// It is put in this macro so that it can be edited in one place.
// Lines must end with a "\"
// ... / __VA_ARGS__ is a list of the types of the callback function
#define handler_code(...)                                        \
/* Function pointer for our callback  */  \
RetData (*callback)(__VA_ARGS__);             \
\
/* Typecast the stored function ptr */   \
callback = (RetData (*)(__VA_ARGS__)) org_func;                  \
\
/* Call our callback function.
 * Store the return data (if any) in the variable data.
 * The size of data can be set in SerialCall.h
 * Return types larger than data likely won't work.
 * This relies on the avr-gcc ABI.
 */ \
RetData data = callback(GET_ARGS(__VA_ARGS__)); \
                                                                      \
/* Return data if the callback return type is not void */ \
if (!is_void(R)) {                                                    \
  /* Calculate the pointer to the actual return data.
   * The offset calculation is based on the avr-gcc calling convention
   * as specified in the ABI. See http://gcc.gnu.org/wiki/avr-gcc */ \
  uint8_t* offset = (uint8_t*)&data + (sizeof(data)-sizeof(R)-sizeof(R)%2);     \
  p->return_data(offset, sizeof(R));                                  \
}



// These templates implement the handlers with 0 to 4 arguments
template <typename R>
HANDLER_PROTO {handler_code()}

template <typename R, typename A1>
HANDLER_PROTO {handler_code(A1)}

template <typename R, typename A1, typename A2>
HANDLER_PROTO {handler_code(A1,A2)}

template <typename R, typename A1, typename A2, typename A3>
HANDLER_PROTO {handler_code(A1,A2,A3)}

template <typename R, typename A1, typename A2, typename A3, typename A4>
HANDLER_PROTO {handler_code(A1,A2,A3, A4)}

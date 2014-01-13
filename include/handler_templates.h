#ifndef HANDLER_TEMPLATES_H
#define HANDLER_TEMPLATES_H

// Forward declaration of SerialCall so the handler prototypes will accept
// the pointer
class SerialCall;

// The args / prototype are defined in these macros so that it can be
// edited in just one place
#define HANDLER_ARGS SerialCall*, void (*)()
#define HANDLER_PROTO void handle(SerialCall* p, void (*org_func)())


// Templates for functions with zero to n args
template <typename R>
HANDLER_PROTO;

template <typename R, typename A1>
HANDLER_PROTO;

template <typename R, typename A1, typename A2>
HANDLER_PROTO;

template <typename R, typename A1, typename A2, typename A3>
HANDLER_PROTO;

template <typename R, typename A1, typename A2, typename A3, typename A4>
HANDLER_PROTO;

#endif

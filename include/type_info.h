/**
 * Check if two types are the same.
 * Usage: is_same<type1, type2>::value
 */
template <typename, typename>
struct is_same { static const bool value = false;};
template <typename T>
struct is_same<T,T> { static const bool value = true;};


/**
 * Check if a type is void.
 * Useful macro for templated functions that accept a function pointer,
 * to check if the return type is void.
 * 
 * @T The type that should be checked if it is void.
 */
#define is_void(T) (is_same<T,void>::value)


/**
 * Returns the sum of the size of the args to a function in bytes.
 * Equivalent to sizeof(arg1) + sizeof(arg2) + ... + sizeof(argN)
 * 
 * @func Name of function or function pointer
 */
template <typename R>
uint8_t sizeof_args( R (*func)()) {
  // No more args
  return 0;
}

template <typename R, typename A1, typename ... Args>
uint8_t sizeof_args( R (*func)(A1, Args ...) ) {
  // "Peel" off one argument at a time by recursively calling itself
  // with a NULL function ptr with all args but the first
  uint8_t sz_remaining_args = sizeof_args( (R (*) (Args...)) 0);
  return sizeof(A1) + sz_remaining_args;
}

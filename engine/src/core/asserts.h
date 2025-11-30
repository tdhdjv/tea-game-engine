#pragma once

#include "../defines.h"

#define TASSERTION_ENABLED 1

#if TASSERTION_ENABLED == 1
  #if _MSC_VER
    #include <intrin.h>
    #define debug_break() __debugbreak()
  #else
    #define debug_break() __builtin_trap()
  #endif

  void assert_failure_report(const char *expression, const char *message, const char *file, i32 line);

  #define TASSERT(expr) {                                    \
    if(!(expr)) {                                          \
      assert_failure_report(#expr, "", __FILE__, __LINE__);\
      debug_break();                                       \
    }                                                      \
  }
  
  #define TASSERT_MSG(expr, message) {                            \
    if(!(expr)) {                                               \
      assert_failure_report(#expr, message, __FILE__, __LINE__);\
      debug_break();                                            \
    }                                                           \
  }

  #ifdef _DEBUG
  #define TASSERT_DEBUG(expr, message) {                                  \
      if(!(expr)) {                                          \
        assert_failure_report(#expr, "", __FILE__, __LINE__);\
        debug_break();                                       \
      }                                                      \
    }
  #else
  #define TASSERT_DEBUG(expr)
  #endif
#else

  #define TASSERT(expr) if(!(expr)) return false;
  #define TASSERT_MSG(expr, message) if(!(expr)) return false;
  #define TASSERT_DEBUG(expr) if(!(expr)) return false;

#endif

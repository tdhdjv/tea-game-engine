#pragma once

#include "../defines.h"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

//Disable debug and trace logging for release build
#if TRELEASE == 1
  #define LOG_DEBUG_ENABLED 0
  #define LOG_TRACE_ENEABLED 0 
#endif

typedef enum logLevel {
  LOG_LEVEL_FATAL = 0,
  LOG_LEVEL_ERROR = 1,
  LOG_LEVEL_WARN = 2,
  LOG_LEVEL_INFO = 3,
  LOG_LEVEL_DEBUG = 4,
  LOG_LEVEL_TRACE = 5
} logLevel;

b8 log_init();
void log_shutdown();

TAPI void log_output(logLevel level, const char* message, ...);

#define TFATAL(message, ...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__)
#define TERROR(message, ...) log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__)
#if LOG_WARN_ENABLED == 1
  #define TWARN(message, ...) log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__)
#else
  #define TWARN(mesage, ...)
#endif
#if LOG_INFO_ENABLED == 1
  #define TINFO(message, ...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__)
#else
  #define TINFO(mesage, ...)
#endif
#if LOG_DEBUG_ENABLED == 1
  #define TDEBUG(message, ...) log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__)
#else
  #define TDEBUG(mesage, ...)
#endif
#if LOG_TRACE_ENABLED == 1
  #define TTRACE(message, ...) log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__)
#else
  #define TTRACE(mesage, ...)
#endif

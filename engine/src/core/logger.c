#include "logger.h"
#include "../platform/platform.h"
#include "asserts.h"

//TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

b8 log_init() {
  // TODO: create info file.
  return true;
}

void log_shutdown() {
  // TODO: close log info files/clean up
}

void log_output(LogLevel level, const char* message, ...) {
  const char* levelStrings[6] = {"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"};
  b8 isError = level < LOG_LEVEL_WARN;

  //There is a MESSAGE limit
  const i32 logLength = 32000;
  char outMessage[logLength];
  memset(outMessage, 0, sizeof(outMessage));

  // Format original message.
  // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
  // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
  // which is the type GCC/Clang's va_start expects. 
  __builtin_va_list arg_ptr;
  va_start(arg_ptr, message);
  vsnprintf(outMessage, 32000, message, arg_ptr);
  va_end(arg_ptr);

  char outMessage2[logLength];
  sprintf(outMessage2, "[%s]: %s\n", levelStrings[level], outMessage);

  // TODO: platform-specific output.
  if(isError) {
    platform_console_write_error(outMessage2, level);
  } else {
    platform_console_write(outMessage2, level);
  }
}

void assert_failure_report(const char *expression, const char *message, const char *file, i32 line) {
  log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}

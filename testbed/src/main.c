#include <core/logger.h>
#include <core/asserts.h>

#include <platform/platform.h>

int main(void) {
  TFATAL("FATAL");
  TERROR("ERROR");
  TWARN("WARN");
  TINFO("INFO");
  TDEBUG("DEBUG");
  TTRACE("TRACE");

  void *platformState;
  if(platform_startup(&platformState, "Tea Engine Testbed", 100, 100, 1200, 720)) {
    while(true) {
      platform_pump_message(&platformState);
    }
  }
  platform_shutdown(&platformState);

  return 0;
}

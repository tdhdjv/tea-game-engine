#include <core/logger.h>
#include <core/asserts.h>

int main(void) {
  TFATAL("Test message: %f", 1.0f);
  TERROR("Test message: %f", 1.0f);
  TWARN("Test message: %f", 1.0f);
  TINFO("Test message: %f", 1.0f);
  TDEBUG("Test message: %f", 1.0f);
  TTRACE("Test message");
  TASSERT(false);
  return 0;
}

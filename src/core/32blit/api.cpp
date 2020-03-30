#include "api.hpp"

extern char __api_start;

namespace blit {
  API &api = *(API *)(uintptr_t)0x0000f800;
}

extern "C" int blit_debugf(const char * psFormatString, ...) {
  va_list args;
  va_start(args, psFormatString);
  int ret = blit::api.debugf(psFormatString, args);
  va_end(args);
  return ret;
}
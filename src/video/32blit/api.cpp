#include "api.hpp"

extern char __api_start;

namespace blit {
  API &api = *(API *)(uintptr_t)0x0000f800;
}
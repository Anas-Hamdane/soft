#include "stl.h"
#include "common.h"

namespace soft {
  void __todo__impl(const char* file, int line, const char* func) {
    std::println(stderr, "`todo()` call:");
    std::println(stderr, "feature not yet implemented in function {} at {}:{}", func, file, line);
    std::abort();
  }

  void __unreachable__impl(const char* file, int line, const char* func) {
    std::println(stderr, "`unreachable()` call in function {} at {}:{}", func, file, line);
    std::abort();
  }
}

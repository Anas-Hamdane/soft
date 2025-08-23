#pragma once

#include "ir/Program.h"

namespace soft {
  namespace codegen {
    std::string generate(const Program& program);
  }
}

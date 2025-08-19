#include "ir.h"

namespace soft {
  namespace codegen {
    struct Memory {
      Type type;
      off_t offset;
    };

    void generate_instruction(const ir::Instruction& instr);
    void generate_params(const std::vector<ir::Register>& params);
    void generate_function(const ir::Function& fn);
    std::string generate(const ir::Program& program);
  }
}

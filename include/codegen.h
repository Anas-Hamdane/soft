#include "ir.h"

namespace soft {
  namespace codegen {
    struct Memory {
      Type type;
      off_t offset;
    };
    struct Register {
      Type type;
      size_t index;
    };

    struct GPR {
      enum class Knd : int {
        RAX = 0, RCX, RDX, RSI, RDI,
        R8, R9, R10, R11
      } knd;
      bool reserved;
    };
    struct XMM {
      enum class Knd : int {
        XMM0 = 0, XMM1, XMM2, XMM3, XMM4, XMM5,
        XMM6, XMM7, XMM8, XMM9, XMM10, XMM11,
        XMM12, XMM13, XMM14, XMM15
      } knd;
      bool reserved;
    };

    std::string gpr_by_size(const GPR::Knd& knd, size_t byte);
    std::string xmm_by_size(const XMM::Knd& knd, size_t byte);
    std::string resolve_constant(const ir::Constant& constant);

    void generate_terminator(const ir::Terminator& term);
    void generate_instruction(const ir::Instruction& instr);
    void generate_params(const std::vector<ir::Slot>& params);
    void generate_fn_prologue(const ir::Function& fn);
    void generate_function(const ir::Function& fn);

    std::string generate(const ir::Program& program);
  }
}

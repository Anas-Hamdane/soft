#include "ir.h"

namespace soft {
  namespace codegen {
    /*
     * Memory represents an allocated stack variable with his type
     * and his stack offset.
     * Register represents a physical register that has a type
     * and a kind, the kind also is used to check whether the register
     * is reserved or not in an array of pairs <Register::Knd, bool>.
     */
    struct Memory {
      Type type;
      off_t offset;
    };
    struct Register {
      Type type;
      enum class Knd : int {
        RAX = 0, RCX, RDX, RSI, RDI,
        R8, R9, R10, R11, XMM0 , XMM1,
        XMM2, XMM3, XMM4, XMM5, XMM6,
        XMM7, XMM8, XMM9, XMM10, XMM11,
        XMM12, XMM13, XMM14, XMM15
      } knd;
    };

    void generate_terminator(const ir::Terminator& term);
    void generate_instruction(const ir::Instruction& instr);
    void generate_params(const std::vector<ir::Slot>& params);
    void generate_fn_prologue(const ir::Function& fn);
    void generate_function(const ir::Function& fn);

    std::string generate(const ir::Program& program);
  }
}

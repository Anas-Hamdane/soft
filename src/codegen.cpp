#include "codegen.h"
#include "common.h"

#define f(fmt, ...) out += std::format(fmt "\n" __VA_OPT__(,) __VA_ARGS__)
#define is_float(l) (l.knd == Type::Knd::Float)
#define is_int(l) (l.knd == Type::Knd::Int)

namespace soft {
  namespace codegen {
    constexpr std::array<GPR, 14> gpr = {{
      {GPR::Knd::RAX, 8, false},
      {GPR::Knd::RBX, 8, false},
      {GPR::Knd::RCX, 8, false},
      {GPR::Knd::RDX, 8, false},
      {GPR::Knd::RSI, 8, false},
      {GPR::Knd::RDI, 8, false},
      // {GPR::Knd::RSP, 8, false},
      // {GPR::Knd::RBP, 8, false},
      {GPR::Knd::R8 , 8, false},
      {GPR::Knd::R9 , 8, false},
      {GPR::Knd::R10, 8, false},
      {GPR::Knd::R11, 8, false},
      {GPR::Knd::R12, 8, false},
      {GPR::Knd::R13, 8, false},
      {GPR::Knd::R14, 8, false},
      {GPR::Knd::R15, 8, false},
    }};
    constexpr std::array<XMM, 16> xmm = {{
      {XMM::Knd::XMM0 , 8, false},
      {XMM::Knd::XMM1 , 8, false},
      {XMM::Knd::XMM2 , 8, false},
      {XMM::Knd::XMM3 , 8, false},
      {XMM::Knd::XMM4 , 8, false},
      {XMM::Knd::XMM5 , 8, false},
      {XMM::Knd::XMM6 , 8, false},
      {XMM::Knd::XMM7 , 8, false},
      {XMM::Knd::XMM8 , 8, false},
      {XMM::Knd::XMM9 , 8, false},
      {XMM::Knd::XMM10, 8, false},
      {XMM::Knd::XMM11, 8, false},
      {XMM::Knd::XMM12, 8, false},
      {XMM::Knd::XMM13, 8, false},
      {XMM::Knd::XMM14, 8, false},
      {XMM::Knd::XMM15, 8, false},
    }};
    std::unordered_map<size_t, Memory> stack;
    std::string out;
    off_t offset; // stack offset


    std::string gpr_by_size(const GPR::Knd& knd, size_t byte)
    {
      static constexpr std::array<std::string, 14> regs64 = {
        "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", /* "%rsp", "%rbp", */
        "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"
      };

      static const std::array<std::string, 14> regs32 = {
        "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi", /* "%esp", "%ebp", */
        "%r8d", "%r9d", "%r10d", "%r11d", "%r12d", "%r13d", "%r14d", "%r15d"
      };

      static const std::array<std::string, 14> regs16 = {
        "%ax", "%bx", "%cx", "%dx", "%si", "%di", /* "%sp", "%bp", */
        "%r8w", "%r9w", "%r10w", "%r11w", "%r12w", "%r13w", "%r14w", "%r15w"
      };

      static const std::array<std::string, 14> regs8 = {
        "%al", "%bl", "%cl", "%dl", "%sil", "%dil", /* "%spl", "%bpl", */
        "%r8b", "%r9b", "%r10b", "%r11b", "%r12b", "%r13b", "%r14b", "%r15b"
      };

      int index = static_cast<int>(knd);

      switch (byte) {
        case 8:  return regs64[index];
        case 4:  return regs32[index];
        case 2:  return regs16[index];
        case 1:  return regs8[index];
        default: unreachable();
      }
    }
    std::string xmm_by_size(const XMM::Knd& knd, size_t byte)
    {
      static constexpr std::array<std::string, 16> regs = {
        "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5",
        "%xmm6", "%xmm7", "%xmm8", "%xmm9", "%xmm10", "%xmm11",
        "%xmm12", "%xmm13", "%xmm14", "%xmm15"
      };

      int index = static_cast<int>(knd);

      switch (byte)
      {
        case 4:
        case 8:
          return regs[index];
        default:
          unreachable();
      }
    }
    std::string resolve_constant(const ir::Constant& constant)
    {
      std::string form;
      switch (constant.v.index())
      {
        case 0: // uint64_t
          form += "$" + std::to_string(std::get<0>(constant.v));
          break;
        case 1: // int64_t
          form += "$" + std::to_string(std::get<1>(constant.v));
          break;
        case 2: // double
        {
          // TODO:
        }
      }

      return form;
    }

    char suffix(const Type& type)
    {
      if (is_float(type))
      {
        switch (type.byte)
        {
          case 4:  return 's';
          case 8:  return 'd';
          default: unreachable();
        }
      }
      switch (type.byte) 
      {
        case 1:  return 'b';
        case 2:  return 'w';
        case 4:  return 'l';
        case 8:  return 'q';
        default: unreachable();
      }
    }
    std::string register_by_size(const std::string& r, size_t byte)
    {
      std::string base = r;

      // floating points registers
      if      (r == "%xmm0" ) return "%xmm0";
      else if (r == "%xmm1" ) return "%xmm1";
      else if (r == "%xmm2" ) return "%xmm2";
      else if (r == "%xmm3" ) return "%xmm3";
      else if (r == "%xmm4" ) return "%xmm4";
      else if (r == "%xmm5" ) return "%xmm5";
      else if (r == "%xmm6" ) return "%xmm6";
      else if (r == "%xmm7" ) return "%xmm7";
      else if (r == "%xmm8" ) return "%xmm8";
      else if (r == "%xmm9" ) return "%xmm9";
      else if (r == "%xmm10") return "%xmm10";
      else if (r == "%xmm11") return "%xmm11";
      else if (r == "%xmm12") return "%xmm12";
      else if (r == "%xmm13") return "%xmm13";
      else if (r == "%xmm14") return "%xmm14";
      else if (r == "%xmm15") return "%xmm15";

      // others
      if      (r == "%al"  || r == "%ax" || r == "%eax" || r == "%rax") base = "%rax";
      else if (r == "%bl"  || r == "%bx" || r == "%ebx" || r == "%rbx") base = "%rbx";
      else if (r == "%cl"  || r == "%cx" || r == "%ecx" || r == "%rcx") base = "%rcx";
      else if (r == "%dl"  || r == "%dx" || r == "%edx" || r == "%rdx") base = "%rdx";
      else if (r == "%spl" || r == "%sp" || r == "%esp" || r == "%rsp") base = "%rsp";
      else if (r == "%bpl" || r == "%bp" || r == "%ebp" || r == "%rbp") base = "%rbp";
      else if (r == "%sil" || r == "%si" || r == "%esi" || r == "%rsi") base = "%rsi";
      else if (r == "%dil" || r == "%di" || r == "%edi" || r == "%rdi") base = "%rdi";
      else if (r.substr(0,2) == "%r8" )                                 base = "%r8";
      else if (r.substr(0,2) == "%r9" )                                 base = "%r9";
      else if (r.substr(0,3) == "%r10")                                 base = "%r10";
      else if (r.substr(0,3) == "%r11")                                 base = "%r11";
      else if (r.substr(0,3) == "%r12")                                 base = "%r12";
      else if (r.substr(0,3) == "%r13")                                 base = "%r13";
      else if (r.substr(0,3) == "%r14")                                 base = "%r14";
      else if (r.substr(0,3) == "%r15")                                 base = "%r15";

      if (base == "%rax")
      {
        if (byte == 1) return "%al";
        if (byte == 2) return "%ax";
        if (byte == 4) return "%eax";
        if (byte == 8) return "%rax";
      }
      else if (base == "%rbx")
      {
        if (byte == 1) return "%bl";
        if (byte == 2) return "%bx";
        if (byte == 4) return "%ebx";
        if (byte == 8) return "%rbx";
      }
      else if (base == "%rcx")
      {
        if (byte == 1) return "%cl";
        if (byte == 2) return "%cx";
        if (byte == 4) return "%ecx";
        if (byte == 8) return "%rcx";
      }
      else if (base == "%rdx")
      {
        if (byte == 1) return "%dl";
        if (byte == 2) return "%dx";
        if (byte == 4) return "%edx";
        if (byte == 8) return "%rdx";
      }
      else if (base == "%rsp")
      {
        if (byte == 1) return "%spl";
        if (byte == 2) return "%sp";
        if (byte == 4) return "%esp";
        if (byte == 8) return "%rsp";
      }
      else if (base == "%rbp")
      {
        if (byte == 1) return "%bpl";
        if (byte == 2) return "%bp";
        if (byte == 4) return "%ebp";
        if (byte == 8) return "%rbp";
      }
      else if (base == "%rsi")
      {
        if (byte == 1) return "%sil";
        if (byte == 2) return "%si";
        if (byte == 4) return "%esi";
        if (byte == 8) return "%rsi";
      }
      else if (base == "%rdi")
      {
        if (byte == 1) return "%dil";
        if (byte == 2) return "%di";
        if (byte == 4) return "%edi";
        if (byte == 8) return "%rdi";
      }
      else if (base == "%r8")
      {
        if (byte == 1) return "%r8b";
        if (byte == 2) return "%r8w";
        if (byte == 4) return "%r8d";
        if (byte == 8) return "%r8";
      }
      else if (base == "%r9")
      {
        if (byte == 1) return "%r9b";
        if (byte == 2) return "%r9w";
        if (byte == 4) return "%r9d";
        if (byte == 8) return "%r9";
      }
      else if (base == "%r10")
      {
        if (byte == 1) return "%r10b";
        if (byte == 2) return "%r10w";
        if (byte == 4) return "%r10d";
        if (byte == 8) return "%r10";
      }
      else if (base == "%r11")
      {
        if (byte == 1) return "%r11b";
        if (byte == 2) return "%r11w";
        if (byte == 4) return "%r11d";
        if (byte == 8) return "%r11";
      }
      else if (base == "%r12")
      {
        if (byte == 1) return "%r12b";
        if (byte == 2) return "%r12w";
        if (byte == 4) return "%r12d";
        if (byte == 8) return "%r12";
      }
      else if (base == "%r13")
      {
        if (byte == 1) return "%r13b";
        if (byte == 2) return "%r13w";
        if (byte == 4) return "%r13d";
        if (byte == 8) return "%r13";
      }
      else if (base == "%r14")
      {
        if (byte == 1) return "%r14b";
        if (byte == 2) return "%r14w";
        if (byte == 4) return "%r14d";
        if (byte == 8) return "%r14";
      }
      else if (base == "%r15")
      {
        if (byte == 1) return "%r15b";
        if (byte == 2) return "%r15w";
        if (byte == 4) return "%r15d";
        if (byte == 8) return "%r15";
      }

      unreachable();
    }

    void generate_terminator(const ir::Terminator& term)
    {
      switch (term.index())
      {
        case 0: // Return
        {
          auto& ret = std::get<0>(term);
          switch (ret.v.index())
          {
            case 0: // Constant
            {
              auto& constant = std::get<0>(ret.v);
              const char suff = suffix(constant.type);
              std::string src = resolve_constant(constant);

              std::string mov = "mov";
              std::string dst;
              if (constant.type.knd == Type::Knd::Float)
              {
                dst = xmm_by_size(XMM::Knd::XMM0, constant.type.byte);
                mov += 's' + suff;
              }
              else
              {
                dst = gpr_by_size(GPR::Knd::RAX, constant.type.byte);
                mov += suff;
              }

              f("  {} {}, {}", mov, src, dst);
              break;
            }
            case 1: // Register
            {
              todo();
            }
          }

          f("  popq %rbp");
          f("  retq");
        }
      }
    }
    void generate_instruction(const ir::Instruction& instr)
    {
      switch (instr.index())
      {
        case 0: // Alloca
        {
          const ir::Alloca& alloca = std::get<0>(instr);
          Memory mem = { alloca.type, (offset += alloca.type.byte) };
          stack[alloca.reg.id] = mem;
          return;
        }
        case 1: // Store
        {

        }
        case 2: // BinOp
        {

        }
        case 3: // UnOp
        {

        }
        case 4: // Conv
        {

        }
        default: unreachable();
      }
    }
    void generate_params(const std::vector<ir::Register>& params)
    {
      static constexpr std::array<std::string_view, 6> integer_regs = {
        "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
      };
      static constexpr std::array<std::string_view, 8> float_regs = {
        "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7"
      };

      size_t ir_idx = 0; // integer register index
      size_t fr_idx = 0; // float register index
      off_t stack_params_offset = 16;
      for (const auto& param : params)
      {
        const char suff = suffix(param.type);
        offset += param.type.byte;

        switch (param.type.knd)
        {
          case Type::Knd::Float:
          {
            if (fr_idx < float_regs.size())
            {
              f("  movs{} {}, -{}(%rbp)", suff, float_regs[fr_idx++], offset);
              stack[param.id] = { param.type, offset };
            }
            else
            {
              f("  movs{} {}(%rbp), %xmm0", suff, stack_params_offset);
              f("  movs{} %xmm0, -{}(%rbp)", suff, offset);
              stack[param.id] = { param.type, offset };
              stack_params_offset += param.type.byte;
            }
            break;
          }
          case Type::Knd::Int:
          case Type::Knd::UInt:
          {
            if (ir_idx < integer_regs.size())
            {
              f("  mov{} {}, -{}(%rbp)", suff, integer_regs[ir_idx++], offset);
              stack[param.id] = { param.type, offset };
            }
            else
            {
              // intermediate register
              const std::string ir = register_by_size("%rax", param.type.byte);
              f("  mov{} {}(%rbp), {}", suff, stack_params_offset, ir);
              f("  mov{} {}, -{}(%rbp)", suff, ir, offset);
              stack[param.id] = { param.type, offset };
              stack_params_offset += param.type.byte;
            }
            break;
          }
        }
      }
    }
    void generate_function(const ir::Function& fn)
    {
      if (fn.mod == ir::Function::Mod::Declaration)
        return; // do nothing

      f(".globl {}", fn.name);
      f(".type {}, @function", fn.name);
      f("{}:", fn.name);

      // proloque
      f("  pushq %rbp");
      f("  movq %rsp, %rbp");
      offset = 0;

      generate_params(fn.params);

      for (auto& instr : fn.instrs)
        generate_instruction(instr);

      if (fn.terminator.has_value())
        generate_terminator(*fn.terminator);
    }

    std::string generate(const ir::Program& program)
    {
      out += ".section .text\n";

      for (auto& fn : program.fns)
        generate_function(fn);

      return out;
    }
  }
}

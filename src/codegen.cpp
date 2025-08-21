#include "codegen.h"
#include "common.h"

#define f(fmt, ...) out += std::format(fmt "\n" __VA_OPT__(,) __VA_ARGS__)
#define is_float(l) (l.knd == Type::Knd::Float)
#define is_int(l) (l.knd == Type::Knd::Int)

namespace soft {
  namespace codegen {
    std::array<std::pair<Register::Knd, bool>, 25> pool = {{
      {Register::Knd::RAX  , false},
      {Register::Knd::RCX  , false},
      {Register::Knd::RDX  , false},
      {Register::Knd::RSI  , false},
      {Register::Knd::RDI  , false},
      {Register::Knd::R8   , false},
      {Register::Knd::R9   , false},
      {Register::Knd::R10  , false},
      {Register::Knd::R11  , false},
      {Register::Knd::XMM0 , false},
      {Register::Knd::XMM1 , false},
      {Register::Knd::XMM2 , false},
      {Register::Knd::XMM3 , false},
      {Register::Knd::XMM4 , false},
      {Register::Knd::XMM5 , false},
      {Register::Knd::XMM6 , false},
      {Register::Knd::XMM7 , false},
      {Register::Knd::XMM8 , false},
      {Register::Knd::XMM9 , false},
      {Register::Knd::XMM10, false},
      {Register::Knd::XMM11, false},
      {Register::Knd::XMM12, false},
      {Register::Knd::XMM13, false},
      {Register::Knd::XMM14, false},
      {Register::Knd::XMM15, false},
    }};

    std::unordered_map<size_t, std::variant<Register, Memory>> registers;
    std::string out;
    off_t offset; // stack offset

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
    std::string register_to_string(const Register& r)
    {
      static constexpr std::array<std::string, 9> gpr64 = {
        "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
      };
      static constexpr std::array<std::string, 9> gpr32 = {
        "eax", "ecx", "edx", "esi", "edi", "r8d", "r9d", "r10d", "r11d"
      };
      static constexpr std::array<std::string, 9> gpr16 = {
        "ax", "cx", "dx", "si", "di", "r8w", "r9w", "r10w", "r11w"
      };
      static constexpr std::array<std::string, 9> gpr8 = {
        "al", "cl", "dl", "sil", "dil", "r8b", "r9b", "r10b", "r11b"
      };
      static constexpr std::array<std::string, 16> xmms = {
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
        "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13",
        "xmm14", "xmm15",
      };

      size_t index = (int) r.knd;
      if (index >= 9)
        return xmms[index];

      switch (r.type.byte)
      {
        case 1: return "%" + gpr8[index];
        case 2: return "%" + gpr16[index];
        case 4: return "%" + gpr32[index];
        case 8: return "%" + gpr64[index];
      }
      unreachable();
    }
    std::string resolve_constant(const ir::Constant& c)
    {
      std::string form;
      switch (c.v.index())
      {
        case 0: // uint64_t
          form += "$" + std::to_string(std::get<0>(c.v));
          break;
        case 1: // int64_t
          form += "$" + std::to_string(std::get<1>(c.v));
          break;
        case 2: // double
        {
          // TODO:
        }
      }

      return form;
    }
    std::string resolve_slot(const ir::Slot& s)
    {
      std::variant<Register, Memory>& r = registers[s.id];
      switch (r.index())
      {
        case 0: // Register
        {
          const Register& reg = std::get<0>(r);
          return register_to_string(reg);
        }
        case 1: // Memory
        {
          const Memory& mem = std::get<1>(r);
          return std::format("-{}(%rbp)", mem.offset);
        }
      }
      unreachable();
    }
    std::string resolve_value(const ir::Value& v)
    {
      switch (v.index())
      {
        case 0: return resolve_constant(std::get<0>(v));
        case 1: return resolve_slot(std::get<1>(v));
      }
      unreachable();
    }
    std::string return_register(const Type& type)
    {
      switch (type.knd)
      {
        case Type::Knd::Float:
          return "%xmm0";
        case Type::Knd::Int:
        case Type::Knd::UInt:
          switch (type.byte)
          {
            case 1: return "%al";
            case 2: return "%ax";
            case 4: return "%eax";
            case 8: return "%rax";
            default: unreachable();
          }
      }
      unreachable();
    }
    std::string resolve_mov(const Type& type)
    {
      std::string result = "mov";
      if (is_float(type))
        result += 's';

      result += suffix(type);
      return result;
    }
    void allocate_register(const ir::Slot& s)
    {
      switch (s.type.knd)
      {
        case Type::Knd::Int:
        case Type::Knd::UInt:
        {
          for (size_t i = 0; i < 9; ++i)
          {
            if (!pool[i].second) // not reserved
            {
              Register regstr = { s.type, pool[i].first };
              registers[s.id] = regstr;
              return;
            }
          }

          // fallback to memory
          Memory mem = { s.type, (offset += s.type.byte) };
          registers[s.id] = mem;
          return;
        }
        case Type::Knd::Float:
        {
          for (size_t i = 9; i < pool.size(); ++i)
          {
            if (!pool[i].second) // not reserved
            {
              Register regstr = { s.type, pool[i].first };
              registers[s.id] = regstr;
              return;
            }
          }

          // fallback to memory
          Memory mem = { s.type, (offset += s.type.byte) };
          registers[s.id] = mem;
          return;
        }
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

          std::string mov = resolve_mov(ret.type);
          std::string src = resolve_value(ret.v);
          std::string dst = return_register(ret.type);

          f("  {} {}, {}", mov, src, dst);
          f("  popq %rbp");
          f("  retq");
          return;
        }
        default:
          unreachable();
      }
    }
    void generate_instruction(const ir::Instruction& instr)
    {
      switch (instr.index())
      {
        case 0: // Alloca
        {
          auto& alloca = std::get<0>(instr);
          Memory mem = { alloca.type, (offset += alloca.type.byte) };
          registers[alloca.slot.id] = mem;
          return;
        }
        case 1: // Conv
        {
          todo();
        }
        case 2: // Store
        {
          auto& store = std::get<2>(instr);

          std::string mov = resolve_mov(store.dst.type);
          std::string src = resolve_value(store.src);
          std::string dst = resolve_slot(store.dst);

          f("  {} {}, {}", mov, src, dst);
          return;
        }
        case 3: // Load
        {
          auto& load = std::get<3>(instr);
          allocate_register(load.dst);

          std::string mov = resolve_mov(load.dst.type);
          std::string src = resolve_value(load.src);
          std::string dst = resolve_slot(load.dst);

          f("  {} {}, {}", mov, src, dst);
          return;
        }
        case 5: // BinOp
        {
          todo();
        }
        case 6: // UnOp
        {
          todo();
        }
        default: unreachable();
      }
    }
    void generate_params(const std::vector<ir::Slot>& params)
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
              registers[param.id] = Memory { param.type, offset };
            }
            else
            {
              f("  movs{} {}(%rbp), %xmm0", suff, stack_params_offset);
              f("  movs{} %xmm0, -{}(%rbp)", suff, offset);
              registers[param.id] = Memory { param.type, offset };
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
              registers[param.id] = Memory { param.type, offset };
            }
            else
            {
              // intermediate register
              const std::string ir = register_by_size("%rax", param.type.byte);
              f("  mov{} {}(%rbp), {}", suff, stack_params_offset, ir);
              f("  mov{} {}, -{}(%rbp)", suff, ir, offset);
              registers[param.id] = Memory { param.type, offset };
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

      // prologue
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

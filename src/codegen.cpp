#include "codegen.h"
#include "common.h"

#define f(fmt, ...) std::format(fmt "\n" __VA_OPT__(,) __VA_ARGS__)
#define is_float(l) (l.knd == Type::Knd::Float)
#define is_int(l) (l.knd == Type::Knd::Int)

namespace soft {
  namespace codegen {
    std::unordered_map<size_t, Memory> stack;
    std::string out;
    off_t offset; // stack offset

    char suffix(Type type)
    {
      if (is_float(type))
      {
        switch (type.byte)
        {
          case 4:  return 's';
          case 8:  return 'd';
          default: std::unreachable();
        }
      }
      switch (type.byte) 
      {
        case 1:  return 'b';
        case 2:  return 'w';
        case 4:  return 'l';
        case 8:  return 'q';
        default: std::unreachable();
      }
    }
    std::string register_by_size(const std::string& r, size_t byte)
    {
      std::string base = r;

      // floating points registers
      if (r == "xmm0")      return "xmm0";
      else if (r == "xmm1") return "xmm1";
      else if (r == "xmm2") return "xmm2";
      else if (r == "xmm3") return "xmm3";
      else if (r == "xmm4") return "xmm4";
      else if (r == "xmm5") return "xmm5";
      else if (r == "xmm6") return "xmm6";
      else if (r == "xmm7") return "xmm7";

      // others
      if      (r == "al"  || r == "ax" || r == "eax" || r == "rax") base = "rax";
      else if (r == "bl"  || r == "bx" || r == "ebx" || r == "rbx") base = "rbx";
      else if (r == "cl"  || r == "cx" || r == "ecx" || r == "rcx") base = "rcx";
      else if (r == "dl"  || r == "dx" || r == "edx" || r == "rdx") base = "rdx";
      else if (r == "spl" || r == "sp" || r == "esp" || r == "rsp") base = "rsp";
      else if (r == "bpl" || r == "bp" || r == "ebp" || r == "rbp") base = "rbp";
      else if (r == "sil" || r == "si" || r == "esi" || r == "rsi") base = "rsi";
      else if (r == "dil" || r == "di" || r == "edi" || r == "rdi") base = "rdi";
      else if (r.substr(0,2) == "r8")                               base = "r8";
      else if (r.substr(0,2) == "r9")                               base = "r9";
      else if (r.substr(0,3) == "r10")                              base = "r10";
      else if (r.substr(0,3) == "r11")                              base = "r11";
      else if (r.substr(0,3) == "r12")                              base = "r12";
      else if (r.substr(0,3) == "r13")                              base = "r13";
      else if (r.substr(0,3) == "r14")                              base = "r14";
      else if (r.substr(0,3) == "r15")                              base = "r15";

      if (base == "rax")
      {
        if (byte == 1) return "al";
        if (byte == 2) return "ax";
        if (byte == 4) return "eax";
        if (byte == 8) return "rax";
      }
      else if (base == "rbx")
      {
        if (byte == 1) return "bl";
        if (byte == 2) return "bx";
        if (byte == 4) return "ebx";
        if (byte == 8) return "rbx";
      }
      else if (base == "rcx")
      {
        if (byte == 1) return "cl";
        if (byte == 2) return "cx";
        if (byte == 4) return "ecx";
        if (byte == 8) return "rcx";
      }
      else if (base == "rdx")
      {
        if (byte == 1) return "dl";
        if (byte == 2) return "dx";
        if (byte == 4) return "edx";
        if (byte == 8) return "rdx";
      }
      else if (base == "rsp")
      {
        if (byte == 1) return "spl";
        if (byte == 2) return "sp";
        if (byte == 4) return "esp";
        if (byte == 8) return "rsp";
      }
      else if (base == "rbp")
      {
        if (byte == 1) return "bpl";
        if (byte == 2) return "bp";
        if (byte == 4) return "ebp";
        if (byte == 8) return "rbp";
      }
      else if (base == "rsi")
      {
        if (byte == 1) return "sil";
        if (byte == 2) return "si";
        if (byte == 4) return "esi";
        if (byte == 8) return "rsi";
      }
      else if (base == "rdi")
      {
        if (byte == 1) return "dil";
        if (byte == 2) return "di";
        if (byte == 4) return "edi";
        if (byte == 8) return "rdi";
      }
      else if (base == "r8")
      {
        if (byte == 1) return "r8b";
        if (byte == 2) return "r8w";
        if (byte == 4) return "r8d";
        if (byte == 8) return "r8";
      }
      else if (base == "r9")
      {
        if (byte == 1) return "r9b";
        if (byte == 2) return "r9w";
        if (byte == 4) return "r9d";
        if (byte == 8) return "r9";
      }
      else if (base == "r10")
      {
        if (byte == 1) return "r10b";
        if (byte == 2) return "r10w";
        if (byte == 4) return "r10d";
        if (byte == 8) return "r10";
      }
      else if (base == "r11")
      {
        if (byte == 1) return "r11b";
        if (byte == 2) return "r11w";
        if (byte == 4) return "r11d";
        if (byte == 8) return "r11";
      }
      else if (base == "r12")
      {
        if (byte == 1) return "r12b";
        if (byte == 2) return "r12w";
        if (byte == 4) return "r12d";
        if (byte == 8) return "r12";
      }
      else if (base == "r13")
      {
        if (byte == 1) return "r13b";
        if (byte == 2) return "r13w";
        if (byte == 4) return "r13d";
        if (byte == 8) return "r13";
      }
      else if (base == "r14")
      {
        if (byte == 1) return "r14b";
        if (byte == 2) return "r14w";
        if (byte == 4) return "r14d";
        if (byte == 8) return "r14";
      }
      else if (base == "r15")
      {
        if (byte == 1) return "r15b";
        if (byte == 2) return "r15w";
        if (byte == 4) return "r15d";
        if (byte == 8) return "r15";
      }

      std::unreachable();
    }

    void generate_instruction(ir::Instruction& instr)
    {
      switch (instr.index())
      {
        case 0: // Alloca
        {
          ir::Alloca& alloca = std::get<0>(instr);
          Memory mem = { alloca.type, (offset += alloca.type.byte) };
          stack[alloca.reg.id] = mem;
          break;
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
        default: std::unreachable();
      }
    }
    void generate_params(std::vector<ir::Register>& params)
    {
      const static std::array<std::string_view, 6> nregs = 
             { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };
      const static std::array<std::string_view, 8> fregs =
             { "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7" };

      size_t nr = 0; // track the current nreg
      size_t fr = 0; // track the current freg
      off_t prms_offset = 16; // stack parameters offset
      for (size_t i = 0; i < params.size(); ++i)
      {
        ir::Register prm = params[i];
        char suff = suffix(prm.type);

        if (is_float(prm.type))
        {
          if (fr < fregs.size())
          {
            out += f("movs{} {}, -{}(%%rbp)", suff, fregs[fr], offset += prm.type.byte);
            stack[prm.id] = { prm.type, offset };
            continue;
          }
          else
          {
            out += f("movs{} {}(%rbp), %xmm8", suff, prms_offset);
            out += f("movs{} %xmm8, -{}(%rbp)", suff, offset += prm.type.byte);
            stack[prm.id] = { prm.type, offset };
            prms_offset += prm.type.byte;
            continue;
          }
        }
        else 
        {
          if (nr < nregs.size())
          {
            out += f("mov{} {}, -{}(%%rbp)", suff, nregs[fr], offset += prm.type.byte);
            stack[prm.id] = { prm.type, offset };
            continue;
          }
          else
          {
            std::string ir = register_by_size("rax", prm.type.byte);
            out += f("movs{} {}(%rbp), {}", suff, prms_offset, ir);
            out += f("movs{} {}, -{}(%rbp)", suff, ir, offset += prm.type.byte);
            stack[prm.id] = { prm.type, offset };
            prms_offset += prm.type.byte;
            continue;
          }
        }
      }
    }
    void generate_function(ir::Function& fn)
    {
      if (fn.mod == ir::Function::Mod::Declaration)
        return; // do nothing

      out += f(".globl {}", fn.name);
      out += f(".type {}, @function", fn.name);
      out += f("{}:", fn.name);

      // proloque
      out += f("  pushq %rbp");
      out += f("  movq %rsp, %rbp");
      offset = 0;

      generate_params(fn.params);

      for (auto& instr : fn.instrs)
        generate_instruction(instr);
    }

    std::string generate(ir::Program program)
    {
      out += ".section .text\n";

      for (auto& fn : program.fns)
        generate_function(fn);

      return out;
    }
  }
}

#include "common.h"
#include "codegen/codegen.h"
#include "codegen/Storage.h"

#define append(fmt, ...) out += std::format(fmt "\n" __VA_OPT__(,) __VA_ARGS__) 

namespace soft {
  namespace codegen {
    std::vector<Storage> storage;
    std::string out;
    size_t offset;

    char suffix(const Type& type)
    {
      if (type.isFloatingPoint())
      {
        switch (type.getByteSize())
        {
          case 4: return 's';
          case 8: return 'd';
        }
      }
      else
      {
        switch (type.getByteSize())
        {
          case 1: return 'b';
          case 2: return 'w';
          case 4: return 'l';
          case 8: return 'q';
        }
      }
      unreachable();
    }
    std::string mov(const Type& type)
    {
      std::string mov = "mov";
      if (type.isFloatingPoint())
        mov += 's';

      mov += suffix(type);
      return mov;
    }
    void generate_function(const Function& fn)
    {
      if (!fn.isDefined())
        return; // do nothing

      size_t total_registers = fn.getTotalRegisters();
      if (total_registers > storage.capacity())
        storage.reserve(total_registers - storage.capacity());

      const std::string& name = fn.getName();
      append(".globl {}", name);
      append(".type {}, @function", name);
      append("{}:", name);

      // prologue
      append("  pushq %rbp");
      append("  movq %rsp, %rbp");
      offset = 0;
    }
    std::string generate(const Program& program)
    {
      append("# Program: {}", program.getName());
      append(".section .text\n");

      const auto& functions = program.getFunctions();
      for (const auto& fn : functions)
        generate_function(fn);

      return out;
    }
  }
}

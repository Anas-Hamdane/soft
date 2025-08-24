#include "common.h"
#include "codegen/codegen.h"
#include "codegen/Storage.h"
#include <cassert>

#define append(fmt, ...) out += std::format(fmt "\n" __VA_OPT__(,) __VA_ARGS__) 

namespace soft {
  namespace codegen {
    // to track reserved and unreserved registers
    // NOTE: the order is crucial since we access them by
    // the value of Type::Knd enum which is equal to the index
    // of the value in this array.
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

    std::unordered_map<size_t, Storage> storage;
    std::string out;
    size_t offset;

    void deallocate(const Register& register_v)
    {
      pool[static_cast<int>(register_v.getKnd())].second = false;
    }
    void deallocate(const Slot& slot)
    {
      const auto& stored = storage[slot.getId()];
      if (stored.isRegister())
      {
        return deallocate(stored.getValue<1>());
      }
    }
    void deallocate(const Value& value)
    {
      if (value.isSlot())
      {
        return deallocate(value.getValue<1>());
      }
    }
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
    std::string form(const Type& type)
    {
      std::string mov = "mov";
      if (type.isFloatingPoint())
        mov += 's';

      mov += suffix(type);
      return mov;
    }
    std::string form(const Constant& constant)
    {
      switch (constant.getIndex())
      {
        case 0: // int64_t
          return std::format("${}", constant.getValue<0>());
        case 1: // double
          todo();
      }
      unreachable();
    }
    std::string form(const Value& value)
    {
      switch (value.getIndex())
      {
        case 0: // Constant
          return form(value.getValue<0>());
        case 1: // Slot
          return storage[value.getValue<1>().getId()].toString();
      }
      unreachable();
    }
    Storage make_temp(const Type& type)
    {
      static constexpr size_t integer_register_size = 9;
      static constexpr size_t float_register_size = 15;

      assert(!type.isVoid());
      size_t start = type.isFloatingPoint() ? integer_register_size : 0;
      size_t end = type.isFloatingPoint() ? float_register_size : integer_register_size;

      for (size_t i = start; i < end; ++i)
      {
        if (!pool[i].second) // not reserved
        {
          return Storage( Register(type, pool[i].first) );
        }
      }

      // if there's no free registers
      // fallback to memory
      offset += type.getByteSize();
      return Storage( Memory(type, offset) );
    }
    Storage allocate_register(const Type& type)
    {
      static constexpr size_t integer_register_size = 9;
      static constexpr size_t float_register_size = 15;

      assert(!type.isVoid());
      size_t start = type.isFloatingPoint() ? integer_register_size : 0;
      size_t end = type.isFloatingPoint() ? float_register_size : integer_register_size;

      for (size_t i = start; i < end; ++i)
      {
        if (!pool[i].second) // not reserved
        {
          pool[i].second = true;
          return Storage( Register(type, pool[i].first) );
        }
      }

      // if there's no free registers
      // fallback to memory
      offset += type.getByteSize();
      return Storage( Memory(type, offset) );
    }

    void generate_instruction(const Instruction& instruction)
    {
      switch (instruction.index())
      {
        case 0: // Alloca
        {
          auto& alloca = std::get<0>(instruction);
          offset += alloca.getType().getByteSize();

          Memory mem(alloca.getType(), offset);
          storage[alloca.getDst().getId()].setValue<Memory>(mem);

          return;
        }
        case 1: // Store
        {
          auto& store = std::get<1>(instruction);

          std::string mov = form(store.getDst().getType());
          std::string src = form(store.getSrc());
          std::string dst = storage[store.getDst().getId()].toString();

          append("  {} {}, {}", mov, src, dst);
          deallocate(store.getSrc());
          return;
        }
        case 2: // Load
        {
          auto& load = std::get<2>(instruction);
          const auto& type = load.getDst().getType();
          Storage reg = allocate_register(load.getDst().getType());

          std::string mov = form(type);
          std::string src = form(load.getSrc());
          std::string dst = reg.toString();

          append("  {} {}, {}", mov, src, dst);
          storage[load.getDst().getId()] = reg;
          return;
        }
        case 3: // Convert
        {
          todo();
        }
        case 4: // BinOp
        {
          todo();
        }
        case 5: // UnOp
        {
          todo();
        }
      }
    }
    void generate_params(const std::vector<Slot>& params)
    {
      static constexpr std::array<std::string_view, 6> integer_regs = {
        "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
      };
      static constexpr std::array<std::string_view, 8> float_regs = {
        "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7"
      };

      size_t integer_index = 0;
      size_t float_index = 0;
      off_t stack_params_offset = 16;

      for (const auto& param : params)
      {
        const std::string mov = form(param.getType());

        // the destination
        offset += param.getType().getByteSize();
        Memory mem(param.getType(), offset);

        const bool is_integer = param.getType().isInteger();
        size_t& index = is_integer ? integer_index : float_index;
        const size_t end = is_integer ? integer_regs.size() : float_regs.size();

        if (index < end)
        {
          const std::string_view& src = is_integer ? integer_regs[index++] : float_regs[index++];
          append("  {} {}, {}", mov, src, mem.toString());
        }
        else
        {
          Storage temp = make_temp(param.getType());
          const std::string temp_form = temp.toString();

          append("  {} {}(%rbp), {}", mov, stack_params_offset, temp_form);
          append("  {} {}, {}", mov, temp_form, mem.toString());

          stack_params_offset += param.getType().getByteSize();
        }

        storage[param.getId()].setValue<Memory>(mem);
      }
    }
    void generate_function(const Function& fn)
    {
      if (!fn.isDefined())
        return; // do nothing

      size_t total_registers = fn.getTotalRegisters();
      size_t capacity = storage.max_load_factor() * storage.bucket_count();
      if (total_registers > capacity)
        storage.reserve(total_registers - capacity);

      const std::string& name = fn.getName();
      append(".globl {}", name);
      append(".type {}, @function", name);
      append("{}:", name);

      // prologue
      append("  pushq %rbp");
      append("  movq %rsp, %rbp");
      offset = 0;

      generate_params(fn.getParams());

      const auto& body = fn.getBody();
      for (const auto& instruction : body)
        generate_instruction(instruction);

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

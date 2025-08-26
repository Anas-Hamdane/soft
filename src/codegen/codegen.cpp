#include "common.h"
#include "codegen/codegen.h"
#include "codegen/Storage.h"
#include "codegen/DataLabel.h"
#include <cassert>

#define appendln(fmt, ...) out += std::format(fmt "\n" __VA_OPT__(,) __VA_ARGS__) 
#define append(fmt, ...) out += std::format(fmt __VA_OPT__(,) __VA_ARGS__) 

namespace soft {
  namespace codegen {
    // to track reserved and unreserved registers
    // NOTE: the order is crucial since we access them by
    // the int value of the enum class Type::Knd which is equal
    // to the index of it's value in this array.
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

    std::vector<DataLabel> labels;
    std::unordered_map<double, DataLabel*> double_labels;
    std::unordered_map<float, DataLabel*> float_labels;

    std::string out;
    size_t offset;

    void deallocate(const Register& register_v)
    {
      pool[static_cast<int>(register_v.getKnd())].second = false;
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

    DataLabel floating_point_label(const Constant& constant)
    {
      // float
      if (constant.getType().isFloatingPoint(32))
      {
        float value = (double) constant.getFloatValue();

        if (float_labels.find(value) != float_labels.end())
          return *float_labels[value];

        DataLabel label(std::format(".F32N{}", float_labels.size()), {Data(constant)});
        labels.push_back(label);
        float_labels[value] = &labels.back();
        return label;
      }
      // double
      else if (constant.getType().isFloatingPoint(64))
      {
        double value = constant.getFloatValue();

        if (double_labels.find(value) != double_labels.end())
          return *double_labels[value];

        DataLabel label(std::format(".F64N{}", double_labels.size()), {Data(constant)});
        labels.push_back(label);
        double_labels[value] = &labels.back();
        return label;
      }
      unreachable();
    }
    // to string functions
    std::string movts(const Type& type)
    {
      std::string mov = "mov";
      if (type.isFloatingPoint())
        mov += 's';

      mov += suffix(type);
      return mov;
    }
    std::string constantts(const Constant& constant)
    {
      if (constant.isIntegerValue())
        return std::format("${}", constant.getIntegerValue());

      else if (constant.isFloatValue())
      {
        DataLabel label = floating_point_label(constant);
        return std::format("{}(%rip)", label.getName());
      }

      unreachable();
    }
    std::string valuets(const Value& value)
    {
      if (value.isConstant())
        return constantts(value.getConstant());
      else if (value.isSlot())
        return storage[value.getSlot().getId()].toString();

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
    Register allocate_register(const Type& type)
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
          return Register(type, pool[i].first);
        }
      }

      // I don't know
      todo();
    }
    void load_constant(const Constant& constant, const Register& dst)
    {
      appendln("  {} {}, {}", movts(dst.getType()), constantts(constant), dst.toString());
    }
    void load_register(const Register& reg, const Register& dst)
    {
      appendln("  {} {}, {}", movts(dst.getType()), reg.toString(), dst.toString());
    }
    void load_memory(const Memory& mem, const Register& dst)
    {
      appendln("  {} {}, {}", movts(dst.getType()), mem.toString(), dst.toString());
    }

    // NOTE: `Constant` source values are not allowed and should
    // be handled in the IR phase
    void int2int(Slot& src, Slot& dst)
    {
      Type& sty = src.getType(); // src type
      Type& dty = dst.getType(); // dst type

      if (!sty.isInteger() || !dty.isInteger())
        return;

      if (dty.cmpTo(sty))
        return;


      // the source storage
      // WARN: we assume there's only a `Constant` and a `Slot`
      // possibilities for the `Value`
      Storage& ss = storage[src.getId()];

      // in this case if the src is a variable in `Memory` we should load
      // it into a register of the same size(bitwidth) and change the bitwidth
      // of the `dst` to match the given destination type bitwidth, so it will
      // be used in the next instruction with that new size
      if (sty.getBitwidth() > dty.getBitwidth())
      {
        Storage ds;
        if (ss.isMemory())
        {
          ds = allocate_register(sty);
          appendln("  {} {}, {}", movts(sty), ss.toString(), ds.toString());
        }
        else // if (ss.isRegister())
        {
          ds = ss;
        }

        ds.getType().setBitwidth(dty.getBitwidth());
        storage[dst.getId()] = ds;
      }
      else
      {
        // mov[s|z][x][x]
        std::string mov = "mov";
        if (ss.getType().isSigned()) mov += 's';
        else mov += 'z';
        mov += suffix(sty);
        mov += suffix(dty);

        Storage ds;
        if (ss.isMemory())
        {
          ds = allocate_register(dty);
          appendln("  {} {}, {}", mov, ss.toString(), ds.toString());
        }
        else // if (ss.isRegister())
        {
          ds = ss;
          ds.getType().setBitwidth(dty.getBitwidth());
          appendln("  {} {}, {}", mov, ss.toString(), ds.toString());
        }

        storage[dst.getId()] = ds;
      }
    }
    void int2float(Slot& src, Slot& dst)
    {
      Type& sty = src.getType(); // src type
      Type& dty = dst.getType(); // dst type

      if (!sty.isInteger() || !dty.isFloatingPoint())
        return;

      // since cvtsi2s[s|d]x requires a doubleword or a quad word source
      if (sty.getBitwidth() < 32)
      {
        // make a temporary destination with an "Integer" type
        // to pass it to `int2int_cast()` and re-assign it to the
        // `src` to convert it to a floating point.
        Slot tmp_dst = dst;
        tmp_dst.setType(sty);

        int2int(src, tmp_dst);
        src = tmp_dst;
      }

      // cvtsi2s[s|d][l|q]
      std::string cvt = std::format("cvtsi2s{}{}", suffix(dty), suffix(sty));

      Storage ds = allocate_register(dty); // destination storage
      Storage ss = storage[src.getId()]; // src storage
      appendln("  {} {}, {}", cvt, ss.toString(), ds.toString());

      storage[dst.getId()] = ds;
    }
    void float2int(Slot& src, Slot& dst)
    {
      Type& sty = src.getType(); // src type
      Type& dty = dst.getType(); // dst type

      if (!sty.isFloatingPoint() || !dty.isInteger())
        return;

      size_t original_bitwidth = dty.getBitwidth();
      if (dty.getBitwidth() < 32) // min
        dty.setBitwidth(32);

      Storage ss = storage[src.getId()]; // src storage
      Storage ds = allocate_register(dty); // dst storage

      std::string cvt = std::format("cvts{}2si", suffix(sty));
      appendln("  {} {}, {}", cvt, ss.toString(), ds.toString());

      // restore original size
      if (dty.getBitwidth() != original_bitwidth)
        dty.setBitwidth(32);

      storage[dst.getId()] = ds;
    }
    void float2float(Slot& src, Slot& dst)
    {
      Type& sty = src.getType(); // src type
      Type& dty = dst.getType(); // dst type

      if (!sty.isFloatingPoint() || !dty.isFloatingPoint())
        return;

      Storage ss = storage[src.getId()]; // src storage
      Storage ds; // dst storage

      // reuse the same register
      if (ss.isRegister())
      {
        ds = ss;
        ds.getType().setBitwidth(dty.getBitwidth());
      }
      // allocate a new register
      else
      {
        ds = allocate_register(dty);
      }

      std::string cvt = std::format("cvts{}2s{}", suffix(sty), suffix(dty));
      appendln("  {} {}, {}", cvt, ss.toString(), ds.toString());

      storage[dst.getId()] = ds;
    }

    void generate_instruction(Instruction& instruction)
    {
      switch (instruction.index())
      {
        case 0: // Alloca
        {
          const auto& alloca = std::get<0>(instruction);
          offset += alloca.getType().getByteSize();

          Memory mem(alloca.getType(), offset);
          storage[alloca.getDst().getId()].setValue(mem);

          return;
        }
        case 1: // Store
        {
          const auto& store = std::get<1>(instruction);
          const auto& value = store.getSrc();

          std::optional<Storage> ss; // src storage
          if (value.isSlot())
          {
            Storage stored = storage[value.getSlot().getId()];

            if (stored.isMemory())
            {
              ss = allocate_register(store.getDst().getType());
              load_memory(stored.getMemory(), ss->getRegister());
            }
            else // if (stored.isRegister())
            {
              ss = stored.getRegister();
            }
          }
          else if (value.isConstant() && value.getType().isFloatingPoint())
          {
            ss = allocate_register(store.getDst().getType());
            load_constant(value.getConstant(), ss->getRegister());
          }

          std::string mov = movts(store.getDst().getType());
          std::string dst = storage[store.getDst().getId()].toString();

          std::string src;
          if (ss.has_value()) src = ss->toString();
          else src = valuets(value);
          appendln("  {} {}, {}", mov, src, dst);

          if (ss.has_value() && ss->isRegister())
            deallocate(ss->getRegister());

          return;
        }
        case 2: // Convert
        {
          auto& convert = std::get<2>(instruction);

          if (!convert.getSrc().isSlot())
          {
            std::println("Error: the source should be a valid Slot when Casting");
            exit(1);
          }

          Slot& src = convert.getSrc().getSlot();
          Slot& dst = convert.getDst();

          const Type& sty = src.getType();
          const Type& dty = dst.getType();

          // int to int
          if (sty.isInteger() && dty.isInteger())
            return int2int(src, dst);

          // int to float
          else if (sty.isInteger() && dty.isFloatingPoint())
            return int2float(src, dst);

          // float to int
          else if (sty.isFloatingPoint() && dty.isInteger())
            return float2int(src, dst);

          // float to float
          else if (sty.isFloatingPoint() && dty.isFloatingPoint())
            return float2float(src, dst);

          // idk
          else
            unreachable();
        }
        case 3: // BinOp
        {
          todo();
        }
        case 4: // UnOp
        {
          todo();
        }
      }
      unreachable();
    }
    void generate_data(const std::vector<DataLabel>& data)
    {
      for (const auto& elm : data)
      {
        appendln();
        append("{}", elm.toString());
      }
    }
    void generate_terminator(const Return& terminator)
    {
      Register return_register;
      Register::Knd return_knd;

      if (terminator.getType().isInteger())
        return_knd = Register::Knd::RAX;
      else // if (terminator.getType().isFloatingPoint())
        return_knd = Register::Knd::XMM0;

      return_register.setType(terminator.getType());
      return_register.setKnd(return_knd);

      const Value& value = terminator.getValue();

      // constants
      if (value.isConstant())
      {
        load_constant(value.getConstant(), return_register);
      }

      // slots
      else if (value.isSlot())
      {
        const Storage& stored = storage[value.getSlot().getId()];

        if (stored.isMemory())
          load_memory(stored.getMemory(), return_register);
        else if (stored.isRegister() && stored.getRegister().getKnd() != return_knd)
          load_register(stored.getRegister(), return_register);
      }

      appendln("  popq %rbp");
      appendln("  ret");
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
        const std::string mov = movts(param.getType());

        // the destination
        offset += param.getType().getByteSize();
        Memory mem(param.getType(), offset);

        const bool is_integer = param.getType().isInteger();
        size_t& index = is_integer ? integer_index : float_index;
        const size_t end = is_integer ? integer_regs.size() : float_regs.size();

        if (index < end)
        {
          const std::string_view& src = is_integer ? integer_regs[index++] : float_regs[index++];
          appendln("  {} {}, {}", mov, src, mem.toString());
        }
        else
        {
          Storage temp = make_temp(param.getType());
          const std::string temp_form = temp.toString();

          appendln("  {} {}(%rbp), {}", mov, stack_params_offset, temp_form);
          appendln("  {} {}, {}", mov, temp_form, mem.toString());

          stack_params_offset += param.getType().getByteSize();
        }

        storage[param.getId()].setValue(mem);
      }
    }
    void generate_function(Function& fn)
    {
      if (!fn.isDefined())
        return; // do nothing

      size_t total_registers = fn.getTotalRegisters();
      size_t capacity = storage.max_load_factor() * storage.bucket_count();
      if (total_registers > capacity)
        storage.reserve(total_registers - capacity);

      const std::string& name = fn.getName();
      appendln(".globl {}", name);
      appendln(".type {}, @function", name);
      appendln("{}:", name);

      // prologue
      appendln("  pushq %rbp");
      appendln("  movq %rsp, %rbp");
      offset = 0;

      generate_params(fn.getParams());

      auto& body = fn.getBody();
      for (auto& instruction : body)
        generate_instruction(instruction);

      generate_terminator(fn.getTerminator());

      if (!labels.empty())
        generate_data(labels);
    }
    std::string generate(Program& program)
    {
      appendln("# Program: {}", program.getName());
      appendln(".section .text\n");

      auto& functions = program.getFunctions();
      for (auto& fn : functions)
        generate_function(fn);

      return out;
    }
  }
}

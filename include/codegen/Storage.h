#pragma once

#include "stl.h"
#include "data/Type.h"
#include <cstddef>

namespace soft {
  namespace codegen {
    class Memory {
      public:
        Memory(Type type, size_t offset);
        Memory();

        Type& getType();
        const Type& getType() const;
        size_t getOffset() const;

        void setType(Type type);
        void setOffset(size_t offset);

        std::string toString() const;

      private:
        Type type;
        size_t offset;
    };
    class Register {
      public:
        enum class Knd : int {
          RAX = 0, RCX, RDX, RSI, RDI,
          R8, R9, R10, R11, XMM0 , XMM1,
          XMM2, XMM3, XMM4, XMM5, XMM6,
          XMM7, XMM8, XMM9, XMM10, XMM11,
          XMM12, XMM13, XMM14, XMM15
        };

        Register(Type type, Knd knd);
        Register();

        Type& getType();
        const Type& getType() const;
        Knd getKnd() const;

        void setType(Type type);
        void setKnd(Knd knd);

        std::string toString() const;

      private:
        Type type;
        Knd knd;
    };
    class Storage {
      public:
        Storage(Memory value);
        Storage(Register value);
        Storage();

        bool isMemory() const;
        bool isRegister() const;
        // Returns:
        //   0: if the value inside is of type `Memory`
        //   1: if the value inside is of type `Register`
        size_t getIndex() const;

        Memory& getMemory();
        Register& getRegister();
        Type& getType();

        const Memory& getMemory() const;
        const Register& getRegister() const;
        const Type& getType() const;

        void setValue(Memory value);
        void setValue(Register value);

        std::string toString() const;

      private:
        std::variant<Memory, Register> value;
    };
  }
}

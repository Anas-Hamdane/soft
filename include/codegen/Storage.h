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

        std::string toString();

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

        std::string toString();

      private:
        Type type;
        Knd knd;
    };
    class Storage {
      public:
        Storage(std::variant<Memory, Register> value);
        Storage();

        bool isMemory() const;
        bool isRegister() const;

        size_t getIndex() const;
        template<size_t idx>
        requires(idx == 0 || idx == 1)
        auto& getValue() { return std::get<idx>(value); }

        template<size_t idx>
        requires(idx == 0 || idx == 1)
        const auto& getValue() const { return std::get<idx>(value); }

        Type& getType();
        const Type& getType() const;

        template<typename T>
        requires(std::same_as<T, Memory> || std::same_as<T, Register>)
        void setValue(T value) { this->value = std::move(value); }

        std::string toString();

      private:
        std::variant<Memory, Register> value;
        Type* type = nullptr;
    };
  }
}

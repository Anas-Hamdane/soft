#include "codegen/Storage.h"
#include "common.h"

namespace soft {
  namespace codegen {
    Memory::Memory(Type type, size_t offset)
      : type(std::move(type)), offset(offset) {}
    Memory::Memory() = default;

    Type& Memory::getType() { return this->type; }
    const Type& Memory::getType() const { return this->type; }
    size_t Memory::getOffset() const { return this->offset; }

    void Memory::setType(Type type) { this->type = std::move(type); }
    void Memory::setOffset(size_t offset) { this->offset = offset; }

    std::string Memory::toString() const { return std::format("-{}(%rbp)", this->offset);}

    Register::Register(Type type, Knd knd)
      : type(std::move(type)), knd(knd) {}
    Register::Register() = default;

    Type& Register::getType() { return this->type; }
    const Type& Register::getType() const { return this->type; }
    Register::Knd Register::getKnd() const { return this->knd; }

    void Register::setType(Type type) { this->type = std::move(type); }
    void Register::setKnd(Knd knd) { this->knd = knd; }

    std::string Register::toString() const 
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

      size_t index = static_cast<int>(this->knd);
      if (index >= 9)
        return "%" + xmms[index - 9];

      switch (this->type.getByteSize())
      {
        case 1: return "%" + gpr8[index];
        case 2: return "%" + gpr16[index];
        case 4: return "%" + gpr32[index];
        case 8: return "%" + gpr64[index];
      }
      unreachable();
    }

    Storage::Storage(Memory value)
      : value(std::move(value)) {}
    Storage::Storage(Register value)
      : value(std::move(value)) {}
    Storage::Storage() = default;

    bool Storage::isMemory() const { return getIndex() == 0; }
    bool Storage::isRegister() const { return getIndex() == 1; }
    size_t Storage::getIndex() const { return this->value.index(); }

    Memory& Storage::getMemory() { return std::get<0>(this->value); }
    Register& Storage::getRegister() { return std::get<1>(this->value); }
    Type& Storage::getType()
    {
      if (isMemory()) return getMemory().getType();
      else if (isRegister()) return getRegister().getType();

      unreachable();
    }

    const Memory& Storage::getMemory() const { return std::get<0>(this->value); }
    const Register& Storage::getRegister() const { return std::get<1>(this->value); }
    const Type& Storage::getType() const
    {
      if (isMemory()) return getMemory().getType();
      else if (isRegister()) return getRegister().getType();

      unreachable();
    }

    void Storage::setValue(Memory value) { this->value = std::move(value); }
    void Storage::setValue(Register value) { this->value = std::move(value); }

    std::string Storage::toString() const
    {
      if (isRegister()) return this->getRegister().toString();
      else return this->getMemory().toString();
    }
  }
}

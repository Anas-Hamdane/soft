#include "data/Type.h"
namespace soft {
  Type::Type(Knd knd, size_t bitwidth, bool signd)
    : knd(knd), bitwidth(bitwidth), signd(signd) {}
  Type::Type() = default;

  bool Type::isSigned() const { return this->signd; }
  bool Type::isVoid() const { return this->knd == Knd::Void; }

  bool Type::isInteger() const { return this->knd == Knd::Integer; }
  bool Type::isInteger(size_t bitwidth) const { return isInteger() && this->bitwidth == bitwidth; }

  bool Type::isFloatingPoint() const { return this->knd == Knd::Float; }
  bool Type::isFloatingPoint(size_t bitwidth) const { return isFloatingPoint() && this->bitwidth == bitwidth; }

  size_t Type::getBitwidth() const { return this->bitwidth; }
  Type::Knd Type::getKnd() const { return this->knd; }
  size_t Type::getByteSize() const { return this->bitwidth / 8; }

  void Type::setSigned(bool signd) { this->signd = signd; }
  void Type::setBitwidth(size_t bitwidth) { this->bitwidth = bitwidth; }
  void Type::setKnd(Knd knd) { this->knd = knd; }

  bool Type::cmpTo(const Type& type) const { return cmpBitwidth(type.getBitwidth()) && cmpKnd(type.getKnd()); }
  bool Type::cmpBitwidth(const size_t bitwidth) const { return this->bitwidth == bitwidth; }
  bool Type::cmpKnd(const Knd knd) const { return this->knd == knd; }
}

#include "data/Constant.h"

namespace soft {
  Constant::Constant(Type type, int64_t value)
    : type(std::move(type)), value(value) {}
  Constant::Constant(Type type, double value)
    : type(std::move(type)), value(value) {}
  Constant::Constant() = default;

  bool Constant::isIntegerValue() const { return getIndex() == 0; }
  bool Constant::isFloatValue() const { return getIndex() == 1; }
  // Returns:
  //   0: if the value inside is of type `int64_t`
  //   1: if the value inside is of type `double`
  size_t Constant::getIndex() const { return this->value.index(); }

  int64_t Constant::getIntegerValue() const { return std::get<0>(this->value); }
  double Constant::getFloatValue() const { return std::get<1>(this->value); }
  Type& Constant::getType() { return this->type; }
  const Type& Constant::getType() const { return this->type; }

  void Constant::setValue(int64_t value) { this->value = value; }
  void Constant::setValue(double value) { this->value = value; }
  void Constant::setType(Type type) { this->type = std::move(type); }
}

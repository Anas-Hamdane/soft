#include "data/Constant.h"

namespace soft {
  Constant::Constant(Type type, std::variant<int64_t, double> value)
    : type(std::move(type)), value(std::move(value)) {}
  Constant::Constant() = default;

  Type& Constant::getType() { return this->type; }
  const Type& Constant::getType() const { return this->type; }

  size_t Constant::getIndex() const { return this->value.index(); }
  void Constant::setType(Type type) { this->type = std::move(type); }
}

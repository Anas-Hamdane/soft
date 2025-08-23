#include "data/Value.h"
#include "common.h"

namespace soft {
  Value::Value(std::variant<Constant, Slot> value)
    : value(std::move(value)) {}
  Value::Value() = default;

  bool Value::isConstant() const { return getIndex() == 0; }
  bool Value::isSlot() const { return getIndex() == 1; }

  Type& Value::getType()
  {
    if (this->type)
      return *this->type;

    switch (this->value.index())
    {
      case 0:
        this->type = &std::get<0>(this->value).getType();
      case 1:
        this->type = &std::get<1>(this->value).getType();
    }

    return *this->type;
  }
  const Type& Value::getType() const
  {
    if (this->type)
      return *this->type;

    switch (this->value.index())
    {
      case 0: return std::get<0>(this->value).getType();
      case 1: return std::get<1>(this->value).getType();
    }

    unreachable();
  }
  size_t Value::getIndex() const { return this->value.index(); }

  void Value::setType(Type type) { *this->type = std::move(type); };
}

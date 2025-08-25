#include "data/Value.h"
#include "common.h"

namespace soft {
  Value::Value(Constant value)
    : value(std::move(value)) {}
  Value::Value(Slot value)
    : value(std::move(value)) {}
  Value::Value() = default;

  bool Value::isConstant() const { return getIndex() == 0; }
  bool Value::isSlot() const { return getIndex() == 1; }
  size_t Value::getIndex() const { return this->value.index(); }

  Constant& Value::getConstant() { return std::get<0>(this->value); }
  Slot& Value::getSlot() { return std::get<1>(this->value); }
  Type& Value::getType()
  {
    if (this->type)
      return *this->type;

    switch (this->value.index())
    {
      case 0: this->type = &std::get<0>(this->value).getType(); break;
      case 1: this->type = &std::get<1>(this->value).getType(); break;
      default: unreachable();
    }

    return *this->type;
  }
  const Constant& Value::getConstant() const { return std::get<0>(this->value); }
  const Slot& Value::getSlot() const { return std::get<1>(this->value); }
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

  void Value::setValue(Constant value) { this->value = value; }
  void Value::setValue(Slot value) { this->value = value; }
}

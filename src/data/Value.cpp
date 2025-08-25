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
    if (isConstant()) return getConstant().getType();
    else if (isSlot()) return getSlot().getType();

    unreachable();
  }
  const Constant& Value::getConstant() const { return std::get<0>(this->value); }
  const Slot& Value::getSlot() const { return std::get<1>(this->value); }
  const Type& Value::getType() const
  {
    if (isConstant()) return getConstant().getType();
    else if (isSlot()) return getSlot().getType();

    unreachable();
  }

  void Value::setValue(Constant value) { this->value = value; }
  void Value::setValue(Slot value) { this->value = value; }
}

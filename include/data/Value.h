#pragma once

#include "Constant.h"
#include "Slot.h"

namespace soft {
  class Value {
    public:
      Value(std::variant<Constant, Slot> value);
      Value();

      bool isConstant() const;
      bool isSlot() const;

      Type& getType();
      const Type& getType() const;

      template<size_t idx>
      requires (idx == 0 || idx == 1)
      auto& getValue() { return std::get<idx>(value); }
      template<size_t idx>
      requires (idx == 0 || idx == 1)
      const auto& getValue() const { return std::get<idx>(value); }

      // Returns:
      //   0 if the value inside is a `Constant` object.
      //   1 if the value inside is a `Slot` object.
      size_t getIndex() const;

      void setType(Type type);
      template<typename T>
      requires(std::same_as<T, Constant> || std::same_as<T, Slot>)
      void setValue(T value) { this->value = std::move(value); }

    private:
      Type* type;
      std::variant<Constant, Slot> value;
  };
}

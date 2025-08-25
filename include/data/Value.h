#pragma once

#include "Constant.h"
#include "Slot.h"

namespace soft {
  class Value {
    public:
      Value(Constant value);
      Value(Slot value);
      Value();

      bool isConstant() const;
      bool isSlot() const;
      // Returns:
      //   0 if the value inside is a `Constant` object.
      //   1 if the value inside is a `Slot` object.
      size_t getIndex() const;

      Constant& getConstant();
      Slot& getSlot();
      Type& getType();
      const Constant& getConstant() const;
      const Slot& getSlot() const;
      const Type& getType() const;

      void setValue(Constant value);
      void setValue(Slot value);

    private:
      std::variant<Constant, Slot> value;
      Type* type = nullptr;
  };
}

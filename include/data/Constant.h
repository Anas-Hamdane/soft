#pragma once

#include "stl.h"
#include "Type.h"

namespace soft {
  class Constant {
    public:
      Constant(Type type, int64_t value);
      Constant(Type type, double value);
      Constant();

      bool isIntegerValue() const;
      bool isFloatValue() const;
      // Returns:
      //   0: if the value inside is of type `int64_t`
      //   1: if the value inside is of type `double`
      size_t getIndex() const;

      int64_t getIntegerValue() const;
      double getFloatValue() const;
      Type& getType();

      const Type& getType() const;

      void setValue(int64_t value);
      void setValue(double value);
      void setType(Type type);

    private:
      Type type;
      std::variant<int64_t, double> value;
  };
}

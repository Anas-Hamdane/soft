#pragma once

#include "stl.h"
#include "Type.h"

namespace soft {
  class Constant {
    public:
      Constant(Type type, std::variant<int64_t, double> value);
      Constant();

      Type& getType();
      const Type& getType() const;

      template<size_t idx>
      requires (idx == 0 || idx == 1)
      auto& getValue() { return std::get<idx>(value); }

      template<size_t idx>
      requires (idx == 0 || idx == 1)
      const auto& getValue() const { return std::get<idx>(value); }

      // Returns:
      //   0: if the value inside is of type `int64_t`
      //   1: if the value inside is of type `double`
      size_t getIndex() const;

      void setType(Type type);

      template<typename T>
      requires(std::same_as<T, int64_t> || std::same_as<T, double>)
      void setValue(T value) { this->value = std::move(value); }

    private:
      Type type;
      std::variant<int64_t, double> value;
  };
}

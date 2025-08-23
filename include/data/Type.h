#pragma once

#include <cstddef>

namespace soft {
  class Type {
    public:
      enum class Knd { Integer, Float, Void };
      Type(Knd knd, size_t bitwidth, bool signd = true);
      Type();

      // returns true if the integer value is signed
      bool isSigned() const;
      // returns true if the kind is void whatever the bitwidth is
      bool isVoid() const;

      // return true if it's an integer
      bool isInteger() const;
      // return true if it's an integer with a specific bitwidth
      bool isInteger(size_t bitwidth) const;

      // returns true if it's a floating point
      bool isFloatingPoint() const;
      // returns true if it's a floating point with a specific bitwidth
      bool isFloatingPoint(size_t bitwidth) const;

      size_t getBitwidth() const;
      Knd getKnd() const;
      size_t getByteSize() const;

      void setSigned(bool signd);
      void setBitwidth(size_t bitwidth);
      void setKnd(Knd knd);

      // return true if the type given has the same kind and bitwidth
      bool cmpTo(const Type& type) const;
      // returns true if the bitwidth is equal to this object's bitwidth
      bool cmpBitwidth(const size_t bitwidth) const;
      // returns true if the knd is equal to this object's knd
      bool cmpKnd(const Knd knd) const;
    private:
      Knd knd;
      size_t bitwidth;
      bool signd = true;
  };
}

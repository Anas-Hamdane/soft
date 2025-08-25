#pragma once

#include "data/Value.h"

namespace soft {
  class Alloca {
    public:
      Alloca(Type type, Slot dst);

      Type& getType();
      Slot& getDst();

      const Type& getType() const;
      const Slot& getDst() const;

      void setType(Type type);
      void setDst(Slot dst);

    private:
      Type type;
      Slot dst;
  };
  class Store {
    public:
      Store(Value src, Slot dst);

      Value& getSrc();
      Slot& getDst();

      const Value& getSrc() const;
      const Slot& getDst() const;

      void setSrc(Value src);
      void setDst(Slot dst);

    private:
      Value src;
      Slot dst;
  };
  class Convert {
    public:
      Convert(Value src, Slot dst);

      Value& getSrc();
      Slot& getDst();

      const Value& getSrc() const;
      const Slot& getDst() const;

      void setSrc(Value src);
      void setDst(Slot dst);

    private:
      Value src;
      Slot dst;
  };
  class BinOp {
    public:
      enum class Op { Add, Sub, Mul, Div };
      BinOp(Value lhs, Value rhs, Op op, Slot dst);

      Value& getLeft();
      Value& getRight();
      Slot& getDst();
      Op& getOp();

      const Value& getLeft() const;
      const Value& getRight() const;
      const Slot& getDst() const;
      const Op& getOp() const;

      void setLeft(Value lhs);
      void setRight(Value rhs);
      void setDst(Slot dst);
      void setOp(Op op);

    private:
      Value lhs, rhs;
      Slot dst;
      Op op;
  };
  class UnOp {
    public:
      enum class Op { Neg, Not };
      UnOp(Value operand, Slot dst, Op op);

      Value& getOperand();
      Slot& getDst();
      Op& getOp();

      const Value& getOperand() const;
      const Slot& getDst() const;
      const Op& getOp() const;

      void setOperand(Value operand);
      void setDst(Slot dst);
      void setOp(Op op);

    private:
      Value operand;
      Slot dst;
      Op op;
  };
  using Instruction = std::variant<Alloca, Store, Convert, BinOp, UnOp>;
}

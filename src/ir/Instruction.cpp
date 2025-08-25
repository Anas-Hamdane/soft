#include "ir/Instruction.h"

namespace soft {
  Alloca::Alloca(Type type, Slot dst)
    : type(std::move(type)), dst(std::move(dst)) {}

  Type& Alloca::getType() { return this->type; }
  Slot& Alloca::getDst() { return this->dst; }

  const Type& Alloca::getType() const { return this->type; }
  const Slot& Alloca::getDst() const { return this->dst; }

  void Alloca::setType(Type type) { this->type = std::move(type); }
  void Alloca::setDst(Slot dst) { this->dst = std::move(dst); }

  Store::Store(Value src, Slot dst)
    : src(std::move(src)), dst(std::move(dst)) {}

  Value& Store::getSrc() { return this->src; }
  Slot& Store::getDst() { return this->dst; }

  const Value& Store::getSrc() const { return this->src; }
  const Slot& Store::getDst() const { return this->dst; }

  void Store::setSrc(Value src) { this->src = std::move(src); }
  void Store::setDst(Slot dst) { this->dst = std::move(dst); }

  Convert::Convert(Value src, Slot dst)
    : src(std::move(src)), dst(std::move(dst)) {}

  Value& Convert::getSrc() { return this->src; }
  Slot& Convert::getDst() { return this->dst; }

  const Value& Convert::getSrc() const { return this->src; }
  const Slot& Convert::getDst() const { return this->dst; }

  void Convert::setSrc(Value src) { this->src = std::move(src); }
  void Convert::setDst(Slot dst) { this->dst = std::move(dst); }

  BinOp::BinOp(Value lhs, Value rhs, Op op, Slot dst)
    : lhs(std::move(lhs)), rhs(std::move(rhs)), dst(std::move(dst)), op(std::move(op)) {}

  Value& BinOp::getLeft() { return this->lhs; }
  Value& BinOp::getRight() { return this->rhs; }
  Slot& BinOp::getDst() { return this->dst; }
  BinOp::Op& BinOp::getOp() { return this->op; }

  const Value& BinOp::getLeft() const { return this->lhs; }
  const Value& BinOp::getRight() const { return this->rhs; }
  const Slot& BinOp::getDst() const { return this->dst; }
  const BinOp::Op& BinOp::getOp() const { return this->op; }

  void BinOp::setLeft(Value lhs) { this->lhs = std::move(lhs); }
  void BinOp::setRight(Value rhs) { this->rhs = std::move(rhs); }
  void BinOp::setDst(Slot dst) { this->dst = std::move(dst); }
  void BinOp::setOp(Op op) { this->op = std::move(op); }

  UnOp::UnOp(Value operand, Slot dst, Op op)
    : operand(operand), dst(dst), op(op) {}

  Value& UnOp::getOperand() { return this->operand; }
  Slot& UnOp::getDst() { return this->dst; }
  UnOp::Op& UnOp::getOp() { return this->op; }

  const Value& UnOp::getOperand() const { return this->operand; }
  const Slot& UnOp::getDst() const { return this->dst; }
  const UnOp::Op& UnOp::getOp() const { return this->op; }

  void UnOp::setOperand(Value operand) { this->operand = std::move(operand); }
  void UnOp::setDst(Slot dst) { this->dst = std::move(dst); }
  void UnOp::setOp(Op op) { this->op = std::move(op); }
}

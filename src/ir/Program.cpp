#include "ir/Program.h"

namespace soft {
  Return::Return(Type type, Value value)
    : type(std::move(type)), value(std::move(value)) {}
  Return::Return() = default;

  Value& Return::getValue() { return this->value; }
  Type& Return::getType() { return this->type; }

  const Value& Return::getValue() const { return this->value; }
  const Type& Return::getType() const { return this->type; }

  void Return::setType(Type type) { this->type = std::move(type); }
  void Return::setValue(Value value) { this->value = std::move(value); }

  Function::Function(std::string name, Type type, bool defined)
    : name(std::move(name)), type(std::move(type)), defined(defined) {}
  Function::Function() = default;

  bool Function::isDefined() const { return defined; }
  bool Function::isTerminated() const { return this->terminator.has_value(); }

  std::string& Function::getName() { return this->name; }
  Type& Function::getType() { return this->type; }
  Return& Function::getTerminator() { return *this->terminator; }
  std::vector<Slot>& Function::getParams() { return this->params; }
  std::vector<Instruction>& Function::getBody() { return this->body; }
  size_t Function::getTotalRegisters() const { return this->total_registers; }

  const std::string& Function::getName() const { return this->name; }
  const Type& Function::getType() const { return this->type; }
  const Return& Function::getTerminator() const { return *this->terminator; }
  const std::vector<Slot>& Function::getParams() const { return this->params; }
  const std::vector<Instruction>& Function::getBody() const { return this->body; }

  void Function::setName(std::string name)        { this->name = std::move(name); }
  void Function::setType(Type type)               { this->type = std::move(type); }
  void Function::setTerminator(Return terminator) { this->terminator = std::move(terminator); }
  void Function::setParams(std::vector<Slot> params)    { this->params = std::move(params); }
  void Function::setBody(std::vector<Instruction> body) { this->body = std::move(body); }
  void Function::setDefined(bool defined)               { this->defined = defined; }
  void Function::setTotalRegisters(size_t total_registers) { this->total_registers = total_registers; }

  void Function::addParam(Slot param) { this->params.push_back(std::move(param)); }
  void Function::addInstruction(Instruction instruction) { this->body.push_back(std::move(instruction)); }

  Global::Global(std::string name, Type type, Constant init)
    : name(std::move(name)), type(std::move(type)), init(std::move(init)) {}
  Global::Global() = default;

  std::string& Global::getName() { return this->name; }
  Type& Global::getType() { return this->type; }
  Constant& Global::getInit() { return this->init; }

  const std::string& Global::getName() const { return this->name; }
  const Type& Global::getType() const { return this->type; }
  const Constant& Global::getInit() const { return this->init; }

  void Global::setName(std::string name) { this->name = std::move(name); }
  void Global::setType(Type type) { this->type = std::move(type); }
  void Global::setInit(Constant init) { this->init = std::move(init); }

  Program::Program(std::string name)
    : name(std::move(name)) {}
  Program::Program() = default;

  std::string& Program::getName() { return this->name; }
  std::vector<Function>& Program::getFunctions() { return this->functions; }
  std::vector<Global>& Program::getGlobals() { return this->globals; }

  const std::string& Program::getName() const { return this->name; }
  const std::vector<Function>& Program::getFunctions() const { return this->functions; }
  const std::vector<Global>& Program::getGlobals() const { return this->globals; }

  void Program::setName(std::string name) { this->name = std::move(name); }
  void Program::setFunctions(std::vector<Function> functions) { this->functions = std::move(functions); }
  void Program::setGlobals(std::vector<Global> globals) { this->globals = std::move(globals); }

  void Program::addFunction(Function fn) { this->functions.push_back(std::move(fn)); }
  void Program::addGlobal(Global global) { this->globals.push_back(std::move(global)); }
}

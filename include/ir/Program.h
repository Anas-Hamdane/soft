#pragma once

#include "stl.h"
#include "data/Value.h"
#include "Instruction.h"

namespace soft {
  class Return {
    public:
      Return(Type type, Value value = {});
      Return();

      Value& getValue();
      Type& getType();

      const Value& getValue() const;
      const Type& getType() const;

      void setType(Type type);
      void setValue(Value value);

    private:
      Type type;
      Value value;
  };
  class Function {
    public:
      Function(std::string name, Type type, bool defined = false);
      Function();

      bool isDefined() const;
      bool isTerminated() const;

      std::string& getName();
      Type& getType();
      Return& getTerminator();
      std::vector<Slot>& getParams();
      std::vector<Instruction>& getBody();
      size_t getTotalRegisters() const;

      const std::string& getName() const;
      const Type& getType() const;
      const Return& getTerminator() const;
      const std::vector<Slot>& getParams() const;
      const std::vector<Instruction>& getBody() const;

      void setName(std::string name);
      void setType(Type type);
      void setTerminator(Return terminator);
      void setParams(std::vector<Slot> params);
      void setBody(std::vector<Instruction> body);
      void setDefined(bool defined);
      void setTotalRegisters(size_t total_registers);

      void addParam(Slot param);
      void addInstruction(Instruction instruction);

    private:
      std::string name;
      Type type;
      std::optional<Return> terminator;
      std::vector<Slot> params;
      std::vector<Instruction> body;
      size_t total_registers;
      bool defined;
  };
  class Global {
    public:
      Global(std::string name, Type type, Constant init);
      Global();

      std::string& getName();
      Type& getType();
      Constant& getInit();

      const std::string& getName() const;
      const Type& getType() const;
      const Constant& getInit() const;

      void setName(std::string name);
      void setType(Type type);
      void setInit(Constant init);

    private:
      std::string name;
      Type type;
      Constant init;
  };
  class Program {
    public:
      Program(std::string name);
      Program();

      std::string& getName();
      std::vector<Function>& getFunctions();
      std::vector<Global>& getGlobals();

      const std::string& getName() const;
      const std::vector<Function>& getFunctions() const;
      const std::vector<Global>& getGlobals() const;

      void setName(std::string name);
      void setFunctions(std::vector<Function> functions);
      void setGlobals(std::vector<Global> globals);

      void addFunction(Function fn);
      void addGlobal(Global global);

    private:
      std::string name;
      std::vector<Function> functions;
      std::vector<Global> globals;
  };
}

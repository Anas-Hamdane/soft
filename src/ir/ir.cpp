#include "ir/ir.h"
#include "common.h"

namespace soft {
  namespace ir {
    std::unordered_map<std::string, Slot> symbol_table;
    std::unordered_map<std::string, Function*> fns_table;
    std::unordered_map<std::string, Global> globals;
    Function* current_function;
    Program program;
    size_t id;

    Value constant_folding(const Constant& a, BinOp::Op op, const Constant& b)
    {
      auto constant_double_value = [](const Constant& c)
      {
        if (c.isIntegerValue()) return (double) c.getIntegerValue();
        else if (c.isFloatValue()) return c.getFloatValue();
        unreachable();
      };
      auto constant_int64_value = [](const Constant& c)
      {
        if (c.isFloatValue()) return (int64_t) c.getFloatValue();
        else if (c.isIntegerValue()) return c.getIntegerValue();
        unreachable();
      };

      auto calculate_double_constant = [](BinOp::Op op, double l, double r)
      {
        switch (op) {
          case BinOp::Op::Add: return l + r;
          case BinOp::Op::Sub: return l - r;
          case BinOp::Op::Mul: return l * r;
          case BinOp::Op::Div: return l / r;
          default:             unreachable();
        }
      };
      auto calculate_int64_constant = [](BinOp::Op op, int64_t l, int64_t r)
      {
        switch (op) {
          case BinOp::Op::Add: return l + r;
          case BinOp::Op::Sub: return l - r;
          case BinOp::Op::Mul: return l * r;
          case BinOp::Op::Div: return l / r;
          default:             unreachable();
        }
      };

      Constant result;
      Type& type = result.getType();
      type.setBitwidth(std::max(a.getType().getBitwidth(), b.getType().getBitwidth()));

      if (a.getType().isFloatingPoint() || b.getType().isFloatingPoint())
      {
        double av = constant_double_value(a);
        double bv = constant_double_value(b);

        type.setKnd(Type::Knd::Float);
        result.setValue(calculate_double_constant(op, av, bv));
      }
      else
      {
        int64_t av = constant_int64_value(a);
        int64_t bv = constant_int64_value(b);

        type.setKnd(Type::Knd::Integer);
        result.setValue(calculate_int64_constant(op, av, bv));
      }

      return Value(result);
    }
    void constant_cast(Constant& c, const Type& type)
    {
      // different bitwidths
      if (!c.getType().cmpBitwidth(type.getBitwidth()))
        c.getType().setBitwidth(type.getBitwidth());

      // if it's just the bitwidth difference
      // we don't need to cast anything
      if (c.getType().cmpKnd(type.getKnd()))
        return;

      if (c.isIntegerValue()) c.setValue((double) c.getIntegerValue());
      else if (c.isFloatValue()) c.setValue((int64_t) c.getFloatValue());
      else unreachable();

      // casted successfully
      c.getType().setKnd(type.getKnd());
      return;
    }
    void cast(Value& value, const Type& type)
    {
      // types are equal no need to cast
      if (value.getType().cmpTo(type))
        return;

      if (value.isConstant())
        return constant_cast(value.getConstant(), type);

      Slot slot(type, id++);
      current_function->addInstruction( Convert(value, slot) );
      value.setValue(slot);
    }
    Value assign(Value src, Slot dst)
    {
      cast(src, dst.getType());
      current_function->addInstruction( Store(src, dst) );
      return src;
    }
    Value generate_expr(const std::unique_ptr<ast::Expr>& expr)
    {
      switch (expr->index())
      {
        case 0: // IntLit
        {
          auto& lit = std::get<0>(*expr);
          Constant constant;
          constant.setValue((int64_t) lit->v);
          constant.getType().setKnd(Type::Knd::Integer);

          // if the value is bigger than LONG_MAX_VAL
          // it will throw an overflow in parsing
          // therefore it is not possible here
          if (lit->v < (uint64_t) INT_MAX_VAL)
            constant.getType().setBitwidth(4);
          else
            constant.getType().setBitwidth(8);

          return Value(constant);
        }
        case 1: // FloatLit
        {
          auto& lit = std::get<1>(*expr);
          Constant constant;
          constant.setValue((double) lit->v);
          constant.getType().setKnd(Type::Knd::Float);

          // if the value is bigger than LONG_MAX_VAL
          // it will throw an overflow in parsing
          // therefore it is not possible here
          if (lit->v < (uint64_t) FLOAT_MAX_VAL)
            constant.getType().setBitwidth(4);
          else
            constant.getType().setBitwidth(8);

          return Value(constant);
        }
        case 2: // CharLit
        {
          todo();
        }
        case 3: // StrLit
        {
          todo();
        }
        case 4: // ArrLit
        {
          todo();
        }
        case 5: // Identifier
        {
          auto& ide = std::get<5>(*expr);
          if (symbol_table.find(ide->name) == symbol_table.end())
          {
            std::println("Use of undeclared identifier '{}'", ide->name);
            exit(1);
          }

          return Value(symbol_table[ide->name]);
        }
        case 6: // VarDecl
        {
          auto& dec = std::get<6>(*expr);
          if (symbol_table.find(dec->name) != symbol_table.end())
          {
            std::println("Redefinition of variable '{}'", dec->name);
            exit(1);
          }
          if (!dec->type && !dec->init)
          {
            std::println("Either an initializer or a type is required in the declaration of variablle '{}'", dec->name);
            exit(1);
          }

          Type type;
          Value value;
          bool initialized = false;

          if (dec->init)
          {
            value = generate_expr(dec->init);
            type = value.getType();
            initialized = true;
          }

          // override even if there's an initialized
          // Priority goes to the specified type
          if (dec->type)
            type = *dec->type;

          Slot slot = { type, id++ };
          symbol_table[dec->name] = slot;

          current_function->addInstruction( Alloca(type, slot) );

          if (initialized)
            return assign(value, slot);

          return {};
        }
        case 7: // FnCall
        {
          todo();
        }
        case 8: // AssgnOp
        {
          auto& assgn = std::get<8>(*expr);
          Value src = generate_expr(assgn->val);
          Value dst = generate_expr(assgn->var);

          if (!dst.isSlot()) // not a Register
          {
            std::println("Cannot assign to a non-variable");
            exit(1);
          }

          return assign(src, dst.getSlot());
        }
        case 9: // BinOp
        {
          auto& operation = std::get<9>(*expr);
          Value lhs = generate_expr(operation->lhs);
          Value rhs = generate_expr(operation->rhs);

          BinOp::Op op;
          switch (operation->op)
          {
            case Token::Knd::Plus:  op = BinOp::Op::Add; break;
            case Token::Knd::Minus: op = BinOp::Op::Sub; break;
            case Token::Knd::Mul:   op = BinOp::Op::Mul; break;
            case Token::Knd::Div:   op = BinOp::Op::Div; break;
            default:                unreachable();
          }
          
          if (lhs.isConstant() && rhs.isConstant())
            return constant_folding(lhs.getConstant(), op, rhs.getConstant());

          Type lt = lhs.getType();
          Type rt = rhs.getType();

          Slot dst;
          dst.setId(id++);
          dst.getType().setBitwidth(std::max(lt.getBitwidth(), rt.getBitwidth()));

          if (lt.isFloatingPoint() || rt.isFloatingPoint())
            dst.getType().setKnd(Type::Knd::Float);
          else
            dst.getType().setKnd(Type::Knd::Integer);

          cast(lhs, dst.getType());
          cast(rhs, dst.getType());

          current_function->addInstruction( BinOp(lhs, rhs, op, dst) );
          return Value(dst);
        }
        case 10: // UnOp
        {
          auto& operation = std::get<10>(*expr);
          Value operand = generate_expr(operation->oprand);

          UnOp::Op op;
          switch (operation->op) {
            case Token::Knd::Minus: op = UnOp::Op::Neg; break;
            case Token::Knd::Not:   op = UnOp::Op::Not; break;
            default:                unreachable();
          }

          Slot dst(operand.getType(), id++);
          current_function->addInstruction( UnOp(operand, dst, op) );
          return Value(dst);
        }
      }

      unreachable();
    }

    void generate_return(const std::unique_ptr<ast::Return>& stmt)
    {
      if (!current_function)
      {
        std::println(stderr, "`return` outside of a function? are you crazy?");
        exit(1);
      }

      // for now
      if (current_function->getType().isVoid())
      {
        std::println(stderr, "`return` statement inside a void function is not allowed");
        exit(1);
      }

      if (current_function->isTerminated())
        return; // don't do anything

      Value value = generate_expr(stmt->expr);

      // the return value type does not equal to the return type of the function
      if (!value.getType().cmpTo(current_function->getType()))
        cast(value, current_function->getType());

      current_function->setTerminator( Return(value.getType(), value) );
    }
    void generate_fn_dec(const std::unique_ptr<ast::FnDecl>& stmt)
    {
      Function fn(stmt->name, *stmt->type, false);
      symbol_table.clear();
      id = 0;

      for (auto& param : stmt->params)
      {
        if (symbol_table.find(param->name) != symbol_table.end())
        {
          std::println("Redefinition of symbol '{}'", param->name);
          exit(1);
        }
        if (!param->type)
        {
          std::println("parameter type must be specified");
          exit(1);
        }

        Slot slot = { *param->type, id++ };
        fn.addParam(slot);
        symbol_table[param->name] = slot;
      }

      program.addFunction(fn);
      fns_table[fn.getName()] = &program.getFunctions().back();
    }
    void generate_fn_def(const std::unique_ptr<ast::FnDef>& stmt)
    {
      Function fn(stmt->dec->name, *stmt->dec->type, true);

      symbol_table.clear();
      id = 0;
      for (auto& param : stmt->dec->params)
      {
        if (symbol_table.find(param->name) != symbol_table.end())
        {
          std::println("Redefinition of symbol '{}'", param->name);
          exit(1);
        }
        if (!param->type)
        {
          std::println("parameter type must be specified");
          exit(1);
        }

        Slot slot = { *param->type, id++ };
        fn.addParam(slot);
        symbol_table[param->name] = slot;
      }

      current_function = &fn;

      for (auto& stmt : stmt->body)
        generate_stmt(stmt);    

      fn.setTotalRegisters(id);
      program.addFunction(fn);
      fns_table[fn.getName()] = &program.getFunctions().back();
    }
    void generate_stmt(const std::unique_ptr<ast::Stmt>& stmt)
    {
      switch (stmt->index())
      {
        case 0:  generate_return(std::get<0>(*stmt));     break;
        case 1:  generate_expr(std::get<1>(*stmt)->expr); break;
        case 2:  generate_fn_dec(std::get<2>(*stmt));      break;
        case 3:  generate_fn_def(std::get<3>(*stmt));      break;
        default: unreachable();
      }
    }

    Program generate(const std::vector<std::unique_ptr<ast::Stmt>>& ast, std::string program_name)
    {
      program.setName(std::move(program_name));
      current_function = nullptr;
      id = 0;

      for (auto& stmt : ast)
        generate_stmt(stmt);

      return program;
    }
  }
}

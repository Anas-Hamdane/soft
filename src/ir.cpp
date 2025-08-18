#include "ir.h"
#include "common.h"

#define is_float(l) (l.knd == Type::Knd::Float)
#define is_int(l) (l.knd == Type::Knd::Int)

namespace soft {
  namespace ir {
    std::unordered_map<std::string, Register> symbol_table;
    std::unordered_map<std::string, Function*> fns_table;
    std::unordered_map<std::string, Global> globals;
    Function* current_function = nullptr;
    size_t register_id = 0;
    Program program;

    Type value_type(Value v)
    {
      switch (v.index())
      {
        case 0: // Constant
          return std::get<0>(v).type;       
        case 1: // Register
          return std::get<1>(v).type;
        default:
          std::unreachable();
      }
    }
    Constant evaluate_constants(BinOp::Op op, Constant l, Constant r)
    {
      auto constant_double_value = [](Constant c) {
        switch (c.v.index()) {
          case 0: return (double) std::get<0>(c.v);
          case 1: return (double) std::get<1>(c.v);
          case 2: return std::get<2>(c.v);
          default: std::unreachable();
        }
      };
      auto constant_int64_value = [](Constant c) {
        switch (c.v.index()) {
          case 0: return (int64_t) std::get<0>(c.v);
          case 1: return std::get<1>(c.v);
          case 2: return (int64_t) std::get<2>(c.v);
          default: std::unreachable();
        }
      };

      auto calculate_double_constant = [](BinOp::Op op, double l, double r) {
        switch (op) {
          case BinOp::Op::Add: return l + r;
          case BinOp::Op::Sub: return l - r;
          case BinOp::Op::Mul: return l * r;
          case BinOp::Op::Div: return l / r;
          default: std::unreachable();
        }
      };
      auto calculate_int64_constant = [](BinOp::Op op, int64_t l, int64_t r) {
        switch (op) {
          case BinOp::Op::Add: return l + r;
          case BinOp::Op::Sub: return l - r;
          case BinOp::Op::Mul: return l * r;
          case BinOp::Op::Div: return l / r;
          default: std::unreachable();
        }
      };

      Constant result;
      result.type.byte = std::max(l.type.byte, r.type.byte);
      if (is_float(l.type) || is_float(r.type))
      {
        double lv = constant_double_value(l);
        double rv = constant_double_value(r);

        result.type.knd = Type::Knd::Float;
        result.v = calculate_double_constant(op, lv, rv);
      }
      else {
        int64_t lv = constant_int64_value(l);
        int64_t rv = constant_int64_value(r);

        result.type.knd = Type::Knd::Float;
        result.v = calculate_int64_constant(op, lv, rv);
      }

      return result;
    }
    void generate_store(Value src, Register dst) {
      current_function->instrs.push_back(Store { src, dst });
    }
    void cast(Value& v, Type type)
    {
      Type vt = value_type(v);
      if (vt.knd == type.knd && vt.byte == type.byte)
        return;

      Register dst = { type, register_id++ };
      current_function->instrs.push_back( Conv{ v, dst });
      v = dst;
    }
    Value generate_assignment(Value v, Register dst)
    {
      cast(v, dst.type);
      generate_store(v, dst);
      return v;
    }
    Value generate_expr(const std::unique_ptr<ast::Expr>& expr)
    {
      switch (expr->index())
      {
        case 0: // IntLit
        {
          auto& lit = std::get<0>(*expr);
          Constant constant;
          constant.v = lit->v;
          constant.type.knd = Type::Knd::UInt;
          // if the value is bigger than LONG_MAX_VAL
          // it will throw an overflow in parsing
          // therefore it is not possible here
          if (lit->v < (uint64_t) INT_MAX_VAL)
            constant.type.byte = 4;
          else
            constant.type.byte = 8;

          return constant;
        }
        case 1: // FLoatLit
        {
          auto& lit = std::get<1>(*expr);
          Constant constant;
          constant.v = lit->v;
          constant.type.knd = Type::Knd::Int;

          if (lit->v < FLOAT_MAX_VAL)
            constant.type.byte = 4;
          else
            constant.type.byte = 8;

          return constant;
        }
        case 2: // CharLit
        {
          // TODO:
          std::println("todo");
          exit(1);
        }
        case 3: // StrLit
        {
          // TODO:
          std::println("todo");
          exit(1);
        }
        case 4: // ArrLit
        {
          // TODO:
          std::println("todo");
          exit(1);
        }
        case 5: // Identifier
        {
          auto& ide = std::get<5>(*expr);
          if (symbol_table.find(ide->name) != symbol_table.end())
          {
            std::println("Use of undeclared identifier {}", ide->name);
            exit(1);
          }

          return symbol_table[ide->name];
        }
        case 6: // VarDecl
        {
          auto& dec = std::get<6>(*expr);
          if (symbol_table.find(dec->name) != symbol_table.end())
          {
            std::println("Redefinition of variable {}", dec->name);
            exit(1);
          }
          if (!dec->type && !dec->init)
          {
            std::println("Either an initializer or a type is required in variable declaration of {}", dec->name);
            exit(1);
          }

          Type type;
          Value value;
          bool initialized = false;

          if (dec->init) {
            value = generate_expr(dec->init);
            type = value_type(value);
            initialized = true;
          }

          // override even if there's an initialized
          // Priority goes to the specified type
          if (dec->type)
            type = *dec->type;

          Register reg = { type, register_id++ };
          symbol_table[dec->name] = reg;

          Alloca alloca{ .type = type, .reg = reg };
          current_function->instrs.push_back(alloca);

          if (initialized)
            return generate_assignment(value, reg);

          return {};
        }
        case 7: // FnCall
        {
          // TODO:
          std::println("todo");
          exit(1);
        }
        case 8: // AssgnOp
        {
          auto& assgn = std::get<8>(*expr);
          Value val = generate_expr(assgn->val);
          Value var = generate_expr(assgn->var);

          if (var.index() != 1) // not a Register
          {
            std::println("Cannot assign to a non-variable");
            exit(1);
          }

          return generate_assignment(val, std::get<1>(var));
        }
        case 9: // BinOp
        {
          auto& binop = std::get<9>(*expr);
          Value lhs = generate_expr(binop->lhs);
          Value rhs = generate_expr(binop->rhs);

          BinOp::Op op;
          switch (binop->op)
          {
            case Token::Knd::Plus:  op = BinOp::Op::Add; break;
            case Token::Knd::Minus: op = BinOp::Op::Sub; break;
            case Token::Knd::Mul:   op = BinOp::Op::Mul; break;
            case Token::Knd::Div:   op = BinOp::Op::Div; break;
            default:                std::unreachable();
          }
          
          if (lhs.index() == 0 && rhs.index() == 0)
            return evaluate_constants(op, std::get<0>(lhs), std::get<0>(rhs));

          Type lt = value_type(lhs);
          Type rt = value_type(rhs);

          Register dst;
          dst.id = register_id++;
          dst.type.byte = std::max(lt.byte, rt.byte);

          if (is_float(lt) || is_float(rt))
            dst.type.knd = Type::Knd::Float;
          else if (is_int(lt) || is_int(rt))
            dst.type.knd = Type::Knd::Int;
          else
            dst.type.knd = Type::Knd::UInt;

          cast(lhs, dst.type);
          cast(rhs, dst.type);

          current_function->instrs.push_back(BinOp{ op, lhs, rhs, dst });
          return dst;
        }
        case 10: // UnOp
        {
          auto& unop = std::get<10>(*expr);
          Value opr = generate_expr(unop->oprand);

          UnOp::Op op;
          switch (unop->op) {
            case Token::Knd::Minus: op = UnOp::Op::Neg; break;
            case Token::Knd::Not:   op = UnOp::Op::Not; break;
            default:                std::unreachable();
          }

          Register dst;
          dst.id = register_id++;
          dst.type = value_type(opr);

          current_function->instrs.push_back(UnOp{op, opr, dst});
          return dst;
        }
        default: std::unreachable();
      }
    }

    void generate_fndef(const std::unique_ptr<ast::FnDef>& def)
    {
      Function fn;
      fn.name = def->dec->name;
      fn.mod = Function::Mod::Definition;

      if (def->dec->type)
        fn.type = *def->dec->type;

      register_id = 0;
      auto old_table = symbol_table;
      for (auto& param : def->dec->params)
      {
        if (symbol_table.find(param->name) != symbol_table.end())
        {
          std::println("Redefinition of the same symbol");
          exit(1);
        }
        if (!param->type)
        {
          std::println("parameter type must be specified");
          exit(1);
        }

        Register reg = { *param->type, register_id++ };
        fn.params.emplace_back(reg);
        symbol_table[param->name] = reg;
      }

      current_function = &fn;
      for (auto& stmt : def->body)
        generate_stmt(stmt);    

      symbol_table = old_table;
      program.fns.push_back(fn);
      fns_table[fn.name] = &program.fns.back();
    }
    void generate_fndec(const std::unique_ptr<ast::FnDecl>& dec)
    {
      Function fn;
      fn.name = dec->name;
      fn.mod = Function::Mod::Declaration;

      if (dec->type)
        fn.type = *dec->type;

      register_id = 0;
      auto old_table = symbol_table;

      for (auto& param : dec->params)
      {
        if (symbol_table.find(param->name) != symbol_table.end())
        {
          std::println("Redefinition of the same symbol {}", param->name);
          exit(1);
        }
        if (!param->type)
        {
          std::println("parameter type must be specified for {}", param->name);
          exit(1);
        }

        Register reg = { *param->type, register_id++ };
        fn.params.emplace_back(reg);
        symbol_table[param->name] = reg;
      }

      symbol_table = old_table;
      program.fns.push_back(fn);
      fns_table[fn.name] = &program.fns.back();
    }
    void generate_return(const std::unique_ptr<ast::Return>& rt)
    {
      if (!current_function)
      {
        std::println("`return` outside of a function? are you crazy?");
        exit(1);
      }

      if (!current_function->type.has_value() && rt->expr)
      {
        std::println("returning an expression in a void function is not allowed");
        exit(1);
      }

      if (current_function->terminator.has_value())
        return; // don't do anything

      Value v = generate_expr(rt->expr);    
      current_function->terminator->emplace<Return>(v);
    }
    void generate_stmt(const std::unique_ptr<ast::Stmt>& stmt)
    {
      switch (stmt->index())
      {
        case 0:  generate_return(std::get<0>(*stmt));     break;
        case 1:  generate_expr(std::get<1>(*stmt)->expr); break;
        case 2:  generate_fndec(std::get<2>(*stmt));      break;
        case 3:  generate_fndef(std::get<3>(*stmt));      break;
        default: std::unreachable();
      }
    }

    Program generate(const std::vector<std::unique_ptr<ast::Stmt>>& ast)
    {
      for (auto& stmt : ast)
        generate_stmt(std::move(stmt));

      return program;
    }
  }
}

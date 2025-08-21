#include "ir.h"
#include "common.h"
#include <cassert>

#undef alloca
#define is_float(l) (l.knd == Type::Knd::Float)
#define is_int(l) (l.knd == Type::Knd::Int)
#define is_uint(l) (l.knd == Type::Knd::UInt)

namespace soft {
  namespace ir {
    std::unordered_map<std::string, Slot> symbol_table;
    std::unordered_map<std::string, Function*> fns_table;
    std::unordered_map<std::string, Global> globals;
    Function* current_function = nullptr;
    size_t register_id = 0;
    Program program;

    void alloca(const Type& type, const Slot& dst)
    {
      current_function->instrs.push_back( Alloca { type, dst } );
    }
    void store(const Value& src, const Slot& dst)
    {
      current_function->instrs.push_back( Store { src, dst } );
    }
    void load(const Value& src, const Slot& dst)
    {
      current_function->instrs.push_back( Load { src, dst } );
    }
    void conv(const Value& src, const Slot& dst)
    {
      current_function->instrs.push_back( Conv { src, dst } );
    }
    void binop(BinOp::Op op, const Value& lhs, const Value& rhs, const Slot& dst)
    {
     current_function->instrs.push_back( BinOp { op, lhs, rhs, dst } );
    }
    void unop(UnOp::Op op, const Value& opr, const Slot& dst)
    {
      current_function->instrs.push_back( UnOp { op, opr, dst } );
    }

    Type value_type(const Value& v)
    {
      switch (v.index())
      {
        case 0: // Constant
          return std::get<0>(v).type;       
        case 1: // Register
          return std::get<1>(v).type;
        default:
          unreachable();
      }
    }
    Constant evaluate_constants(BinOp::Op op, Constant l, Constant r)
    {
      auto constant_double_value = [](Constant c) {
        switch (c.v.index()) {
          case 0:  return (double) std::get<0>(c.v);
          case 1:  return (double) std::get<1>(c.v);
          case 2:  return std::get<2>(c.v);
          default: unreachable();
        }
      };
      auto constant_int64_value = [](Constant c) {
        switch (c.v.index()) {
          case 0:  return (int64_t) std::get<0>(c.v);
          case 1:  return std::get<1>(c.v);
          case 2:  return (int64_t) std::get<2>(c.v);
          default: unreachable();
        }
      };

      auto calculate_double_constant = [](BinOp::Op op, double l, double r) {
        switch (op) {
          case BinOp::Op::Add: return l + r;
          case BinOp::Op::Sub: return l - r;
          case BinOp::Op::Mul: return l * r;
          case BinOp::Op::Div: return l / r;
          default:             unreachable();
        }
      };
      auto calculate_int64_constant = [](BinOp::Op op, int64_t l, int64_t r) {
        switch (op) {
          case BinOp::Op::Add: return l + r;
          case BinOp::Op::Sub: return l - r;
          case BinOp::Op::Mul: return l * r;
          case BinOp::Op::Div: return l / r;
          default:             unreachable();
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
    void cast_constant(Constant& c, const Type& type)
    {
      if (c.type.byte != type.byte)
        c.type.byte = type.byte;

      // if it's just the byte difference
      // we don't need to cast anything
      if (c.type.knd == type.knd)
        return;

      switch (c.v.index())
      {
        case 0: // uint64_t
        {
          if (is_int(type))
            c.v = (int64_t) std::get<0>(c.v);
          else
            goto float_dst;

          break;
        }
        case 1: // int64_t
        {
          if (is_uint(type))
            c.v = (uint64_t) std::get<1>(c.v);
          else
            goto float_dst;

          break;
        }
        case 2: // double
        {
          if (is_int(type))
            c.v = (int64_t) std::get<2>(c.v);
          else
            c.v = (uint64_t) std::get<2>(c.v);

          break;
        }
      }

      // casted successfully
      c.type.knd = type.knd;
      return;

float_dst:
      todo();
    }
    void cast(Value& src, const Type& type)
    {
      Type vt = value_type(src);
      if (vt.knd == type.knd && vt.byte == type.byte)
        return;

      if (src.index() == 0)
        return cast_constant(std::get<0>(src), type);

      Slot dst = { type, register_id++ };
      conv(src, dst);
      src = dst;
    }
    Value generate_assignment(Value src, Slot dst)
    {
      cast(src, dst.type);

      if (src.index() == 1) // Register
      {
        Slot ldst = { dst.type, register_id++ };
        load(src, ldst); src = ldst;
      }

      store(src, dst);
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
          constant.v = (uint64_t) lit->v;
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
          constant.v = (double) lit->v;
          constant.type.knd = Type::Knd::Float;

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
          if (symbol_table.find(ide->name) == symbol_table.end())
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

          Slot slot = { type, register_id++ };
          symbol_table[dec->name] = slot;

          alloca(type, slot);

          if (initialized)
            return generate_assignment(value, slot);

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
          Value dst = generate_expr(assgn->var);

          if (dst.index() != 1) // not a Register
          {
            std::println("Cannot assign to a non-variable");
            exit(1);
          }

          return generate_assignment(val, std::get<1>(dst));
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
          
          if (lhs.index() == 0 && rhs.index() == 0)
            return evaluate_constants(op, std::get<0>(lhs), std::get<0>(rhs));

          Type lt = value_type(lhs);
          Type rt = value_type(rhs);

          Slot dst;
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

          binop(op, lhs, rhs, dst);
          return dst;
        }
        case 10: // UnOp
        {
          auto& operation = std::get<10>(*expr);
          Value opr = generate_expr(operation->oprand);

          UnOp::Op op;
          switch (operation->op) {
            case Token::Knd::Minus: op = UnOp::Op::Neg; break;
            case Token::Knd::Not:   op = UnOp::Op::Not; break;
            default:                unreachable();
          }

          Slot dst;
          dst.id = register_id++;
          dst.type = value_type(opr);

          unop(op, opr, dst);
          return dst;
        }
        default: unreachable();
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

        Slot reg = { *param->type, register_id++ };
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

        Slot reg = { *param->type, register_id++ };
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
      Type vt = value_type(v);

      if (vt.knd != current_function->type->knd || vt.byte != current_function->type->byte)
        cast(v, *current_function->type);

      current_function->terminator = Return { v };
    }
    void generate_stmt(const std::unique_ptr<ast::Stmt>& stmt)
    {
      switch (stmt->index())
      {
        case 0:  generate_return(std::get<0>(*stmt));     break;
        case 1:  generate_expr(std::get<1>(*stmt)->expr); break;
        case 2:  generate_fndec(std::get<2>(*stmt));      break;
        case 3:  generate_fndef(std::get<3>(*stmt));      break;
        default: unreachable();
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

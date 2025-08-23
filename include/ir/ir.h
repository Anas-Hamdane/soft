#pragma once

#include "parser.h"
#include "Program.h"

namespace soft {
  namespace ir {
    void generate_stmt(const std::unique_ptr<ast::Stmt>& stmt);
    Program generate(const std::vector<std::unique_ptr<ast::Stmt>>& ast, std::string program_name);
  }
}

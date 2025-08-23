#include "stl.h"
#include "opts.h"
#include "file.h"
#include "lexer.h"
#include "parser.h"
#include "ir/ir.h"

using namespace soft;

int main(int argc, char *argv[])
{
  Opts opts = parse_opts(argc, argv);

  if (opts.help || !opts.input_file)
    help(opts.program, !opts.help);

  std::string content = read_file(opts.input_file);
  std::vector<Token> tkns = lexer::lex(content);

  auto ast = ast::generate(tkns);
  Program program = ir::generate(ast, opts.program);

  // std::string code = codegen::generate(program);
  // std::print("{}", code);
  return 0;
}

#include "stl.h"
#include "file.h"
#include "lexer.h"
#include "opts.h"
using namespace soft;

int main(int argc, char *argv[])
{
  Opts opts = parse_opts(argc, argv);

  if (opts.help || !opts.input_file)
    help(opts.program, !opts.help);

  std::string content = read_file(opts.input_file);
  std::vector<Token> tkns = lexer::lex(content);
  lexer::print_tokens(tkns);
  return 0;
}

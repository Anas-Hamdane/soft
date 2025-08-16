#include "opts.h"
#include <cstdio>
#include <cstdlib>
using namespace soft;

#define println(fmt, ...) printf(fmt "\n" __VA_OPT__(,) __VA_ARGS__)

void help(const char* program, int ec = 0)
{
  println("Usage:");
  println("  %s <input> [options..]", program);
  println();
  println("Options:");
  println("  -o <output>   specifies the output file");
  println("  -S            only compile, don't link");
  println();
  println("  --emit-asm    emit assembly into the output file");
  println("  --save-temps  saves the temporary files");
  println("  --help        print this help");
  exit(ec);
}

int main(int argc, char *argv[])
{
  Opts opts = parse_opts(argc, argv);

  if (opts.help || !opts.input_file)
    help(opts.program, !opts.help);

  return 0;
}

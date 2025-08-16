#include "opts.h"
#include <cassert>
#include <string.h>
using namespace soft;

Opts soft::parse_opts(int argc, char *argv[])
{
  Opts opts = 
  {
    .program = {},
    .input_file = {},
    .output_file = {},
    .emit_asm = false,
    .just_compile = false,
    .save_temps = false,
    .help = false,
  };

  opts.program = argv[0];
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-o") == 0) 
    {
      assert(opts.output_file == nullptr);
      opts.output_file = argv[i++];
    }

    else if (strcmp(argv[i], "-S") == 0)
    {
      opts.just_compile = true;
    }

    else if (strcmp(argv[i], "--emit-asm") == 0)
    {
      opts.emit_asm = true;
    }

    else if (strcmp(argv[i], "--save-temps") == 0)
    {
      opts.save_temps = true;
    }

    else if (strcmp(argv[i], "--help") == 0)
    {
      opts.help = true;
    }

    else {
      // multiple input files are not supported for now
      assert(opts.input_file == nullptr);
      opts.input_file = argv[i++];
    }
  }

  return opts;
}

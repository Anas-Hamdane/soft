#include "stl.h"
#include "opts.h"
#include <cassert>
#include <string.h>

namespace soft {
  Opts parse_opts(int argc, char *argv[])
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

  void help(const char* program, int ec)
  {
    std::println("Usage:");
    std::println("  %s <input> [options..]", program);
    std::println();
    std::println("Options:");
    std::println("  -o <output>   specifies the output file");
    std::println("  -S            only compile, don't link");
    std::println();
    std::println("  --emit-asm    emit assembly into the output file");
    std::println("  --save-temps  saves the temporary files");
    std::println("  --help        print this help");
    exit(ec);
  }
}

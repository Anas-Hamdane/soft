#pragma once

#include <string>

namespace soft {
  struct Opts {
    std::string program;
    std::string input_file;
    std::string output_file;

    bool emit_asm; // don't compile
    bool just_compile; // don't link
    bool save_temps; // save .s and .o files
    bool help; // print help and exit
  };

  Opts parse_opts(int argc, char* argv[]);
}

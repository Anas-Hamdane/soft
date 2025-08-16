#pragma once

namespace soft {
  struct Opts {
    char* program;
    char* input_file;
    char* output_file;

    bool emit_asm; // don't compile
    bool just_compile; // don't link
    bool save_temps; // save .s and .o files
    bool help; // print help and exit
  };

  Opts parse_opts(int argc, char* argv[]);
  void help(const char* program, int ec = 0);
}

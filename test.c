#include "stdio.h"
#include "cio.h"
#include "cstring.h"
#include "encoding.h"
#include "crypt.h"
#include "cregex.h"
#include "cargs.h"

char* a[] = {
  "123",
  "abc"
};

int main(int argc, char** argv) {
   
  if (!cargs_parse_args(argc, argv)) {
    cargs_show_help(argc, argv);
  }

  return 1;
}

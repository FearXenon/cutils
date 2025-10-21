#include "cregex.h"

char *regex_match(char *regex, char *string, size_t *n) {

  regex_engine engine = {};

  regex_build_dfa(regex, &engine);

  return 0;
}

void regex_build_dfa(char *regex, regex_engine *engine) {

}

#ifndef CREGEX
#define CREGEX
#include <stdint.h>
#include <stdlib.h>

typedef struct {
 
} regex_node;

typedef struct {
  regex_node *nodes;
  int node_count;
} regex_engine;

char* regex_match(char* regex, char* string, size_t* n);
char* regex_replace(char* search, char* replace, char* string);
void regex_build_dfa(char* regex, regex_engine* engine);
#endif
#ifdef CREGEX_IMPLEMENTATION

char *regex_match(char *regex, char *string, size_t *n) {

  regex_engine engine = {};

  regex_build_dfa(regex, &engine);

  return 0;
}

void regex_build_dfa(char *regex, regex_engine *engine) {

}

#endif
#pragma once
#include <stddef.h>
#include <string.h>

#define START_FN(name, ...) struct { \

#define END_FN(name, ...) } s_##name; \


%type% str_to_%type%(str);

#define ARGS \
  START_FN(test, "this is a test") \
    REQ(double, testdouble) \
    OPT("-o", "--opt", int, i) \
    HLP("-h", "--help", "help") \
  END_FN(test) \
  START_FN(cool_function, "very cool function") \
    OPT("-c", "--cool", int a) \
  END_FN(cool_function) \
  START_FN(help, "some help function") \
    START_FN(1) \
    END_FN(1) \
    START_FN(2) \
    END_FN(2) \
  END_FN(help)

typedef struct args {
  #ifdef ARGS
  ARGS
  #endif
  char test_start;
  struct {
    double testdouble;
    int i;
  } s_test;
  char test_end;
  // ...
  char help_start;
  struct {
    struct {
      int i;
    } s_1;
    struct {
      int j;
    } s_2;
  } help_section;
  char help_end;
} args_t;

static inline int cargs_parse_args(int argc, char* argv[]) {

  #ifdef ARGS
  #undef START_FN
  #undef END_FN
  #define START_FN(...) depth++; do {
  #define END_FN(...) } while(0); depth--;
  ARGS
  #endif
  int i = 1;
  int depth = 0;
  depth++;
  do {
    if (argc <= depth) break;
    if (strcmp("test", argv[depth])) {
      cb(arc, argv, depth);
    }
  } while(0);
  depth--;

  do {
    do {
      
    } while (0);

    do {

    } while(0);
  } while(0);

  return 1;
}

static inline int cargs_show_help(int argc, char** argv) {
  return 1;
}


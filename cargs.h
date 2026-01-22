#pragma once
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cstring.h"

#define CONCAT(a, b) a##b
#define CONCAT2(a, b) CONCAT(a, b)
#define STRUCT_NAME(name, id) CONCAT2(CONCAT(_s_, name), CONCAT(_, id))

#define START_FN(name, id, desc, ...) struct s_##name { 
#define REQ(cmd_hint, type, name, ...) type name;
#define OPT(cmd, cmd_verbose, type, name, ...) type name;
#define FLG(cmd, cmd_verbose, type, name, ...) type name;
#define TXT(...)
#define END_FN(name, ...) } s_##name; 
#define HLP(...)

#define ARG_HANDLER(name, id) cb_##name(struct s_##name *args)

//%type% str_to_%type%(str);

/*  
 * ./binary [function] [subfunction] [required] -[optional] -> subfunction cb
 * ./binary [function] [required] -[optional] -> function cb
 * ./binary [function] -> list function subfunctions (and function use)
 * */

static inline char str_to_int(const char* s, int* i) {
  *i = atoi(s);
  return 1;
}

static inline char str_to_long(const char* s, long* l) {
  *l = atol(s);
  return 1;
}

static inline char str_to_double(const char* s, double* d) {
  *d = strtod(s, (char**)0);
  return 1;
}

static inline char is_opt(const char* s) {
  if (s[0] == '-') {
    if (s[1] == '\0' || (s[1] != '\0' && s[1] >= '0' && s[1] <= '9')) {
      return 0; 
    }
    return 1;
  }
  return 0;
}

static inline char is_opt_with_equals(const char *s, const char *search) {

  if (*s == '\0' || *s != '-')
    return 0;

  while (*s && *search) {
    if (*s != *search)
      return 0;
    s++;
    search++;
  }
  return *search == '\0' && *s != '\0' && *s == '=';
}

static inline char str_starts(const char *s, const char *search) {
  while (*s && *search) {
    if (*s != *search)
      return 0;
    s++;
    search++;
  }
  return *search == '\0';
}

struct args {
#ifdef ARGS
  ARGS
#endif
};

static struct args default_args = {
#ifdef ARGS
#undef START_FN
#undef REQ
#undef OPT
#undef FLG
#undef END_FN

#define START_FN(name, ...) .s_##name = {
#define REQ(...)
#define OPT(cmd, cmd_verbose, type, name, default_val, ...) .name = default_val,
#define FLG(cmd, cmd_verbose, type, name, default_val, ...) .name = default_val,
#define END_FN(name, ...) },
  ARGS
#endif
};

#ifdef ARGS
#undef START_FN
#undef REQ
#undef OPT
#undef FLG
#undef END_FN
#define START_FN(name, desc, id, ...) extern void cb_##name(struct s_##name*);
#define REQ(...)
#define OPT(...)
#define FLG(...)
#define TXT(...)
#define END_FN(...)
ARGS
#endif

static int cargs_show_help(int argc, char** argv);

static inline int cargs_parse_args(int argc, char* argv[]) {

  /* argument count start */
#ifdef ARGS
#undef START_FN
#undef END_FN
#undef OPT
#undef FLG
#undef REQ
#define REQ(...) 
#define OPT(...)
#define FLG(...)
#define TXT(...)
#define START_FN(name, ...) \
  int req_count_##name = 0; \
  int opt_count_##name = 0; \
  int flg_count_##name = 0; \
  int txt_count_##name = 0;
#define REQ(...)
#define END_FN(...)
  ARGS
#endif

#ifdef ARGS
#undef START_FN
#undef END_FN
#undef REQ
#undef TXT
#undef OPT
#undef FLG
#define START_FN(name, ...) \
  do { \
    int req_count = 0; int opt_count = 0; int flg_count = 0; int txt_count = 0;
#define REQ(...) \
    req_count++; 
#define OPT(...) \
    opt_count++;
#define FLG(...) \
    flg_count++;
#define TXT(...) \
    txt_count++;
#define END_FN(name, ...) \
    req_count_##name = req_count; \
    opt_count_##name = opt_count; \
    flg_count_##name = flg_count; \
    txt_count_##name = txt_count; \
  } while (0);
  ARGS
#endif
  /* argument count end*/

  /* argument strings array start */
#ifdef ARGS
#undef START_FN
#undef END_FN
#undef REQ
#undef OPT
#undef FLG
#undef TXT
#define REQ(...)
#define OPT(...)
#define FLG(...)
#define TXT(...)
#define START_FN(name, ...) \
  char *req_arr_##name[req_count_##name]; \
  char *opt_arr_##name[opt_count_##name][2]; \
  char *flg_arr_##name[flg_count_##name][2]; \
  char *txt_arr_##name[txt_count_##name][2];
#define END_FN(...)
  ARGS
#endif

#ifdef ARGS
#undef START_FN
#undef END_FN
#undef REQ
#undef OPT
#undef FLG
#undef TXT
#define START_FN(name, ...) \
  do { \
    int req_i = 0; \
    int opt_i = 0; \
    int flg_i = 0; \
    int txt_i = 0; \
    char **generic_req_arr = (char**)&req_arr_##name[0]; \
    char **generic_opt_arr = (char**)&opt_arr_##name; \
    char **generic_flg_arr = (char**)&flg_arr_##name; \
    char **generic_txt_arr = (char**)&txt_arr_##name; \
     
#define REQ(type, name, cmd_hint, ...) \
    generic_req_arr[req_i] = #cmd_hint; \
    req_i++;
#define OPT(cmd, cmd_verbose, ...) \
    generic_opt_arr[opt_i] = cmd; \
    generic_opt_arr[opt_i + 1] = cmd_verbose; \
    opt_i+=2;
#define FLG(cmd, cmd_verbose, ...) \
    generic_flg_arr[flg_i] = cmd; \
    generic_flg_arr[flg_i + 1] = cmd_verbose; \
    flg_i+=2;
#define TXT(cmd, cmd_verbose, ...)
#define END_FN(name, ...) \
  } while(0);

  ARGS
#endif
  /* argument strings array end */

  /* main logic start */
#ifdef ARGS
#undef START_FN
#undef END_FN
#undef REQ
#undef OPT
#undef TXT
#undef FLG
#undef HLP

#define START_FN(name, id, desc, ...) \
  fn_depth++; \
  do { \
    if (strcmp(#name, "") == 0) fn_depth--; \
    int i = fn_depth + 1; \
    if (argc <= fn_depth) break; \
    if (strcmp(#name, argv[fn_depth]) != 0 && strcmp(#name, "") != 0) break; \
    if (argc <= req_count_##name + fn_depth) { /*return 0;*/ } \
    int req_count = req_count_##name; \
    int opt_count = opt_count_##name; \
    int flg_count = flg_count_##name; \
    int txt_count = txt_count_##name; \
    char req_found_arr[req_count_##name]; \
    for (int l = 0; l < req_count_##name; l++) req_found_arr[l] = 0; \
    int req_found = 0; \
    int opt_found = 0; \
    int flg_found = 0; \
    int txt_found = 0; \
    struct s_##name s_##name = s_generic2.s_##name; \
    struct s_##name s_generic = s_##name; \
    struct s_##name s_generic2 = s_generic; \
    void (*generic_cb)(void*) = (void(*)(void*))&cb_##name; \
    while (i < argc) { \
      char *arg = argv[i]; \
      int req_found_i = 0; 
#define REQ(cmd_hint, type, name, ...) \
      if (req_found < req_count && !is_opt(arg) && req_found_arr[req_found_i] == 0) { \
        type parsed; \
        if (str_to_##type(arg, &parsed)!=0) { \
          s_generic.name = parsed; \
          req_found_arr[req_found_i] = 1; \
          req_found++; \
          i++; \
          continue; \
        } \
      } \
      req_found_i++;
#define OPT(cmd, cmd_verbose, type, name, ...) \
      if (opt_found < opt_count) { \
        type parsed; \
        if (is_opt(arg) && (str_equals(arg, cmd) || str_equals(arg, cmd_verbose))) { \
          if (++i < argc) { \
            if (str_to_##type(argv[i], &parsed)!=0) { \
              s_generic.name = parsed; \
              opt_found++; \
              i++; \
              continue; \
            } \
          } else { \
            printf("OPT without value\n"); \
            break; \
          } \
        } \
        if (is_opt_with_equals(arg, cmd)) { \
          int cmd_len = str_len(cmd); \
          char *arg_start = &arg[cmd_len + 1]; \
          if (str_to_##type(arg_start, &parsed)!=0) { \
            s_generic.name = parsed; \
            opt_found++; \
            i++; \
            continue; \
          } \
        } \
        if (is_opt_with_equals(arg, cmd_verbose)) { \
          int cmd_len = str_len(cmd_verbose); \
          char *arg_start = &arg[cmd_len + 1]; \
          if (str_to_##type(arg_start, &parsed)!=0) { \
            s_generic.name = parsed; \
            opt_found++; \
            i++; \
            continue; \
          } \
        } \
      }
#define FLG(cmd, cmd_verbose, type, name, ...) \
      if (flg_found < flg_count) { \
        type parsed; \
        if (is_opt(arg) && str_starts(arg, cmd)) { \
          int cmd_len = str_len(cmd); \
          char *arg_start = &arg[cmd_len]; \
          if (str_to_##type(arg_start, &parsed)!=0) { \
            s_generic.name = parsed; \
            flg_found++; \
            i++; \
            continue; \
          } \
        } \
        if (is_opt(arg) && str_starts(arg, cmd_verbose)) { \
          int cmd_len = str_len(cmd_verbose); \
          char *arg_start = &arg[cmd_len]; \
          if (str_to_##type(arg_start, &parsed)!=0) { \
            s_generic.name = parsed; \
            flg_found++; \
            i++; \
            continue; \
          } \
        } \
      }
#define TXT(cmd, cmd_verbose, text) \
      if (txt_found < txt_count) { \
        if (str_equals(arg, cmd) || str_equals(arg, cmd_verbose)) { \
          printf("%s\n", text); \
          return 1; \
        } \
      }
#define HLP(cmd, cmd_verbose, txt, ...) \
      if (str_equals(arg, cmd) || str_equals(arg, cmd_verbose)) { \
        printf(txt); \
        return 1; \
      } 
#define END_FN(name, ...) \
      printf("Unknown Option: %s\n", argv[i]); \
      i++; \
    } \
    \
    if (req_found != req_count) { \
      printf("Missing required parameters. Usage: %s ", argv[0]);\
      for (int j = 0; j < req_count; j++) { \
        printf("%s ", req_arr_##name[j]); \
      } \
      if (opt_count > 0) { \
        printf("["); \
        for (int j = 0; j < opt_count; j++) { \
          printf("%s", (char*)opt_arr_##name[j][0]);\
          if (strcmp(opt_arr_##name[j][1], "") != 0) printf(" | %s", opt_arr_##name[j][1]); \
          if (j != opt_count - 1) printf(", "); \
        } \
        printf("]"); \
      } \
      printf("\n"); \
      return 0; \
    } else { \
      generic_cb(&s_generic); \
      return 1; \
    } \
    if (strcmp(#name, "") == 0) fn_depth++; \
  } while(0); \
  fn_depth--;

  do {
    int fn_depth = 0;
    struct args s_generic2 = default_args;
    ARGS
  } while(0);

  /* main logic end */

#endif

  return 1;
}

static int cargs_show_help(int argc, char** argv) {
  return 1;
}


#ifndef C_ENCODING
#define C_ENCODING

#include "cstring.h"
#include <stdio.h>
#include <string.h>

#ifdef ENCODING_STANDALONE
#define DECLARE_FUNCTION(name)                                                 \
  void name##_r(char **);                                                      \
  char *name(char *);
#define DEFINE_FUNCTION(name, body)                                            \
  void name##_r(char **c) { body }                                             \
  char *name(char *c) {                                                        \
    if (!c)                                                                    \
      return NULL;                                                             \
    char *ptr = malloc(strlen(c));                                             \
                                                                               \
    name##_r(&ptr);                                                            \
                                                                               \
    return ptr;                                                                \
  }
#else
#define DECLARE_FUNCTION(name)                                                 \
  void name##_r(char **);                                                      \
  char *name(char *);                                                          \
  string_t str_##name(string_t s);                                             \
  string_t *strr_##name(string_t *s);
#define DEFINE_FUNCTION(name, body)                                            \
  void name##_r(char **c) body char *name(char *c) {                           \
    if (!c)                                                                    \
      return NULL;                                                             \
    char *ptr = malloc(strlen(c));                                             \
                                                                               \
    name##_r(&ptr);                                                            \
                                                                               \
    return ptr;                                                                \
  }                                                                            \
                                                                               \
  string_t str_##name(string_t s) {                                            \
    string_t s_new = str_null;                                                 \
    str_clone(&s_new, s);                                                      \
                                                                               \
    strr_##name(&s_new);                                                       \
                                                                               \
    return s_new;                                                              \
  }                                                                            \
                                                                               \
  string_t *strr_##name(string_t *s) {                                         \
                                                                               \
    if (!s)                                                                    \
      return NULL;                                                             \
    str_heapify(s);                                                            \
                                                                               \
    name##_r(&s->str);                                                         \
                                                                               \
    return s;                                                                  \
  }

#endif

#define UTF8_CODEPOINT_1 0x7F
#define UTF8_CODEPOINT_2 0x7FF
#define UTF8_CODEPOINT_3 0xFFFF
#define UTF8_CODEPOINT_4 0x10000
#define UTF8_MAX 0x10FFFF

DECLARE_FUNCTION(utf8_encode);
DECLARE_FUNCTION(utf8_decode);

static void *unicode_to_utf8(unsigned int codepoint, size_t *len);
static unsigned int utf8_to_unicode(char *c, size_t *advance, char *valid);

int minbits(unsigned int n);

static void mem_insert(char **c, size_t offset, void *data, size_t data_len);
static void mem_replace(char **c, size_t offset, size_t len, void *data,
                        size_t data_len);
static void mem_append(char **c, void *data, size_t data_len);

#endif

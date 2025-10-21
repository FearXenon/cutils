#ifndef C_STRING
#define C_STRING

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define str(c) str_init_from_chr(c)
#define strh(c) str_acquire(c)

#define str_null ((string_t){0, 0, 0})

#define STR_HEAP 0x01
#define STR_OWNER 0x02

#define str_auto string_t __attribute__((cleanup(str_free)))

typedef struct string {
  char *str;
  size_t len;
  char flags;
} string_t;
typedef string_t str;

typedef struct tokenizer {
  string_t delimiter;
  string_t base;
  char *start, *end;
} tokenizer_t;

void str_init(string_t *s);
string_t str_init_from_chr(char *c);

string_t str_transfer(string_t *source);
string_t str_acquire(const char *c);
string_t str_acquire_s(const char *c, size_t len);

string_t str_borrow(string_t *dest, const char *src, const size_t len);
void str_heapify(string_t *s);

void str_free(string_t *s);

int str_print(string_t s);
int str_println(string_t s);

int str_chr_is_ws(const char *c);

size_t str_len(const string_t s);

int str_equals(const string_t s1, const string_t s2);
int str_equals_ic(const string_t s1, const string_t s2);

size_t str_pos(string_t s, const string_t search);
size_t str_ipos(string_t s, const string_t search);

tokenizer_t str_token_init(string_t s, const string_t delimiter);
char str_token_next(tokenizer_t *tok, string_t *s);

void str_explode(string_t s, const string_t delimiter, string_t **arr,
                 size_t *len);
string_t str_implode(const string_t delimiter, string_t *arr, size_t len);

string_t str_tolower(string_t s);
string_t str_toupper(string_t s);

string_t str_cat(const string_t s, char *str, ...);
string_t str_prepend(const string_t s, char *str, ...);

string_t str_replace(string_t s, const string_t search, const string_t replace);

string_t str_substr(string_t s, ssize_t offset, ssize_t len);

string_t str_trim(const string_t s);
string_t str_rtrim(const string_t s);
string_t str_ltrim(const string_t s);

string_t str_rev(const string_t s);

string_t str_lpad(string_t s, char c, size_t len);
string_t str_rpad(string_t s, char c, size_t len);

/*
 * strr_XY => string_t reference functions -> modifies the string passed
 * */

string_t *strr_implode(string_t *s, const string_t delimiter, string_t *arr,
                       size_t len);

string_t *strr_tolower(string_t *s);
string_t *strr_toupper(string_t *s);

string_t *strr_cat(string_t *s, char *str, ...);
static void strr_catv(string_t *s, char *str, va_list args);

string_t *strr_prepend(string_t *s, char *str, ...);
static void strr_prependv(string_t *s, char *str, va_list args);

string_t *strr_replace(string_t *s, const string_t search,
                       const string_t replace);

string_t *strr_substr(string_t *s, ssize_t offset, ssize_t len);

string_t *strr_trim(string_t *s);
string_t *strr_rtrim(string_t *s);
string_t *strr_ltrim(string_t *s);

string_t *strr_rev(string_t *s);

string_t *strr_lpad(string_t *s, char c, size_t len);
string_t *strr_rpad(string_t *s, char c, size_t len);

static inline char *str_ptr(const string_t s) {
  extern const char *const str_empty_chr;

  return s.str ? s.str : (char *)str_empty_chr;
}
static void str_calc_len(string_t *s);
void str_clone(string_t *target, string_t s);
void str_clone_from_chr(string_t *target, char *c, size_t len);
static string_t str_from_format(char *str, va_list args);
void str_mem_insert(string_t *string, size_t offset, void *data,
                    size_t data_len);
void str_mem_replace(string_t *string, size_t offset, size_t len, void *data,
                     size_t data_len);
void str_mem_append(string_t *string, void *data, size_t data_len);

#endif

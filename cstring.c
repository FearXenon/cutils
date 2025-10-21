#include "cstring.h"

const char *const str_empty_chr = "";

/**
 * initilize a string from a stack allocated char*
 * */
string_t str_init_from_chr(char *c) {
  string_t s = {.str = c, .len = 0, .flags = 0};

  str_init(&s);

  return s;
}

/**
 * initilize a string from a stack allocated char*
 * */
void str_init(string_t *s) {

  if (!s) {
    *s = str_null;
    return;
  }

  size_t len = 0;
  while (*(s->str + len))
    len++;

  if (len == 0) {
    *s = str_null;
    return;
  }

  s->len = len;
}

/**
 * transfer the char* ownership to another string_t
 * */
string_t str_transfer(string_t *source) {

  string_t dest;

  if (!source)
    return str_null;

  source->flags &= ~STR_OWNER;
  dest.str = source->str;
  dest.flags |= STR_OWNER;

  return dest;
}

/**
 * create a string from a heap allocated char*
 **/
string_t str_acquire(const char *c) {

  if (!c)
    return str_null;

  string_t s;

  s.str = (char *)c;
  s.flags = STR_HEAP | STR_OWNER;

  str_calc_len(&s);

  return s;
}

/**
 * create a string from a heap allocated char* and a given length
 **/
string_t str_acquire_s(const char *c, size_t len) {

  if (!c)
    return str_null;

  string_t s;

  s.str = (char *)c;
  s.len = len;
  s.flags = STR_HEAP | STR_OWNER;

  return s;
}

/**
 * create a string_t from a part of a char*
 * */
string_t str_borrow(string_t *dest, const char *src, const size_t len) {

  if (!dest || !src)
    return str_null;

  dest->str = (char *)src;
  dest->len = len;
  dest->flags = 0;

  return *dest;
}

/**
 * creates a heap allocated version of the given string
 * */
void str_heapify(string_t *s) {

  if (!s)
    return;

  if (!(s->flags & STR_HEAP)) {
    char *tmp = malloc(s->len + 1);

    if (tmp)
      memcpy(tmp, str_ptr(*s), s->len);

    *(tmp + s->len) = 0;
    s->str = tmp;

    // the char* is heap allocated and the current string_t is the owner of it
    s->flags |= STR_HEAP | STR_OWNER;
  }
}

/**
 * frees the char* if the string is its owner
 * */
void str_free(string_t *s) {

  if (!s)
    return;

  if (s->flags & STR_OWNER && s->flags & STR_HEAP) {
    free(s->str);
    s->str = (void *)0;

    s->len = 0;
  }
}

int str_print(string_t s) { return printf("%.*s", (int)s.len, s.str); }

int str_println(string_t s) { return printf("%.*s\n", (int)s.len, s.str); }

/**
 * check if a char is ' ', '\t' or '\r'
 * */
int str_chr_is_ws(const char *c) {
  if (!c)
    return 0;

  if (*c == ' ' || *c == '\t' || *c == '\r')
    return 1;

  return 0;
}

size_t str_len(const string_t s) { return s.len; }

/**
 * append to the given string (see str_from_format)
 * */
string_t str_cat(const string_t string, char *str, ...) {
  string_t new_s;
  str_clone(&new_s, string);

  va_list args;
  va_start(args, str);

  // Forward to strr_cat
  strr_catv(&new_s, str, args);

  va_end(args);
  return new_s;
}

/**
 * append to the given string (see str_from_format)
 * */
string_t *strr_cat(string_t *s, char *str, ...) {

  if (!s)
    return s;

  va_list args;
  va_start(args, str);

  strr_catv(s, str, args);

  va_end(args);

  return s;
}

static void strr_catv(string_t *s, char *str, va_list args) {

  if (!s)
    return;

  str_auto formatted = str_from_format(str, args);

  if (!*str_ptr(*s)) {
    // free the base string if str_null or empty
    str_free(s);

    // If the destination string is empty, just clone the formatted string
    str_clone(s, formatted);
  } else {
    // Concatenate formatted string to the end
    str_mem_append(s, str_ptr(formatted), formatted.len);
  }
}

/**
 * prepend to the given string (see str_from_format)
 * */
string_t str_prepend(const string_t s, char *str, ...) {
  string_t new_s;
  str_clone(&new_s, s);

  va_list args;
  va_start(args, str);

  // Forward to strr_cat
  strr_prependv(&new_s, str, args);

  va_end(args);
  return new_s;
}

/**
 * prepend to the given string (see str_from_format)
 * */
string_t *strr_prepend(string_t *s, char *str, ...) {

  if (!s)
    return s;

  va_list args;
  va_start(args, str);

  strr_prependv(s, str, args);

  va_end(args);

  return s;
}

static void strr_prependv(string_t *s, char *str, va_list args) {

  if (!s)
    return;

  str_heapify(s);

  str_auto formatted = str_from_format(str, args);

  if (!*str_ptr(*s)) {
    // free the base string if str_null or empty
    str_free(s);
    // If the destination string is empty, just clone the formatted string
    str_clone(s, formatted);
  } else {
    // concatenate formatted string to the end
    str_mem_append(&formatted, str_ptr(*s), s->len);

    // free the old one
    str_free(s);

    // clone the new one
    str_clone(s, formatted);
  }
}

/**
 * check if two strings are equal (case sensitive)
 * */
int str_equals(const string_t s1, const string_t s2) {
  if (s1.len != s2.len) {
    return 0;
  }

  return !strcmp(str_ptr(s1), str_ptr(s2));
}

/**
 * check if two strings are equal (case insensitive)
 * */
int str_equals_ic(const string_t s1, const string_t s2) {
  if (s1.len != s2.len)
    return 0;

  char *s1_chr = (char *)str_ptr(s1);
  char *s2_chr = (char *)str_ptr(s2);
  for (; *s1_chr; s1_chr++, s2_chr++) {
    if (!(tolower(*s1_chr) == tolower(*s2_chr)))
      return 0;
  }

  return 1;
}

string_t str_tolower(string_t s) {
  string_t new;
  str_clone(&new, s);

  strr_tolower(&new);

  return new;
}

string_t *strr_tolower(string_t *s) {

  if (!s)
    return s;

  str_heapify(s);

  for (int i = 0; i < s->len; i++)
    s->str[i] = tolower(s->str[i]);

  return s;
}

string_t str_toupper(string_t s) {
  string_t new;

  str_clone(&new, s);

  strr_toupper(&new);

  return new;
}

string_t *strr_toupper(string_t *s) {

  if (!s)
    return s;

  str_heapify(s);

  for (int i = 0; i < s->len; i++)
    s->str[i] = toupper(s->str[i]);

  return s;
}

void str_calc_len(string_t *s) {

  if (!s || !s->str)
    return;

  size_t len = 0;
  char *chr = s->str;
  for (; *chr; chr++)
    len++;

  s->len = len;
}

/**
 * returns the position of the first occurence of search in s (case sensitive)
 * returns -1 if search is not found
 * */
size_t str_pos(string_t s, const string_t search) {
  size_t pos = -1;

  if (!*str_ptr(search))
    return 0;

  if (!*str_ptr(s))
    return -1;

  if (s.len < search.len)
    return -1;

  if (str_equals(s, search))
    return 0;

  char *c = s.str;
  char *cl = c;
  char *c_end = c + s.len - 1;
  char *search_c = search.str;
  while (*cl && cl <= c_end) {

    while (*c && *c == *search_c && c <= c_end) {
      c++, search_c++;
    }

    // found the string
    if (!*search_c) {
      return cl - s.str;
    } else {
      // set the str to the next base str
      c = cl;
      // reset search pointer
      search_c = search.str;
    }
    cl++, c++;
  }

  return pos;
}

/**
 * returns the position of the first occurence of search in s (case insensitive)
 * returns -1 if search is not found
 * */
size_t str_ipos(string_t s, const string_t search) {
  size_t pos = -1;

  if (!*str_ptr(search))
    return 0;

  if (!*str_ptr(s))
    return -1;

  if (s.len < search.len)
    return -1;

  if (str_equals(s, search))
    return 0;

  char *c = s.str;
  char *cl = c;
  char *c_end = c + s.len - 1;
  char *search_c = search.str;
  while (*cl && cl <= c_end) {

    while (*c && tolower(*c) == tolower(*search_c) && c <= c_end) {
      c++, search_c++;
    }

    // found the string
    if (!*search_c) {
      return cl - s.str;
    } else {
      // set the str to the next base str
      c = cl;
      // reset search pointer
      search_c = search.str;
    }
    cl++, c++;
  }

  return pos;
}

/**
 * splits the given string into an array of strings
 * */
void str_explode(string_t s, const string_t delimiter, string_t **arr,
                 size_t *len) {

  *arr = malloc(sizeof(string_t));
  *len = 0;

  if (!*str_ptr(s) || !*str_ptr(delimiter)) {
    *len = 1;
    str_clone(*arr, s);
    return;
  }

  char *s_base = s.str;
  size_t len_base = s.len;
  size_t pos;
  while ((pos = str_pos(s, delimiter)) != -1) {
    // delimiter is not at the very front
    if (pos != 0) {
      str_auto entry;
      str_clone_from_chr(&entry, s.str, pos);
      str_clone(*arr + *len, entry);
    } else {
      str_clone(*arr + *len, str_null);
    }
    // advance the str pointer
    s.str += pos + delimiter.len;
    s.len -= pos + delimiter.len;

    (*len)++;

    *arr = realloc(*arr, sizeof(string_t) * (*len + 1));
    if (!arr)
      return;
  }

  str_clone(*arr + *len, s);
  (*len)++;

  s.str = s_base;
}

/**
 * joins an array of strings into a string
 * */
string_t str_implode(const string_t delimiter, string_t *arr, size_t len) {
  string_t s = str_null;

  strr_implode(&s, delimiter, arr, len);

  return s;
}

/**
 * joins an array of strings into a string
 * */
string_t *strr_implode(string_t *s, const string_t delimiter, string_t *arr,
                       size_t len) {

  if (!s)
    return s;

  if (len == 0)
    return s;

  for (int i = 0; i < len; i++) {
    string_t s_arr = *(arr + i);

    strr_cat(s, "%S", s_arr);

    if (i != len - 1)
      strr_cat(s, "%S", delimiter);
  }

  return s;
}

/**
 * replaces search with replace in the string
 * */
string_t str_replace(string_t s, const string_t search,
                     const string_t replace) {
  string_t s_new = str_null;

  if (!*str_ptr(search))
    return s;

  size_t pos;
  while ((pos = str_pos(s, search)) != -1) {

    if (pos != 0) {
      // add everything up to the "search"
      str_mem_append(&s_new, s.str, pos);
    }

    // add the replace
    str_mem_append(&s_new, replace.str, replace.len);

    s.str += pos + search.len;
    s.len -= pos + search.len;
  }

  // add the rest
  str_mem_append(&s_new, s.str, s.len);

  return s_new;
}

/**
 * replaces search with replace in the string
 * */
string_t *strr_replace(string_t *s, const string_t search,
                       const string_t replace) {

  if (!s)
    return s;

  if (!*str_ptr(search))
    return s;

  str_auto s_cpy;
  str_clone(&s_cpy, *s);
  char *s_cpy_chr = s_cpy.str;

  str_free(s);
  *s = str_null;

  size_t pos;
  while ((pos = str_pos(s_cpy, search)) != -1) {

    if (pos != 0) {
      // add everything up to the "search"
      str_mem_append(s, s_cpy.str, pos);
    }

    // add the replace
    str_mem_append(s, replace.str, replace.len);

    s_cpy.str += pos + search.len;
    s_cpy.len -= pos + search.len;
  }

  // add the rest
  str_mem_append(s, s_cpy.str, s_cpy.len);

  s_cpy.str = s_cpy_chr;

  return s;
}

/**
 * get the substring of a string using offset and length
 * */
string_t str_substr(string_t s, ssize_t offset, ssize_t len) {
  string_t s_new = str_null;

  str_clone(&s_new, s);

  strr_substr(&s_new, offset, len);

  return s_new;
}

/**
 * get the substring of a string using offset and length
 * */
string_t *strr_substr(string_t *s, ssize_t offset, ssize_t len) {

  if (!s)
    return s;

  if (offset >= (ssize_t)s->len || (offset < 0 && len < offset)) {
    str_free(s);
    *s = str_null;
    return s;
  }

  if (len > (ssize_t)s->len)
    len = (ssize_t)s->len;

  if (offset < 0) {
    offset = (ssize_t)s->len + offset;
    if (offset < 0)
      offset = 0;
  }
  if (len < 0)
    len = (ssize_t)s->len + len - offset;

  char *tmp = malloc(len + 1);

  if (!tmp)
    return s;

  memcpy(tmp, s->str + offset, len);

  *(tmp + len) = 0;

  free(s->str);

  s->str = tmp;
  s->len = len;

  return s;
}

/**
 * remove whitespaces at the end and the front
 * */
string_t str_trim(const string_t s) {
  string_t s_new = str_null;

  str_clone(&s_new, s);

  strr_trim(&s_new);

  return s_new;
}

/**
 * remove whitespaces at the end and the front
 * */
string_t *strr_trim(string_t *s) {

  if (!s)
    return s;

  size_t len = s->len;
  char *c = s->str;
  char *c_end = s->str + len - 1;

  str_heapify(s);

  while (str_chr_is_ws(c) && len > 0) {
    c++, len--;
  }

  while (str_chr_is_ws(c_end) && len > 0) {
    c_end--, len--;
  }

  char *tmp = malloc(len + 1);

  if (!tmp)
    return s;

  memcpy(tmp, c, len);

  *(tmp + len) = 0;

  free(s->str);

  s->str = tmp;
  s->len = len;

  return s;
}

/**
 * remove whitespaces at the end
 * */
string_t str_rtrim(const string_t s) {
  string_t s_new = str_null;

  str_clone(&s_new, s);

  strr_rtrim(&s_new);

  return s_new;
}

/**
 * remove whitespaces at the end
 * */
string_t *strr_rtrim(string_t *s) {

  if (!s)
    return s;

  size_t len = s->len;
  char *c = s->str;
  char *c_end = s->str + len - 1;

  if (!(*c))
    return s;

  str_heapify(s);

  while (str_chr_is_ws(c_end) && len > 0) {
    c_end--, len--;
  }

  char *tmp = malloc(len + 1);

  if (!tmp)
    return s;

  memcpy(tmp, s->str, len);

  *(tmp + len) = 0;

  free(s->str);

  s->str = tmp;
  s->len = len;

  return s;
}

/**
 * remove whitespaces at the front
 * */
string_t str_ltrim(const string_t s) {
  string_t s_new = str_null;

  str_clone(&s_new, s);

  strr_ltrim(&s_new);

  return s_new;
}

/**
 * remove whitespaces at the front
 * */
string_t *strr_ltrim(string_t *s) {

  if (!s)
    return s;

  str_heapify(s);

  char *c = s->str;

  size_t start = 0;
  size_t len = s->len;
  while (str_chr_is_ws(c) && len > 0) {
    c++, len--;
  }

  char *tmp = malloc(len + 1);

  if (!tmp)
    return s;

  memcpy(tmp, c, len);

  *(tmp + len) = 0;

  free(s->str);

  s->str = tmp;
  s->len = len;

  return s;
}

/**
 * reverse the string
 * */
string_t str_rev(const string_t s) {
  string_t s_new = str_null;

  char *c = str_ptr(s);

  if (!*c)
    return s_new;

  str_clone(&s_new, s);

  strr_rev(&s_new);

  return s_new;
}

/**
 * reverse the string
 * */
string_t *strr_rev(string_t *s) {

  if (!s || !s->str)
    return s;

  str_heapify(s);

  char *tmp = malloc(s->len);

  if (!tmp)
    return s;

  memcpy(tmp, s->str, s->len);

  int j = 0;
  for (int i = s->len - 1; i >= 0; i--, j++)
    s->str[i] = tmp[j];

  free(tmp);

  return s;
}

//  TODO: pad with UTF8-chars
/**
 * pad the string to len by prepending c
 * */
string_t str_lpad(string_t s, char c, size_t len) {
  string_t s_new;

  str_clone(&s_new, s);

  strr_lpad(&s_new, c, len);

  return s_new;
}

/**
 * pad the string to len by prepending c
 * */
string_t *strr_lpad(string_t *s, char c, size_t len) {

  if (!s)
    return s;

  str_heapify(s);

  if (s->len < len) {
    size_t padding = len - s->len;
    char *tmp = malloc(padding);

    if (!tmp)
      return s;

    memset(tmp, c, padding);

    string_t s_tmp = str_acquire_s(tmp, padding);

    strr_cat(&s_tmp, "%S", *s);

    str_free(s);

    *s = s_tmp;
  }

  return s;
}

// TODO: pad with UTF8-chars
/**
 * pad the string to len by appending c
 * */
string_t str_rpad(string_t s, char c, size_t len) {
  string_t s_new;

  str_clone(&s_new, s);

  strr_rpad(&s_new, c, len);

  return s_new;
}

/**
 * pad the string to len by appending c
 * */
string_t *strr_rpad(string_t *s, char c, size_t len) {

  if (!s)
    return s;

  str_heapify(s);

  if (s->len < len) {
    size_t padding = len - s->len;
    char *tmp = malloc(padding);

    if (!tmp)
      return s;

    memset(tmp, c, padding);

    str_auto s_tmp = str_acquire_s(tmp, padding);

    strr_cat(s, "%S", s_tmp);
  }

  return s;
}

/**
 * initialize the tokenizer
 * */
tokenizer_t str_token_init(string_t s, const string_t delimiter) {
  tokenizer_t tok = {.delimiter = delimiter,
                     .base = s,
                     .start = s.str,
                     .end = ((char *)s.str + s.len - 1)};

  return tok;
}

/**
 * get the next token from the tokenizer
 * */
char str_token_next(tokenizer_t *tok, string_t *s) {

  if (!tok)
    return 0;

  if (!s)
    return 0;

  str_free(s);

  size_t pos;
  pos = str_pos(tok->base, tok->delimiter);

  // return whole string
  if (pos == -1)
    *s = str_acquire_s(tok->base.str, tok->base.len);

  while (tok->base.str <= tok->end) {
    if (pos > 0) {
      str_clone_from_chr(s, tok->base.str, pos);
      tok->base.str += pos + 1;
      return 1;
    } else {
      tok->base.str += tok->delimiter.len;
      pos = str_pos(tok->base, tok->delimiter);
    }
  }

  return 0;
}

/**
 * clones the given string into the pointer
 * clones the char* aswell
 * */
void str_clone(string_t *target, string_t s) {

  if (!target)
    return;

  char *new_str = malloc(s.len + 1);

  if (!new_str)
    return;

  memcpy(new_str, str_ptr(s), s.len);

  target->str = new_str;
  target->len = s.len;
  target->flags = STR_OWNER | STR_HEAP;

  *(target->str + s.len) = 0;
}

/**
 * clones the given char* and creates a new string with it
 * */
void str_clone_from_chr(string_t *target, char *c, size_t len) {

  if (!target)
    return;

  char *new_str = malloc(len + 1);

  if (!new_str)
    return;

  memcpy(new_str, c, len);

  target->str = new_str;
  target->len = len;
  target->flags = STR_OWNER | STR_HEAP;

  *(target->str + len) = 0;
}

/**
 * creates a string from the given format
 *
 * supported formats:
 *
 * %d: int
 * %l: long
 * %s: char* string
 * %S: string_t string
 *
 * */
static string_t str_from_format(char *str, va_list args) {

  string_t s = {.str = (void *)0, .len = 0, .flags = STR_OWNER | STR_HEAP};

  size_t len = 0;

  int arg_int;
  long arg_long;
  string_t arg_str;
  char *arg_c_str;
  char arg_c;

  while (*str) {
    if (*str == '%') {
      str++;

      int tmp_len = 0;
      int factor;
      switch (*str) {
      case 'd':
        arg_int = va_arg(args, int);
        tmp_len = 1;
        factor = 10;
        while (factor < arg_int) {
          factor *= 10;
          tmp_len++;
        }

        len += tmp_len;

        s.str = realloc(s.str, len);
        factor /= 10;
        for (int i = 0; i < tmp_len; i++) {
          s.str[s.len + i] = '0' + (arg_int / factor);
          arg_int -= (arg_int / factor) * factor;
          factor /= 10;
        }
        s.len += tmp_len;
        break;
      case 'l':
        arg_long = va_arg(args, long);
        tmp_len = 1;
        factor = 10;
        while (factor < arg_long) {
          factor *= 10;
          tmp_len++;
        }
        len += tmp_len;

        s.str = realloc(s.str, len);
        factor /= 10;
        for (int i = 0; i < tmp_len; i++) {
          s.str[s.len + i] = '0' + (arg_long / factor);
          arg_long -= (arg_long / factor) * factor;
          factor /= 10;
        }
        s.len += tmp_len;
        break;
      case 'S':
        arg_str = va_arg(args, string_t);
        len += arg_str.len;

        str_mem_append(&s, arg_str.str, arg_str.len);
        break;
      case 's':
        arg_c_str = va_arg(args, char *);
        while (*arg_c_str) {
          tmp_len++, arg_c_str++;
        }
        len += tmp_len;

        str_mem_append(&s, arg_c_str - tmp_len, tmp_len);
        break;
      default:
        break;
      }
      str++;
      continue;

    } else {
      char *str_start = str;
      while (*str && *str != '%') {
        str++, len++;
        if (!*str || *str == '%') {
          str_mem_append(&s, str_start, str - str_start);
        }
      }
    }
  }

  // add null byte
  s.str = realloc(s.str, len + 1);

  if (!s.str)
    return str_null;

  *(s.str + s.len) = 0;

  return s;
}

/**
 * string: pointer to the string
 * offset: offset to insert-position
 * data: pointer to the insert data
 * data_len: length of the insert data
 * */
void str_mem_insert(string_t *string, size_t offset, void *data,
                    size_t data_len) {

  if (!string)
    return;

  if (!data)
    return;

  if (offset > string->len)
    offset = string->len;

  str_heapify(string);

  string->str = realloc(string->str, string->len + data_len + 1);

  if (!string->str)
    return;

  memmove(string->str + offset + data_len, string->str + offset,
          string->len - offset);

  memcpy(string->str + offset, data, data_len);

  string->len += data_len;

  *(string->str + string->len) = 0;
}

/**
 * string: pointer to the string
 * offset: offset to the replaced-start
 * len: length of the replaced string
 * data: pointer to the replacement data
 * data_len: length of the replacement data
 * */
void str_mem_replace(string_t *string, size_t offset, size_t len, void *data,
                     size_t data_len) {

  if (!string)
    return;

  if (!data)
    return;

  if (offset >= string->len)
    offset = string->len - 1;

  if (offset + len > string->len)
    len = string->len - offset;

  str_heapify(string);

  string->str = realloc(string->str, string->len + data_len - len + 1);

  if (!string->str)
    return;

  memmove(string->str + offset + data_len, string->str + offset + len,
          string->len - offset - len);

  memcpy(string->str + offset, data, data_len);

  string->len += data_len - len;

  *(string->str + string->len) = 0;
}

/**
 * string: pointer to the string
 * len: new string length
 * data: data to be appended
 * data_len: size of the data to be appended
 * */
void str_mem_append(string_t *string, void *data, size_t data_len) {

  if (!string)
    return;

  if (!data)
    return;

  str_heapify(string);

  string->str = realloc(string->str, string->len + data_len + 1);

  if (!string->str)
    return;

  memcpy(string->str + string->len, data, data_len);
  string->len += data_len;

  // add null terminator
  *(string->str + string->len) = 0;
}

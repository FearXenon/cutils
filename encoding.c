#include "encoding.h"

DEFINE_FUNCTION(utf8_encode, {
  if (!c || !*c)
    return;

  char *base = *c;
  size_t pos = 0;

  while (base[pos]) {
    size_t len = 1;
    void *utf8_data = unicode_to_utf8((unsigned char)base[pos], &len);

    mem_replace(&base, pos, 1, utf8_data, len);

    pos += len;

    free(utf8_data);
  }

  *c = base;
});

DEFINE_FUNCTION(utf8_decode, {
  if (!c || !*c)
    return;

  char *base = *c;
  size_t pos = 0;

  while (base[pos]) {
    size_t advance = 1;
    char valid = 0;

    unsigned int codepoint = utf8_to_unicode(&base[pos], &advance, &valid);

    if (codepoint > 0xFF) {
      // put question mark
      codepoint = 0x3F;
    }

    if (!valid || advance == 0) {
      pos++;
      // put question mark
      codepoint = 0x3F;
    }

    mem_replace(&base, pos, advance, &codepoint, 1);

    pos++;
  }

  *c = base;
});

static void utf8_set_bytes(unsigned char *ptr, int utf8_len,
                           unsigned int codepoint) {

  memset(ptr, 0x80, utf8_len);

  if (utf8_len == 1) {
    *ptr = codepoint;
  } else if (utf8_len == 2) {
    *ptr = 0xC0 | ((codepoint >> 6) & 0x1F);
    *(ptr + 1) = 0x80 | (codepoint & 0x3F);
  } else if (utf8_len == 3) {
    *ptr = 0xE0 | ((codepoint >> 12) & 0x0F);
    *(ptr + 1) = 0x80 | ((codepoint >> 6) & 0x3F);
    *(ptr + 2) = 0x80 | (codepoint & 0x3F);
  } else if (utf8_len == 4) {
    *ptr = 0xF0 | ((codepoint >> 18) & 0x07);
    *(ptr + 1) = 0x80 | ((codepoint >> 12) & 0x3F);
    *(ptr + 2) = 0x80 | ((codepoint >> 6) & 0x3F);
    *(ptr + 3) = 0x80 | (codepoint & 0x3F);
  }

  *(ptr) &= ~(0x80 >> utf8_len);
}

void *unicode_to_utf8(unsigned int codepoint, size_t *len) {

  unsigned char *ptr = NULL;

  int i = 1;
  if (codepoint <= UTF8_CODEPOINT_1) {
    i = 1;
    *len = i;
    ptr = malloc(i);

    if (!ptr)
      return NULL;

    *ptr = codepoint;

    return ptr;
  } else if (codepoint <= UTF8_CODEPOINT_2) {
    i = 2;
  } else if (codepoint <= UTF8_CODEPOINT_3) {
    i = 3;
  } else if (codepoint <= UTF8_MAX) {
    i = 4;
  } else {
    *len = 1;
    ptr = malloc(1);

    if (!ptr)
      return NULL;

    *ptr = 0;
    return ptr;
  }

  ptr = malloc(i);

  if (!ptr)
    return NULL;

  utf8_set_bytes(ptr, i, codepoint);

  *len = i;

  return ptr;
}

unsigned int utf8_to_unicode(char *c, size_t *advance, char *valid) {

  *valid = 1;
  *advance = 1;

  if (!c) {
    *valid = 0;
    *advance = 1;
    return 0;
  }

  // first bit is 0 -> 1 byte char
  if ((*c & 0x10000000) == 0x00000000) {
    *advance = 1;
    return *c;
  }

  char utf8_len = 0;
  for (int i = 0; i < 4; i++) {
    if (!(*c & (0x80 >> i))) {
      break;
    }
    utf8_len++;
  }

  if (utf8_len == 3 && (*c & (0x80 >> 3))) {
    *valid = 0;
    *advance = 1;
    return 0;
  }

  unsigned int codepoint = 0;

  for (int i = 1; i < utf8_len; i++) {
    if (((unsigned char)c[i] & 0xC0) != 0x80) {
      *valid = 0;
      return 0;
    }
  }

  unsigned char *bytes = (unsigned char *)c;

  if (utf8_len == 2) {
    codepoint = ((bytes[0] & 0x1F) << 6 | (bytes[1] & 0x3F));
  } else if (utf8_len == 3) {
    codepoint =
        ((bytes[0] & 0x1F) << 12 | (bytes[1] & 0x3F) << 6 | (bytes[2] & 0x3F));
  } else if (utf8_len == 4) {
    codepoint = ((bytes[0] & 0x1F) << 18 | (bytes[1] & 0x3F) << 12 |
                 (bytes[2] & 0x3F) << 6 | (bytes[3] & 0x3F));
  }
  *advance = utf8_len;

  return codepoint;
}

int minbits(unsigned int n) {
  if (n == 0)
    return 1;

  int bits = 0;
  while (n) {
    bits++;
    n >>= 1;
  }
  return bits;
}

static void mem_insert(char **c, size_t offset, void *data, size_t data_len) {

  if (!c)
    return;

  if (!data)
    return;

  size_t str_len = strlen(*c);

  *c = realloc(*c, str_len + data_len + 1);

  if (!*c)
    return;

  memmove(*c + offset + data_len, c + offset, str_len - offset);

  memcpy(*c + offset, data, data_len);

  (*c)[str_len + data_len] = 0;
}

static void mem_replace(char **c, size_t offset, size_t len, void *data,
                        size_t data_len) {
  if (!c || !*c || !data)
    return;

  size_t str_len = strlen(*c);
  if (offset + len > str_len)
    return;

  size_t tail_len = str_len - offset - len;
  size_t new_len = str_len - len + data_len;

  char *tail = NULL;
  if (tail_len > 0) {
    tail = malloc(tail_len);
    if (!tail)
      return;
    memcpy(tail, *c + offset + len, tail_len);
  }

  char *new_c = realloc(*c, new_len + 1);
  if (!new_c) {
    free(tail);
    return;
  }
  *c = new_c;

  memcpy(*c + offset, data, data_len);

  if (tail_len > 0) {
    memmove(*c + offset + data_len, tail, tail_len);
    free(tail);
  }

  (*c)[new_len] = 0;
}

/**
 * string: pointer to the string
 * len: new string length
 * data: data to be appended
 * data_len: size of the data to be appended
 * */
static void mem_append(char **c, void *data, size_t data_len) {

  if (!c)
    return;

  if (!data)
    return;

  size_t str_len = strlen(*c);

  *c = realloc(*c, str_len + data_len + 1);

  if (!*c)
    return;

  memcpy(*c + str_len, data, data_len);

  (*c)[str_len + data_len] = 0;
}

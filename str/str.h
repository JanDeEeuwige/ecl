// Description: API for dynamic UTF-8 strings in C
// Author:      Jan De Eeuwige <jan.de.eeuwige@proton.me>
// Copyright:   (C) 2026 Jan De Eeuwige
// License:     MIT

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Way to represent a UTF-8 character
typedef uint32_t str__char_t;

// Representation of a UTF-8 string
typedef struct {
    size_t capacity;        // Maximum size the string can hold without allocating
    size_t num_chars;       // Number of UTF-8 chars in the string
    size_t num_bytes;       // Number of raw uint8_t bytes in the string
    char *c_str;            // Inner, raw char * string
} str__str_t;

// Count the number of UTF-8 characters in a raw C string.
// Returns (size_t) -1 if the string contains invalid UTF-8.
static inline size_t str__utf8_char_count(const char *s) {
    size_t count = 0;
    const unsigned char *p = (const unsigned char *) s;
    while (*p) {
        if (*p < 0x80) {
            p += 1;
        } else if ((*p & 0xE0) == 0xC0) {
            if ((p[1] & 0xC0) != 0x80) return (size_t) -1;
            p += 2;
        } else if ((*p & 0xF0) == 0xE0) {
            if ((p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80) return (size_t) -1;
            p += 3;
        } else if ((*p & 0xF8) == 0xF0) {
            if ((p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80 || (p[3] & 0xC0) != 0x80)
                return (size_t) -1;
            p += 4;
        } else {
            return (size_t) -1; // Invalid leading byte
        }
        count++;
    }
    return count;
}

// Create new from literal
static inline str__str_t str__new(const char *literal) {
    return (str__str_t) {
        .capacity = strlen(literal) + 1,
        .num_bytes = strlen(literal),
        .num_chars = str__utf8_char_count(literal),
        .c_str = strdup(literal)
    };
}

// Clean up a string (use with DEFER to make easy)
static inline void str__delete(str__str_t *str) {
    if (!str) {
        return;
    }
    if (str->c_str) {
        free(str->c_str);
        str->c_str = 0;
    }
    str->capacity = 0;
    str->num_bytes = 0;
    str->num_chars = 0;
}

// Get out a heap-allocated array of decoded UTF-8 codepoints to iterate over.
// Array length is str->num_chars. Caller must free() the returned pointer.
// Returns NULL if str is NULL, empty, or contains invalid UTF-8.
static inline str__char_t *str__chars(const str__str_t *str) {
    if (!str || !str->c_str || str->num_chars == 0 || str->num_chars == (size_t) -1) {
        return NULL;
    }
    str__char_t *out = (str__char_t *) malloc(str->num_chars * sizeof(str__char_t));
    if (!out) {
        return NULL;
    }
    size_t idx = 0;
    const unsigned char *p = (const unsigned char *) str->c_str;
    while (*p && idx < str->num_chars) {
        if (*p < 0x80) {
            out[idx++] = *p++;
        } else if ((*p & 0xE0) == 0xC0) {
            out[idx++] = ((str__char_t)(p[0] & 0x1F) << 6)
                |  (str__char_t) (p[1] & 0x3F);
            p += 2;
        } else if ((*p & 0xF0) == 0xE0) {
            out[idx++] = ((str__char_t)(p[0] & 0x0F) << 12)
                | ((str__char_t) (p[1] & 0x3F) << 6)
                |  (str__char_t) (p[2] & 0x3F);
            p += 3;
        } else if ((*p & 0xF8) == 0xF0) {
            out[idx++] = ((str__char_t) (p[0] & 0x07) << 18)
                | ((str__char_t) (p[1] & 0x3F) << 12)
                | ((str__char_t) (p[2] & 0x3F) << 6)
                |  (str__char_t) (p[3] & 0x3F);
            p += 4;
        } else {
            free(out);
            return NULL;
        }
    }
    return out;
}

// Append one string onto us. Grows capacity if needed.
static inline void str__append(str__str_t *first, const str__str_t *second) {
    if (!first || !second || !second->c_str || second->num_bytes == 0) {
        return;
    }
    size_t new_bytes = first->num_bytes + second->num_bytes;
    if (new_bytes + 1 > first->capacity) {
        size_t new_cap = first->capacity;
        while (new_cap < new_bytes + 1) {
            new_cap *= 2;
        }
        char *new_buf = (char *) realloc(first->c_str, new_cap);
        if (!new_buf) {
            return;
        }
        first->c_str = new_buf;
        first->capacity = new_cap;
    }
    memcpy(first->c_str + first->num_bytes, second->c_str, second->num_bytes);
    first->num_bytes = new_bytes;
    first->num_chars += second->num_chars;
    first->c_str[first->num_bytes] = '\0';
}

// Append a C string onto us (could be a UTF-8 char or a full literal)
static inline void str__push_c_str(str__str_t *str, const char *literal) {
    if (!str || !literal) {
        return;
    }
    size_t lit_bytes = strlen(literal);
    if (lit_bytes == 0) {
        return;
    }
    size_t new_bytes = str->num_bytes + lit_bytes;
    if (new_bytes + 1 > str->capacity) {
        size_t new_cap = str->capacity;
        while (new_cap < new_bytes + 1) {
            new_cap *= 2;
        }
        char *new_buf = (char *) realloc(str->c_str, new_cap);
        if (!new_buf) {
            return;
        }
        str->c_str = new_buf;
        str->capacity = new_cap;
    }
    memcpy(str->c_str + str->num_bytes, literal, lit_bytes);
    str->num_bytes = new_bytes;
    str->num_chars += str__utf8_char_count(literal);
    str->c_str[str->num_bytes] = '\0';
}

// Remove the last character from the string. Returns the removed codepoint, or 0 if empty.
static inline str__char_t str__pop(str__str_t *str) {
    if (!str || !str->c_str || str->num_bytes == 0) {
        return 0;
    }
    // Walk backwards to find the start of the last UTF-8 sequence
    const unsigned char *p = (const unsigned char *) str->c_str;
    size_t i = str->num_bytes - 1;
    while (i > 0 && (p[i] & 0xC0) == 0x80) {
        i--;
    }
    // Decode the codepoint at position i
    str__char_t cp = 0;
    size_t seq_len = str->num_bytes - i;
    if (p[i] < 0x80) {
        cp = p[i];
    } else if ((p[i] & 0xE0) == 0xC0 && seq_len == 2) {
        cp = ((str__char_t) (p[i] & 0x1F) << 6)
           |  (str__char_t) (p[i + 1] & 0x3F);
    } else if ((p[i] & 0xF0) == 0xE0 && seq_len == 3) {
        cp = ((str__char_t) (p[i] & 0x0F) << 12)
           | ((str__char_t) (p[i + 1] & 0x3F) << 6)
           |  (str__char_t) (p[i + 2] & 0x3F);
    } else if ((p[i] & 0xF8) == 0xF0 && seq_len == 4) {
        cp = ((str__char_t) (p[i] & 0x07) << 18)
           | ((str__char_t) (p[i + 1] & 0x3F) << 12)
           | ((str__char_t) (p[i + 2] & 0x3F) << 6)
           |  (str__char_t) (p[i + 3] & 0x3F);
    }
    str->num_bytes = i;
    str->num_chars--;
    str->c_str[i] = '\0';
    return cp;
}

// Remove a single UTF-8 character at the given character index.
// Returns the removed codepoint, or 0 if index is out of bounds.
static inline str__char_t str__remove_at(str__str_t *str, size_t index) {
    if (!str || !str->c_str || index >= str->num_chars) {
        return 0;
    }
    // Walk forward to the byte offset of the target character
    const unsigned char *p = (const unsigned char *) str->c_str;
    size_t byte_off = 0;
    for (size_t ci = 0; ci < index; ci++) {
        if (p[byte_off] < 0x80) {
            byte_off += 1;
        } else if ((p[byte_off] & 0xE0) == 0xC0) {
            byte_off += 2;
        } else if ((p[byte_off] & 0xF0) == 0xE0) {
            byte_off += 3;
        } else if ((p[byte_off] & 0xF8) == 0xF0) {
            byte_off += 4;
        } else {
            return 0; // Invalid UTF-8
        }
    }
    // Determine the byte length of this character
    size_t seq_len = 1;
    if (p[byte_off] < 0x80) {
        seq_len = 1;
    } else if ((p[byte_off] & 0xE0) == 0xC0) {
        seq_len = 2;
    } else if ((p[byte_off] & 0xF0) == 0xE0) {
        seq_len = 3;
    } else if ((p[byte_off] & 0xF8) == 0xF0) {
        seq_len = 4;
    } else {
        return 0;
    }
    // Decode the codepoint
    str__char_t cp = 0;
    if (seq_len == 1) {
        cp = p[byte_off];
    } else if (seq_len == 2) {
        cp = ((str__char_t) (p[byte_off] & 0x1F) << 6)
           | (str__char_t) (p[byte_off + 1] & 0x3F);
    } else if (seq_len == 3) {
        cp = ((str__char_t) (p[byte_off] & 0x0F) << 12)
           | ((str__char_t) (p[byte_off + 1] & 0x3F) << 6)
           | (str__char_t) (p[byte_off + 2] & 0x3F);
    } else {
        cp = ((str__char_t) (p[byte_off] & 0x07) << 18)
           | ((str__char_t) (p[byte_off + 1] & 0x3F) << 12)
           | ((str__char_t) (p[byte_off + 2] & 0x3F) << 6)
           | (str__char_t) (p[byte_off + 3] & 0x3F);
    }
    // Shift the tail left over the removed bytes
    memmove(
        str->c_str + byte_off,
        str->c_str + byte_off + seq_len,
        str->num_bytes - byte_off - seq_len + 1 // +1 for null terminator
    );
    str->num_bytes -= seq_len;
    str->num_chars--;
    return cp;
}

// Insert a C string literal at the given character index.
// Index 0 inserts at the front, index == num_chars appends at the end.
static inline void str__insert(str__str_t *str, size_t index, const char *literal) {
    if (!str || !literal) {
        return;
    }
    if (index > str->num_chars) {
        return;
    }
    size_t lit_bytes = strlen(literal);
    if (lit_bytes == 0) {
        return;
    }
    // Walk forward to the byte offset of the target character index
    const unsigned char *p = (const unsigned char *) str->c_str;
    size_t byte_off = 0;
    for (size_t ci = 0; ci < index; ci++) {
        if (p[byte_off] < 0x80) {
            byte_off += 1;
        } else if ((p[byte_off] & 0xE0) == 0xC0) {
            byte_off += 2;
        } else if ((p[byte_off] & 0xF0) == 0xE0) {
            byte_off += 3;
        } else if ((p[byte_off] & 0xF8) == 0xF0) {
            byte_off += 4;
        } else {
            return; // Invalid UTF-8
        }
    }
    // Grow if needed
    size_t new_bytes = str->num_bytes + lit_bytes;
    if (new_bytes + 1 > str->capacity) {
        size_t new_cap = str->capacity;
        while (new_cap < new_bytes + 1) {
            new_cap *= 2;
        }
        char *new_buf = (char *) realloc(str->c_str, new_cap);
        if (!new_buf) {
            return;
        }
        str->c_str = new_buf;
        str->capacity = new_cap;
    }
    // Shift tail right to make room
    memmove(
        str->c_str + byte_off + lit_bytes,
        str->c_str + byte_off,
        str->num_bytes - byte_off + 1 // +1 for null terminator
    );
    // Copy literal into the gap
    memcpy(str->c_str + byte_off, literal, lit_bytes);
    str->num_bytes = new_bytes;
    str->num_chars += str__utf8_char_count(literal);
}

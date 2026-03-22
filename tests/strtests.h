// Description: Test suite for ECL str.h library
// Author:      Jan De Eeuwige <jan.de.eeuwige@proton.me>
// Copyright:   (C) 2026 Jan De Eeuwige
// License:     MIT

#pragma once

#include <str/str.h>

// -------- Test wrappers --------

static bool strtest_new_ascii(test__log_t *log) {
    str__str_t s = str__new("hello");
    if (s.num_chars != 5) {
        test__log_scream(log, "num_chars != 5 for ASCII string");
        str__delete(&s);
        return false;
    }
    if (s.num_bytes != 5) {
        test__log_scream(log, "num_bytes != 5 for ASCII string");
        str__delete(&s);
        return false;
    }
    if (strcmp(s.c_str, "hello") != 0) {
        test__log_scream(log, "c_str mismatch");
        str__delete(&s);
        return false;
    }
    test__log_inform(log, "ASCII: num_chars=5, num_bytes=5, c_str=\"hello\"");
    str__delete(&s);
    return true;
}

static bool strtest_new_utf8(test__log_t *log) {
    // "café" — é is 2 bytes (U+00E9)
    str__str_t s = str__new("caf\xc3\xa9");
    if (s.num_chars != 4) {
        char detail[128] = { 0 };
        snprintf(detail, sizeof(detail), "num_chars=%zu, expected 4", s.num_chars);
        test__log_scream(log, detail);
        str__delete(&s);
        return false;
    }
    if (s.num_bytes != 5) {
        char detail[128] = { 0 };
        snprintf(detail, sizeof(detail), "num_bytes=%zu, expected 5", s.num_bytes);
        test__log_scream(log, detail);
        str__delete(&s);
        return false;
    }
    test__log_inform(log, "UTF-8: num_chars=4, num_bytes=5 for \"cafe\\xcc\\xa9\"");
    str__delete(&s);
    return true;
}

static bool strtest_push_c_str(test__log_t *log) {
    str__str_t s = str__new("abc");
    str__push_c_str(&s, "d\xc3\xa9""f");  // "déf" — 3 chars, 4 bytes
    if (s.num_chars != 6) {
        char detail[128] = { 0 };
        snprintf(detail, sizeof(detail), "num_chars=%zu, expected 6", s.num_chars);
        test__log_scream(log, detail);
        str__delete(&s);
        return false;
    }
    if (s.num_bytes != 7) {
        char detail[128] = { 0 };
        snprintf(detail, sizeof(detail), "num_bytes=%zu, expected 7", s.num_bytes);
        test__log_scream(log, detail);
        str__delete(&s);
        return false;
    }
    if (strcmp(s.c_str, "abcd\xc3\xa9""f") != 0) {
        test__log_scream(log, "c_str content mismatch after push");
        str__delete(&s);
        return false;
    }
    test__log_inform(log, "push_c_str: \"abc\" + \"def\" correct");
    str__delete(&s);
    return true;
}

static bool strtest_append(test__log_t *log) {
    str__str_t a = str__new("hello ");
    str__str_t b = str__new("world");
    str__append(&a, &b);
    if (strcmp(a.c_str, "hello world") != 0) {
        test__log_scream(log, "c_str mismatch after append");
        str__delete(&a);
        str__delete(&b);
        return false;
    }
    if (a.num_chars != 11 || a.num_bytes != 11) {
        test__log_scream(log, "counts wrong after append");
        str__delete(&a);
        str__delete(&b);
        return false;
    }
    test__log_inform(log, "append: \"hello \" + \"world\" = \"hello world\"");
    str__delete(&a);
    str__delete(&b);
    return true;
}

static bool strtest_pop(test__log_t *log) {
    // "café" — pop should return é (U+00E9)
    str__str_t s = str__new("caf\xc3\xa9");
    str__char_t cp = str__pop(&s);
    if (cp != 0x00E9) {
        char detail[128] = { 0 };
        snprintf(detail, sizeof(detail), "Popped U+%04X, expected U+00E9", cp);
        test__log_scream(log, detail);
        str__delete(&s);
        return false;
    }
    if (s.num_chars != 3 || s.num_bytes != 3) {
        test__log_scream(log, "Counts wrong after pop");
        str__delete(&s);
        return false;
    }
    if (strcmp(s.c_str, "caf") != 0) {
        test__log_scream(log, "c_str wrong after pop");
        str__delete(&s);
        return false;
    }
    test__log_inform(log, "pop: \"cafe\" -> popped U+00E9, left \"caf\"");
    str__delete(&s);
    return true;
}

static bool strtest_remove_at(test__log_t *log) {
    // "aöb" — ö is U+00F6 (2 bytes), at char index 1
    str__str_t s = str__new("a\xc3\xb6""b");
    str__char_t cp = str__remove_at(&s, 1);
    if (cp != 0x00F6) {
        char detail[128] = { 0 };
        snprintf(detail, sizeof(detail), "Removed U+%04X, expected U+00F6", cp);
        test__log_scream(log, detail);
        str__delete(&s);
        return false;
    }
    if (strcmp(s.c_str, "ab") != 0) {
        test__log_scream(log, "c_str wrong after remove_at");
        str__delete(&s);
        return false;
    }
    if (s.num_chars != 2 || s.num_bytes != 2) {
        test__log_scream(log, "Counts wrong after remove_at");
        str__delete(&s);
        return false;
    }
    test__log_inform(log, "remove_at: removed U+00F6 from index 1");
    str__delete(&s);
    return true;
}

static bool strtest_insert(test__log_t *log) {
    str__str_t s = str__new("ac");
    // Insert at middle
    str__insert(&s, 1, "b");
    if (strcmp(s.c_str, "abc") != 0) {
        test__log_scream(log, "Insert at middle failed");
        str__delete(&s);
        return false;
    }
    // Insert at front
    str__insert(&s, 0, "Z");
    if (strcmp(s.c_str, "Zabc") != 0) {
        test__log_scream(log, "Insert at front failed");
        str__delete(&s);
        return false;
    }
    // Insert at end (== num_chars)
    str__insert(&s, s.num_chars, "!");
    if (strcmp(s.c_str, "Zabc!") != 0) {
        test__log_scream(log, "Insert at end failed");
        str__delete(&s);
        return false;
    }
    // Insert multi-byte at middle
    str__insert(&s, 2, "\xc3\xa9");  // é
    if (s.num_chars != 6 || s.num_bytes != 7) {
        test__log_scream(log, "Counts wrong after multi-byte insert");
        str__delete(&s);
        return false;
    }
    test__log_inform(log, "insert: front, middle, end, and multi-byte all correct");
    str__delete(&s);
    return true;
}

static bool strtest_chars_decode(test__log_t *log) {
    // "A€Z" — € is U+20AC (3 bytes)
    str__str_t s = str__new("A\xe2\x82\xac""Z");
    str__char_t *cps = str__chars(&s);
    if (!cps) {
        test__log_scream(log, "str__chars returned NULL");
        str__delete(&s);
        return false;
    }
    if (cps[0] != 'A' || cps[1] != 0x20AC || cps[2] != 'Z') {
        char detail[128] = { 0 };
        snprintf(
            detail, sizeof(detail),
            "Got [ U+%04X, U+%04X, U+%04X ], expected [ U+0041, U+20AC, U+005A ]",
            cps[0], cps[1], cps[2]
        );
        test__log_scream(log, detail);
        free(cps);
        str__delete(&s);
        return false;
    }
    test__log_inform(log, "chars: decoded [ U+0041, U+20AC, U+005A ] correctly");
    free(cps);
    str__delete(&s);
    return true;
}

static bool strtest_invalid_utf8(test__log_t *log) {
    // 0xFF is never valid in UTF-8
    size_t count = str__utf8_char_count("\xff""abc");
    if (count != (size_t) -1) {
        char detail[128] = { 0 };
        snprintf(detail, sizeof(detail), "Got %zu, expected (size_t)-1", count);
        test__log_scream(log, detail);
        return false;
    }
    // Truncated 2-byte sequence
    count = str__utf8_char_count("\xc3");
    if (count != (size_t) -1) {
        test__log_scream(log, "Truncated 2-byte seq not detected");
        return false;
    }
    test__log_inform(log, "Invalid UTF-8 correctly returns (size_t)-1");
    return true;
}
# ECL/String

## Description

Header-only advanced, dynamic string library

### Concept

A C-style implementation of Rust's `String`

Example:

```c
#include <stdio.h>
#include <stdlib.h>
#include <defer/defer.h>
#include <str/str.h>

BEGFN(int, example, (void))
    // Create from a UTF-8 literal
    str__str_t greeting = str__new("Hej ");
    DEFER(str__delete(&greeting));
    // num_chars=4, num_bytes=4

    // Append a C string with multi-byte chars
    str__push_c_str(&greeting, "världen!");
    // "Hej världen!" — num_chars=12, num_bytes=13 (ä is 2 bytes)

    // Insert at character index
    str__insert(&greeting, 4, "hela ");
    // "Hej hela världen!"

    // Iterate over decoded codepoints
    str__char_t *cps = str__chars(&greeting);
    DEFER(free(cps));
    if (cps) {
        for (size_t i = 0; i < greeting.num_chars; i++) {
            printf("U+%04X ", cps[i]);
        }
        printf("\n");
    }

    // Remove character at index (the 'ä' in världen)
    str__char_t removed = str__remove_at(&greeting, 13);
    printf("Removed: U+%04X\n", removed); // U+00E4

    // Pop last character
    str__char_t last = str__pop(&greeting);
    printf("Popped: U+%04X\n", last);

    // Append another str__str_t
    str__str_t suffix = str__new("!!");
    DEFER(str__delete(&suffix));
    str__append(&greeting, &suffix);

    printf("Result: %s\n", greeting.c_str);
    printf("Chars: %zu, Bytes: %zu\n", greeting.num_chars, greeting.num_bytes);

    RETFN(0);
ENDFN

int main(void) {
    return example();
}
```

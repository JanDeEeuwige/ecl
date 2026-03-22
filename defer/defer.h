// Description: Implementation of memory cleanup deferment for C
// Author:      Jan De Eeuwige <jan.de.eeuwige@proton.me>
// Copyright:   (C) 2026 Jan De Eeuwige
// License:     MIT

#pragma once

#include <stdio.h>
#include <stdlib.h>

// Override this to use an older version of the library
#ifndef DEFER__API_VERS
#define DEFER__API_VERS     1
#endif

// -------- Helper Macros --------

#ifndef DEFER__INIT_CAP
#define DEFER__INIT_CAP     256
#endif

#define DEFER__CAT2(a, b)   a##b
#define DEFER__CAT(a, b)    DEFER__CAT2(a, b)

#define DEFER__PANIC_OUT_OF_MEM() \
    do { \
        fprintf(stderr, "Defer panic: out of memory!\n"); \
        exit(1); \
    } while (0)
#define DEFER__GROW() \
    if (defer__count + 1 > defer__cap) { \
        defer__lbls = realloc( \
            defer__lbls, sizeof(void *) * defer__cap * 2 \
        ); \
        if (!defer__lbls) { \
            DEFER__PANIC_OUT_OF_MEM(); \
        } \
        defer__cap *= 2; \
    }
#define DEFER__LOCALS() \
    int defer__count = 0; \
    int defer__cap = DEFER__INIT_CAP; \
    void **defer__lbls = malloc(sizeof(void *) * DEFER__INIT_CAP); \
    if (!defer__lbls) { \
        DEFER__PANIC_OUT_OF_MEM(); \
    }
#define DEFER__UNWIND() \
    goto defer__exec; \
defer__exec: \
    if (defer__count > 0) { \
        defer__count--; \
        goto *defer__lbls[defer__count]; \
    } \
defer__done: __attribute__((unused)); \
    free(defer__lbls);
#define DEFER__INNER(id, ...) \
    DEFER__GROW() \
    defer__lbls[defer__count++] = &&DEFER__CAT(_def_, id); \
    if (0) { \
        DEFER__CAT(_def_, id): \
        { __VA_ARGS__; } \
        goto defer__exec; \
    }

// -------- Function (Non-Voids) Helpers --------

#define BEGFN(ret_type, name, params) \
    ret_type name params { \
    DEFER__LOCALS() \
    ret_type defer__retval = (ret_type) { 0 };
#define ENDFN \
    DEFER__UNWIND() \
    return defer__retval; \
}
#define RETFN(val) do { defer__retval = (val); goto defer__exec; } while (0)

// -------- Procedure (Void) Helpers --------

#define BEGPROC(name, params) \
    void name params { \
    DEFER__LOCALS()
#define ENDPROC \
    DEFER__UNWIND() \
    return; \
}
#define RETPROC do { goto defer__exec; } while (0)

// -------- Defer Itself ---------

#define DEFER(...) DEFER__INNER(__COUNTER__, __VA_ARGS__)

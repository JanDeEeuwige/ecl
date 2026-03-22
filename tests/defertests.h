// Description: Test suite for ECL defer.h library
//              NOTE: Tested independently so that other libraries can rely on it for cleanup
//              in their own internal functions and tests if need be
// Author:      Jan De Eeuwige <jan.de.eeuwige@proton.me>
// Copyright:   (C) 2026 Jan De Eeuwige
// License:     MIT

#pragma once

#include <defer/defer.h>

// -------- Helper functions using defer macros --------

// Single defer increments an int
BEGPROC(deftest__single_defer, (int *out))
    DEFER(*out += 1);
ENDPROC

// Multiple defers write indices in LIFO order into a buffer
BEGPROC(deftest__lifo_order, (int *buf, int *count))
    DEFER(buf[(*count)++] = 0);
    DEFER(buf[(*count)++] = 1);
    DEFER(buf[(*count)++] = 2);
ENDPROC

// Early RETFN still fires defers and returns correct value
BEGFN(int, deftest__early_retfn, (int *fired))
    DEFER(*fired = 1);
    RETFN(42);
    // Should never reach here
    *fired = -1;
ENDFN

// Early RETPROC fires defers
BEGPROC(deftest__early_retproc, (int *fired))
    DEFER(*fired = 1);
    RETPROC;
    *fired = -1;
ENDPROC

// Fall-through ENDFN returns zero-init and fires defers
BEGFN(int, deftest__fallthrough, (int *fired))
    DEFER(*fired = 1);
ENDFN

// Push enough defers to force at least one DEFER__GROW realloc
#define DEFTEST__GROW_COUNT (DEFER__INIT_CAP + 16)
BEGPROC(deftest__stack_growth, (int *counter))
    for (int i = 0; i < DEFTEST__GROW_COUNT; i++) {
        DEFER(*counter += 1);
    }
ENDPROC

// -------- Test wrappers --------

static bool deftest_single_defer(test__log_t *log) {
    int val = 0;
    deftest__single_defer(&val);
    if (val != 1) {
        test__log_scream(log, "Defer block did not fire");
        return false;
    }
    test__log_inform(log, "Single defer fired correctly");
    return true;
}

static bool deftest_lifo_order(test__log_t *log) {
    int buf[3] = { 0 };
    int count = 0;
    deftest__lifo_order(buf, &count);
    // Defers fire LIFO: last registered (2) fires first, then 1, then 0
    if (count != 3 || buf[0] != 2 || buf[1] != 1 || buf[2] != 0) {
        test__log_scream(log, "Defers did not fire in LIFO order");
        char detail[128] = { 0 };
        snprintf(
            detail, sizeof(detail),
            "Got [ %d, %d, %d ] count = %d, expected [ 2, 1, 0 ] count = 3",
            buf[0], buf[1], buf[2], count
        );
        test__log_scream(log, detail);
        return false;
    }
    test__log_inform(log, "LIFO order verified: [ 2, 1, 0 ]");
    return true;
}

static bool deftest_early_retfn(test__log_t *log) {
    int fired = 0;
    int ret = deftest__early_retfn(&fired);
    if (fired != 1) {
        test__log_scream(log, "Defer did not fire on early RETFN");
        return false;
    }
    if (ret != 42) {
        test__log_scream(log, "RETFN returned wrong value");
        return false;
    }
    test__log_inform(log, "Early RETFN: defer fired, returned 42");
    return true;
}

static bool deftest_early_retproc(test__log_t *log) {
    int fired = 0;
    deftest__early_retproc(&fired);
    if (fired != 1) {
        test__log_scream(log, "Defer did not fire on early RETPROC");
        return false;
    }
    test__log_inform(log, "Early RETPROC: defer fired");
    return true;
}

static bool deftest_fallthrough(test__log_t *log) {
    int fired = 0;
    int ret = deftest__fallthrough(&fired);
    if (fired != 1) {
        test__log_scream(log, "Defer did not fire on fall-through ENDFN");
        return false;
    }
    if (ret != 0) {
        test__log_scream(log, "Fall-through ENDFN did not return zero-init value");
        return false;
    }
    test__log_inform(log, "Fall-through: defer fired, returned 0");
    return true;
}

static bool deftest_stack_growth(test__log_t *log) {
    int counter = 0;
    deftest__stack_growth(&counter);
    if (counter != DEFTEST__GROW_COUNT) {
        char detail[128] = { 0 };
        snprintf(
            detail, sizeof(detail), "Expected %d defers to fire, got %d",
            DEFTEST__GROW_COUNT, counter
        );
        test__log_scream(log, detail);
        return false;
    }
    char detail[128] = { 0 };
    snprintf(detail, sizeof(detail), "All %d defers fired after stack growth", DEFTEST__GROW_COUNT);
    test__log_inform(log, detail);
    return true;
}
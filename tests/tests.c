// Description: Run ECL test suite
//              Build: gcc -O2 -g0 -Wall -Werror -Wextra -I../ -O ecltests tests.c
// Author:      Jan De Eeuwige <jan.de.eeuwige@proton.me>
// Copyright:   (C) 2026 Jan De Eeuwige
// License:     MIT

#include <test/test.h>
#include "defertests.h"
#include "strtests.h"

static bool basic_test(test__log_t *log) {
    test__log_inform(log, "This is INFO");
    test__log_warn(log, "This is WARN");
    test__log_scream(log, "This is ERR");
    return true;
}

static const test__test_t g_tests[] = {
    (test__test_t) { .category = "System", .name = "Basic Test", .fn = basic_test },

    // Defer tests
    { .category = "Defer", .name = "Single Defer",   .fn = deftest_single_defer },
    { .category = "Defer", .name = "LIFO Order",     .fn = deftest_lifo_order },
    { .category = "Defer", .name = "Early RETFN",    .fn = deftest_early_retfn },
    { .category = "Defer", .name = "Early RETPROC",  .fn = deftest_early_retproc },
    { .category = "Defer", .name = "Fall-Through",   .fn = deftest_fallthrough },
    { .category = "Defer", .name = "Stack Growth",   .fn = deftest_stack_growth },

    // String tests
    { .category = "String", .name = "New ASCII",      .fn = strtest_new_ascii },
    { .category = "String", .name = "New UTF-8",      .fn = strtest_new_utf8 },
    { .category = "String", .name = "Push C String",  .fn = strtest_push_c_str },
    { .category = "String", .name = "Append",         .fn = strtest_append },
    { .category = "String", .name = "Pop",            .fn = strtest_pop },
    { .category = "String", .name = "Remove At",      .fn = strtest_remove_at },
    { .category = "String", .name = "Insert",         .fn = strtest_insert },
    { .category = "String", .name = "Chars Decode",   .fn = strtest_chars_decode },
    { .category = "String", .name = "Invalid UTF-8",  .fn = strtest_invalid_utf8 },
};

int main(int argc, const char **argv) {
    test__args_t *args = test__args_parse(argc, argv);
    test__run_all(g_tests, sizeof(g_tests) / sizeof(test__test_t), args);
    test__args_delete(args);
    free(args);
}

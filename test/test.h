// Description: Simple C Test framework
// Author:      Jan De Eeuwige <jan.de.eeuwige@proton.me>
// Copyright:   Jan De Eeuwige
// License:     MIT

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// -------- Logging System --------

#define TEST__DEF_LOG_CAP       256 // Default number of messages the log can have
#define TEST__LOG_COL_GREEN     "\033[32m"
#define TEST__LOG_COL_YELLOW    "\033[33m"
#define TEST__LOG_COL_RED       "\033[31m"
#define TEST__LOG_COL_RESET     "\033[0m"

// The severity of test log messages
typedef enum {
    TEST__MSGLVL_INFO,      // Extra info for debugging
    TEST__MSGLVL_WARN,      // Something went wrong, but not catastrophic/maybe not a fail
    TEST__MSGLVL_ERR        // Something went catastrophically wrong
} test__msg_lvl_t;

// Message in the log
typedef struct {
    test__msg_lvl_t lvl;
    time_t time;
    char *msg; // Heap-allocated. Must be cleaned up
} test__msg_t;

// Collection of messages for a log
typedef struct {
    char *full_test_name; // "Category :: Test Name", heap-allocated
    size_t num_msgs;
    size_t max_msgs;
    test__msg_t **msgs;
} test__log_t;

static inline test__log_t test__log_new(const char *full_test_name) {
    char *m_full_test_name = strdup(full_test_name);
    test__msg_t **msgs = (test__msg_t **) calloc(TEST__DEF_LOG_CAP, sizeof(test__msg_t *));
    if (!msgs) {
        fprintf(stderr, "Panic! Out of memory in new test log\n");
        exit(1);
    }
    return (test__log_t) {
        .full_test_name = m_full_test_name,
        .num_msgs = 0,
        .max_msgs = TEST__DEF_LOG_CAP,
        .msgs = msgs
    };
}

static inline void test__msg_delete(test__msg_t *msg) {
    if (!msg) {
        return;
    }
    if (msg->msg) {
        free(msg->msg);
        msg->msg = 0;
    }
}

static inline void test__log_delete(test__log_t *log) {
    if (!log) {
        return;
    }
    if (log->full_test_name) {
        free(log->full_test_name);
        log->full_test_name = NULL;
    }
    log->num_msgs = 0;
    if (log->msgs) {
        for (size_t i = 0; i < log->max_msgs; i++) {
            if (log->msgs[0]) {
                test__msg_delete(log->msgs[0]);
            }
            free(log->msgs[0]);
            log->msgs[0] = NULL;
        }
        log->max_msgs = 0;
    }
}

static inline void test__log_resize_if_cant_append(test__log_t *log) {
    if (log->num_msgs + 1 > log->max_msgs) {
        test__msg_t **new_msgs = (test__msg_t **) calloc(log->max_msgs * 2, sizeof(test__msg_t *));
        if (!new_msgs) {
            fprintf(stderr, "Panic! Out of memory when resizing test log\n");
            exit(1);
        }
        memcpy(new_msgs, log->msgs, log->num_msgs * sizeof(test__msg_t *));
        log->max_msgs *= 2;
        if (log->msgs) {
            free(log->msgs);
        }
        log->msgs = new_msgs;
    }
}

static inline void test__log_inform(test__log_t *log, const char *msg) {
    test__msg_t *new_msg = (test__msg_t *) calloc(1, sizeof(test__msg_t));
    if (!new_msg) {
        fprintf(stderr, "Panic! Out of memory when logging info\n");
        exit(1);
    }
    new_msg->lvl = TEST__MSGLVL_INFO;
    new_msg->time = time(NULL);
    new_msg->msg = strdup(msg);

    test__log_resize_if_cant_append(log);
    log->msgs[log->num_msgs] = new_msg;
    log->num_msgs++;
}

static inline void test__log_warn(test__log_t *log, const char *msg) {
    test__msg_t *new_msg = (test__msg_t *) calloc(1, sizeof(test__msg_t));
    if (!new_msg) {
        fprintf(stderr, "Panic! Out of memory when logging warning\n");
        exit(1);
    }
    new_msg->lvl = TEST__MSGLVL_WARN;
    new_msg->time = time(NULL);
    new_msg->msg = strdup(msg);

    test__log_resize_if_cant_append(log);
    log->msgs[log->num_msgs] = new_msg;
    log->num_msgs++;
}

static inline void test__log_scream(test__log_t *log, const char *msg) {
    test__msg_t *new_msg = (test__msg_t *) calloc(1, sizeof(test__msg_t));
    if (!new_msg) {
        fprintf(stderr, "Panic! Out of memory when logging error\n");
        exit(1);
    }
    new_msg->lvl = TEST__MSGLVL_ERR;
    new_msg->time = time(NULL);
    new_msg->msg = strdup(msg);

    test__log_resize_if_cant_append(log);
    log->msgs[log->num_msgs] = new_msg;
    log->num_msgs++;
}

static inline void test__msg_print(const test__msg_t *msg) {
    if (!msg) {
        return;
    }
    char time_buff[20]; // YYY-MM-DD HH:MM:SS = 19 + 1 for null
    struct tm *tm = localtime(&msg->time);
    strftime(time_buff, 20, "%Y-%m-%d %H:%M:%S", tm);
    switch (msg->lvl) {
        case TEST__MSGLVL_INFO:
            printf("INFO (%s) %s\n", time_buff, msg->msg);
            break;
        case TEST__MSGLVL_WARN:
            printf(
                TEST__LOG_COL_YELLOW "WARN (%s) %s" TEST__LOG_COL_RESET "\n",
                time_buff, msg->msg
            );
            break;
        case TEST__MSGLVL_ERR:
            printf(
                TEST__LOG_COL_RED "ERR  (%s) %s" TEST__LOG_COL_RESET "\n",
                time_buff, msg->msg
            );
            break;
    }
}

static inline void test__log_print(const test__log_t *log) {
    if (!log) {
        return;
    }
    for (size_t i = 0; i < log->num_msgs; i++) {
        test__msg_print(log->msgs[i]);
    }
}

// -------- CLI Arguments --------

// Data that can alter how tests are run
typedef struct {
    bool print_on_pass; // Print the log even on success (normally would skip)

    size_t num_skips;
    char **skips; // Heap allocated list of categories of tests to ignore
} test__args_t;

static inline void test__args_print_usage(void) {
    printf(
        "ECL Tests by Jan De Eeuwige\n"
        "USAGE: ./tests [OPTIONS]\n"
        "Options:\n"
        "  -h, --help                Show this info\n"
        "  -s, --skip CATEGORY       Skip tests in this category (repeatable)\n"
        "  -p, --print-on-pass       Print log messages even on pass\n"
    );
}

static inline test__args_t *test__args_parse(int argc, const char **argv) {
    test__args_t *args = (test__args_t *) calloc(1, sizeof(test__args_t));
    if (!args) {
        fprintf(stderr, "Panic! Out of memory allocating test args\n");
        exit(1);
    }
    args->print_on_pass = false;
    args->num_skips = 0;
    args->skips = NULL;
    size_t skip_cap = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            test__args_print_usage();
            free(args);
            exit(0);
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--skip") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Expected CATEGORY after %s\n\n", argv[i]);
                test__args_print_usage();
                free(args);
                exit(1);
            }
            i++;
            // Deduplicate
            bool found = false;
            for (size_t j = 0; j < args->num_skips; j++) {
                if (strcmp(args->skips[j], argv[i]) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                if (args->num_skips >= skip_cap) {
                    skip_cap = skip_cap == 0 ? 8 : skip_cap * 2;
                    char **new_skips = (char **) realloc(args->skips, skip_cap * sizeof(char *));
                    if (!new_skips) {
                        fprintf(stderr, "Panic! Out of memory growing skips list\n");
                        exit(1);
                    }
                    args->skips = new_skips;
                }
                args->skips[args->num_skips] = strdup(argv[i]);
                args->num_skips++;
            }
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--print-on-pass") == 0) {
            args->print_on_pass = true;
        } else {
            fprintf(stderr, "Unknown argument '%s'\n\n", argv[i]);
            test__args_print_usage();
            free(args);
            exit(1);
        }
    }
    return args;
}

void test__args_delete(test__args_t *args) {
    if (!args) {
        return;
    }
    if (args->skips) {
        for (size_t i = 0; i < args->num_skips; i++) {
            if (args->skips[i]) {
                free(args->skips[i]);
                args->skips[i] = NULL;
            }
        }
        free(args->skips);
        args->skips = NULL;
    }
}

// -------- Tests --------

// The type of object a test actually is, a fn
typedef bool (*test__fn_t)(test__log_t *log);

// Collection of test fn and metadata
typedef struct {
    char *category;
    char *name;
    test__fn_t fn;
} test__test_t;

static inline void test__run_all(const test__test_t *tests, size_t num_tests, const test__args_t *args) {
    size_t num_passed = 0;
    size_t num_failed = 0;
    size_t num_skipped = 0;
    for (size_t i = 0; i < num_tests; i++) {
        printf("[ Test: %s :: %s ] ", tests[i].category, tests[i].name);
        bool skipped = false;
        for (size_t j = 0; j < args->num_skips; j++) {
            if (strcmp(args->skips[j], tests[i].category) == 0) {
                printf(TEST__LOG_COL_YELLOW "SKIPPED" TEST__LOG_COL_RESET "\n");
                num_skipped++;
                skipped = true;
                break;
            }
        }
        if (skipped) {
            continue;
        }
        size_t name_len = strlen(tests[i].category) + strlen(tests[i].name) + strlen(" :: ");
        char *full_name = (char *) calloc(name_len + 1, 1);
        snprintf(full_name, name_len + 1, "%s :: %s", tests[i].category, tests[i].name);
        test__log_t log = test__log_new(full_name);
        free(full_name);
        bool result = tests[i].fn(&log);
        if (result) {
            printf(TEST__LOG_COL_GREEN "PASSED" TEST__LOG_COL_RESET "\n");
            if (args->print_on_pass) {
                test__log_print(&log);
            }
            num_passed++;
        } else {
            printf(TEST__LOG_COL_RED "FAILED" TEST__LOG_COL_RESET "\n");
            test__log_print(&log);
            num_failed++;
        }
        test__log_delete(&log);
    }
    printf(
        "\nPassed: %zu/%zu, Failed: %zu/%zu, Skipped: %zu\n",
        num_passed, num_tests - num_skipped, num_failed, num_tests - num_skipped, num_skipped
    );
}


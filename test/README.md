# ECL/Test

## Description

Header-only C library for unit tests

A Test is defined as a function that takes a log and returns pass/failure (while appending to the log if possible)

### Concept

Define a number of tests like:

```c
bool my_test(test__log_t *log) {
    test__log_info(log, "Hello, world!\n");
    return true;
}
```

Then, create a main function:

```c
test__test_t g_tests[] = {
    (test__test_t) {
        .category = "Simple",
        .name = "My Test",
        .fn = my_test
    }
};
int main(int argc, char **argv) {
    test__args_t *args = test__args_parse(argc, argv);
    test__run_all(g_tests, sizeof(g_tests) / sizeof(test__test_t), args);
    test__args_delete(args);
}
```


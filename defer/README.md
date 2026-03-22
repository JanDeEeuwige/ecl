# ECL/Defer

## Description

Header-only C library to implement Zig/Odin-style defer system

### Concept

You use it by placing a macro call at the start and end of functions with the "real" return, then you use a custom return replacement macro to allow jumping to cleanup instead of actually returning.

Under the hood it works by creating a little label for cleanup and adding its address to a list of label names then calls them at the real return.

Example:

```c
BEGFN(int, open_and_read, (const char *path, size_t len))
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        RETFN(-1);
    }
    DEFR({ close(fd); });

    void *buf = malloc(len);
    if (!buf) {
        RETFN(-1);
    }
    DEFR({ free(buf); });

    if (read(fd, buf, len) < 0) {
        RETFN(-1);
    }
    RETFN(0);
ENDFN
```

Becomes:

```c
#include <defer.h>
int open_and_read(const char *path, size_t len) {
    int _retval = (int) { 0 };
    int _def_count = 0;
    int _def_cap = 256;
    void **_def_lbls = malloc(sizeof(void *) * 256);
    if (!_def_lbls) {
        fprintf(stderr, "Defer panic: out of memory!\n");
        exit(1);
    }
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        do {
            _retval = (-1);
            goto _def_exec;
        } while (0);
    }
    if (_def_count + 1 > _def_cap) {
        _def_lbls = realloc(_def_lbls, sizeof(void *) * _def_cap * 2);
        if (!_def_lbls) {
            fprintf(stderr, "Defer panic: out of memory!\n");
            exit(1);
        }
        _def_cap *= 2;
    }
    _def_lbls[_def_count] = &&_def0;
    _def_count++;
    if (0) {
_def0:
            { close(fd); }
            goto _def_exec;
    }

    void *buf = malloc(len);
    if (!buf) {
        do {
            _retval = (-1);
            goto _def_exec;
        } while (0);
    }
    if (_def_count + 1 > _def_cap) {
        _def_lbls = realloc(_def_lbls, sizeof(void *) * _def_cap * 2);
        if (!_def_lbls) {
            fprintf(stderr, "Defer panic: out of memory!\n");
            exit(1);
        }
        _def_cap *= 2;
    }
    _def_lbls[_def_count] = &&_def1;
    _def_count++;
    if (0) {
_def1:
            { free(buf); }
            goto _def_exec;
    }

    if (read(fd, buf, len) < 0) {
        do {
            _retval = (-1);
            goto _def_exec;
        } while (0);
    }
    do {
        _retval = (0);
        goto _def_exec;
    } while (0);

    goto _def_exec;
_def_exec:
    if (_def_count > 0) {
        _def_count--;
        goto *_def_lbls[_def_count];
    }
_def_done:
    free(_def_lbls);
    return _retval;
}
```

For void functions, you'd use separate macros for the function stuff:

- `BEGPROC(name, params)`
- `ENDPROC`
- `RETPROC`


# Eternal C Library

## Description

Set of header-only helper libraries for C projects

You can use one, some, or all of these for whatever you need

Includes:

- `defer/` - Defer system (a la Zig/Odin)
- `str/` - UTF-8 String Builder
- `test/` - Simple C test framework (not to be confused with `tests/`, our test binary lol)

## Distribution

We use the MIT license and everything is header only. Just add the files to your project as needed!

To run the tests, build the test bin, `./tests/tests.c`:

```sh
cd tests
gcc -O2 -g0 -Wall -Werror -Wextra -I../ -O ecltests tests.c
./ecltests <args>
```

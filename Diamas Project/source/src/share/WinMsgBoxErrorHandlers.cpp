#include <Config.hpp>

#include <cstdlib>
#include <cstdio>

#include <windows.h>

namespace boost
{
void throw_exception(std::exception const &e)
{
    char str[1024];

    _snprintf(str, 1024,
              "Encountered an uncaught exception with message %s",
              e.what());
    MessageBoxA(NULL, str, "Unhandled boost exception", MB_ICONERROR);

    std::abort();
}
}

namespace storm
{
bool HandleAssertionFailure(const char *filename, int lineno,
                            const char *functionName, const char *expr,
                            const char *msg)
{
    char str[1024];

    _snprintf(str, 1024,
              "Encountered an assertion failure at [%s:%d] %s with message: %s\n",
              filename, lineno, functionName, msg);
    MessageBoxA(NULL, str, "Storm assertion failure", MB_ICONERROR);

    std::abort();
    return true;
}

bool HandleFatalFailure(const char *filename, int lineno,
                        const char *functionName,
                        const char *msg)
{
    char str[1024];

    _snprintf(str, 1024,
              "Encountered a fatal failure at [%s:%d] %s with message: %s\n",
              filename, lineno, functionName, msg);
    MessageBoxA(NULL, str, "Storm fatal failure", MB_ICONERROR);

    std::abort();
    return true;
}
}

#include <Config.hpp>

#include <cstdlib>
#include <cstdio>
#include <exception>

namespace boost
{

void throw_exception(std::exception const& e)
{
	std::fprintf(stderr,
	             "Encountered an uncaught exception with message %s",
	             e.what());

	std::abort();
}

}

namespace storm
{

bool HandleAssertionFailure(const char* filename, int lineno,
                            const char* functionName, const char* expr,
                            const char* msg)
{
	std::fprintf(stderr,
	             "Encountered an assertion failure at [%s:%d] %s with message: %s\n",
	             filename, lineno, functionName, msg);

	std::abort();
	return true;
}

bool HandleFatalFailure(const char* filename, int lineno,
                        const char* functionName,
                        const char* msg)
{
	std::fprintf(stderr,
	             "Encountered a fatal failure at [%s:%d] %s with message: %s\n",
	             filename, lineno, functionName, msg);

	std::abort();
	return true;
}

}

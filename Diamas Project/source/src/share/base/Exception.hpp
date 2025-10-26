#ifndef METIN2_BASE_EXCEPTION_HPP
#define METIN2_BASE_EXCEPTION_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <stdexcept>
#include <utility>


class Exception : public std::exception
{
	public:
		Exception(std::string what)
			: m_what(std::move(what))
		{
			// ctor
		}

		virtual const char *what() const BOOST_NOEXCEPT
		{
			return m_what.c_str();
		}

	private:
		std::string m_what;
};

template <class... A>
void ThrowErrorIfNot(bool value, const std::string_view& format, A&&... args)
{
	if (value)
		return;

	std::string s = fmt::format(format, std::forward<A>(args)...);
	throw Exception(std::move(s));
}



#endif

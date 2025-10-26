//
// Copyright (c) 2012-2014 Tim Niederhausen (tim@rnc-ag.de)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef STORM_TOKENIZER_IMPL_HPP
#define STORM_TOKENIZER_IMPL_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif
#include <boost/config/detail/suffix.hpp>


namespace storm
{

BOOST_FORCEINLINE IsChar::IsChar(char ch)
	: ch(ch)
{
	// ctor
}

BOOST_FORCEINLINE size_t IsChar::operator()(const std::string& str) const
{
	auto p = str.find(ch);
	return p;
}

BOOST_FORCEINLINE IsString::IsString(const std::string& s)
	: s(s)
{
	// ctor
}

BOOST_FORCEINLINE size_t IsString::operator()(const std::string& str) const
{
	auto p = str.find(s);
	return p;
}

BOOST_FORCEINLINE IsAnyOf::IsAnyOf(const std::string& s)
	: s(s)
{
	// ctor
}

BOOST_FORCEINLINE size_t IsAnyOf::operator()(const std::string& str) const
{
	auto p = str.find_first_of(s);
	return p;
}

BOOST_FORCEINLINE Tokenizer::Tokenizer(const std::string& input)
	: m_input(input)
	, m_begin(input.begin())
	, m_end(input.begin())
	, m_isFirstToken(true)
{
	// ctor
}

BOOST_FORCEINLINE std::string Tokenizer::GetCurrent() const
{ return std::string(m_begin, m_end); }

template <class Predicate>
bool Tokenizer::Next(const Predicate& p)
{
	if (m_end != m_input.end()) {
		// Advance at least by one character if we're not being called for the
		// first time. This avoids the edge case in which |p| returns
		// zero for the first token.
		m_begin = m_end + !m_isFirstToken;
		m_end = m_begin + p(std::string(m_begin, m_input.cend()));
		m_isFirstToken = false;
		return true;
	}

	m_begin = m_end;
	return false;
}

template <class Container, class Predicate>
void Tokenize(const std::string& input,
              const Predicate& p,
              Container& container)
{
	Tokenizer tok(input);
	while (tok.Next(p))
		container.push_back(tok.GetCurrent());
}

}

#endif

#ifndef METIN2_CLIENT_SCRIPTLIB_PYSTR_HPP
#define METIN2_CLIENT_SCRIPTLIB_PYSTR_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif



#include <string>
#include <string_view>

struct _object;
using PyObject = _object;

class PyStr
{
	public:
		PyStr();
		PyStr(const char* str);
		PyStr(const std::string & str);
		PyStr(std::string_view str);
		PyStr(const PyStr& str);
		~PyStr();

		PyStr& operator=(const PyStr& str);

		PyObject* Get() const;
		std::string_view ToString() const;

	private:
		PyObject* m_o;
};


#endif

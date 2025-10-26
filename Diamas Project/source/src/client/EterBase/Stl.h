#ifndef __INC_ETERBASE_STL_H__
#define __INC_ETERBASE_STL_H__

#pragma warning(disable:4786)	// identifier was truncated to '255' characters in the browser information
#pragma warning(disable:4018)	// signed <-> unsigned mismatch
#pragma warning(disable:4503)	// decorated name length exceeded, name was truncated
#pragma warning(disable:4018)	// '<' : signed/unsigned mismatch

#include <assert.h>

#pragma warning ( push, 3 )

#include <algorithm>
#include <string>
#include <vector>
#include <stack>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <queue>
#include <functional>
#include <SSTREAM>

#pragma warning ( pop )

extern char korean_tolower(const char c);
extern std::string& stl_static_string(const char* c_sz);
extern void stl_lowers(std::string& rstRet);
extern int split_string(const std::string & input, const std::string & delimiter, std::vector<std::string>& results, bool includeEmpties);


struct stl_sz_less
{
	bool operator() (char * const & left, char * const & right) const
	{
		return (strcmp(left, right) < 0);
	}
};

template<typename TContainer>
inline void stl_wipe(TContainer& container)
{
	for (auto i = container.begin(); i != container.end(); ++i)
	{
		delete *i;
		*i = nullptr;
	}
	
	container.clear();
}

template<typename TString>
constexpr int hex2dec(TString szhex)
{
	const int hex0 = toupper(szhex[0]);
	const int hex1 = toupper(szhex[1]);

	return (hex1 >= 'A' ? hex1 - 'A' + 10 : hex1 - '0') +
		   (hex0 >= 'A' ? hex0 - 'A' + 10 : hex0 - '0') * 16;
}

template<typename TString>
constexpr uint32_t htmlColorStringToARGB(TString str)
{
	const uint32_t alp	= hex2dec(str);
	const uint32_t red	= hex2dec(str + 2);
	const uint32_t green	= hex2dec(str + 4);
	const uint32_t blue	= hex2dec(str + 6);
	return (alp << 24 | red << 16 | green << 8 | blue);
}

template<typename TContainer>
inline void stl_wipe_second(TContainer& container)
{
	for (auto i = container.begin(); i != container.end(); ++i)
	{
		delete i->second;
	}
	
	container.clear();
}


template<typename T>
inline void safe_release(T& rpObject, uint8_t expected = 0, const char* file=nullptr, int line=0)
{	
	if (!rpObject)
		return;
	
	auto count = rpObject->Release();
	if(count != expected) {
		auto fmt = fmt::format("{}:{} {:p} RefCount is {} (expected {}).\r\n", file, line, fmt::ptr(rpObject), count, expected).c_str();
		OutputDebugString(fmt);
	}
	rpObject = NULL;
}

#define M2_SAFE_RELEASE(_obj, _expected) \
	 safe_release(_obj, _expected, __FILE__, __LINE__) \

#define M2_SAFE_RELEASE_CHECK(_obj) \
	 safe_release(_obj, 0, __FILE__, __LINE__) \

using CTokenVector = std::vector<std::string>;
using CTokenMap = std::map<std::string, std::string>;
using CTokenVectorMap = std::map<std::string, CTokenVector>;

//class CTokenVector : public std::vector<std::string>


struct stringhash
{
	size_t GetHash(const std::string & str) const
	{
       const unsigned char * s = (const unsigned char*) str.c_str();
       const unsigned char * end = s + str.size();
       size_t h = 0;

       while (s < end)
       {
           h *= 16777619;
           h ^= (unsigned char) *(unsigned char *) (s++);
       }

       return h;
	}

    size_t operator () (const std::string & str) const
    {
		return GetHash(str);
    }
};

char* nth_strchr(const char* s, int c, int n);

#endif

#ifndef __INC_LIBTHECORE_UTILS_H__
#define __INC_LIBTHECORE_UTILS_H__

#include <random>
#include <iterator>
#include <string>
#include <storm/StringUtil.hpp>


#define SAFE_FREE(p)		{ if (p) { free( (void *) p);		(p) = NULL;  } }
#define SAFE_DELETE(p)		{ if (p) { delete (p);			(p) = NULL;  } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[] (p);		(p) = NULL;  } }
#define SAFE_RELEASE(p)		{ if (p) { (p)->Release();		(p) = NULL;  } }

#define LOWER(c)	(((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)	(((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c))

#define str_cmp strcasecmp
#define STRNCPY(dst, src, len)          do {strncpy(dst, src, len); dst[len] = '\0'; } while(0)

    extern char *	str_dup(const char * source);	// 메모리 할당 해서 source 복사 한거 리턴
    extern int		filesize(FILE * fp);	// 파일 크기 리턴

#define core_dump()	core_dump_unix(__FILE__, __LINE__)
    extern void		core_dump_unix(const char *who, long line);	// 코어를 강제로 덤프

#define TOKEN(string) if (!str_cmp(token_string, string))
    // src = 토큰 : 값

    extern void		trim_and_lower(const char * src, char * dest, size_t dest_size);

    // 문자열을 소문자로
    extern void		lower_string(const char * src, char * dest, size_t dest_len);


    // a와 b의 시간이 얼마나 차이나는지 리턴
    extern struct timeval *	timediff(const struct timeval *a, const struct timeval *b);

    // a의 시간에 b의 시간을 더해 리턴
    extern struct timeval *	timeadd(struct timeval *a, struct timeval *b);

	extern void thecore_sleep(struct timeval* timeout); // timeout
	extern void thecore_msleep(uint32_t dwMillisecond);

    extern float	get_float_time();
    extern uint64_t	get_dword_time();

    extern char *	time_str(time_t ct);

#define CREATE(result, type, number)  do { \
	if (!((result) = (type *) calloc ((number), sizeof(type)))) { \
		spdlog::error( "calloc failed [{}] {}", errno, strerror(errno)); \
		abort(); } } while(0)

#define RECREATE(result,type,number) do { \
	if (!((result) = (type *) realloc ((result), sizeof(type) * (number)))) { \
		STORM_LOG(Error, "realloc failed [{}] {}", errno,                  \
			          strerror(errno)); \
		abort(); } } while(0)

    // Next 와 Prev 가 있는 리스트에 추가
#define INSERT_TO_TW_LIST(item, head, prev, next)   \
    if (!(head))                                    \
    {                                               \
	head         = item;                        \
	    (head)->prev = (head)->next = NULL;         \
    }                                               \
    else                                            \
    {                                               \
	(head)->prev = item;                        \
	    (item)->next = head;                    \
	    (item)->prev = NULL;                    \
	    head         = item;                    \
    }

#define REMOVE_FROM_TW_LIST(item, head, prev, next)	\
    if ((item) == (head))           			\
    {                               			\
	if (((head) = (item)->next))			\
	    (head)->prev = NULL;    			\
    }                    				\
    else                 				\
    {                    				\
	if ((item)->next)				\
	    (item)->next->prev = (item)->prev;		\
							\
	if ((item)->prev)				\
	    (item)->prev->next = (item)->next;		\
    }


#define INSERT_TO_LIST(item, head, next)            \
    (item)->next = (head);                      \
	(head) = (item);                            \

#define REMOVE_FROM_LIST(item, head, next)          \
	if ((item) == (head))                       \
	    head = (item)->next;                     \
	else                                        \
	{                                           \
	    temp = head;                            \
		\
		while (temp && (temp->next != (item)))  \
		    temp = temp->next;                  \
			\
			if (temp)                               \
			    temp->next = (item)->next;          \
	}                                           \

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                      \
                ((DWORD)(BYTE) (ch0       ) | ((DWORD)(BYTE) (ch1) <<  8) | \
                 ((DWORD)(BYTE) (ch2) << 16) | ((DWORD)(BYTE) (ch3) << 24))
#endif // defined(MAKEFOURCC)



// _countof for gcc/g++
#if !defined(_countof)
#if !defined(__cplusplus)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#else
extern "C++"
{
	    template <typename _CountofType, size_t _SizeOfArray>
			        char (*__countof_helper(_CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
#define _countof(_Array) sizeof(*__countof_helper(_Array))
}
#endif
#endif

#ifdef _WIN32
extern void gettimeofday(struct timeval* t, struct timezone* dummy);
#endif

extern void stl_lowers(std::string& rstRet);



template <typename RandomGenerator = std::default_random_engine>
struct random_selector
{
	//On most platforms, you probably want to use std::random_device("/dev/urandom")()
	random_selector(RandomGenerator g = RandomGenerator(std::random_device()()))
		: gen(g) {}

	template <typename Iter>
	Iter select(Iter start, Iter end) {
		std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
		std::advance(start, dis(gen));
		return start;
	}

	//convenience function
	template <typename Iter>
	Iter operator()(Iter start, Iter end) {
		return select(start, end);
	}

	//convenience function that works on anything with a sensible begin() and end(), and returns with a ref to the value type
	template <typename Container>
	auto operator()(const Container& c) -> decltype(*begin(c))& {
		return *select(begin(c), end(c));
	}

private:
	RandomGenerator gen;
};




inline unsigned stou(std::string const& str, size_t* idx = 0, int base = 10)
{
	unsigned long result = std::stoul(str, idx, base);
	if (result > std::numeric_limits<unsigned>::max()) {
		throw std::out_of_range("stou");
	}
	return result;
}

inline short stos(std::string const& str, size_t* idx = 0, int base = 10)
{
	int result = std::stoi(str, idx, base);
	if (result > std::numeric_limits<short>::max()) {
		throw std::out_of_range("stos");
	}
	return result;
}
inline unsigned short stous(std::string const& str, size_t* idx = 0,
                            int base = 10)
{
	int result = std::stoi(str, idx, base);
	if (result > std::numeric_limits<unsigned short>::max()) {
		throw std::out_of_range("stous");
	}
	return result;
}

inline unsigned char stouc(std::string const& str, size_t* idx = 0,
                           int base = 10)
{
	int result = std::stoi(str, idx, base);
	if (result > std::numeric_limits<unsigned char>::max()) {
		throw std::out_of_range("stouc");
	}
	return result;
}

inline char stoc(std::string const& str, size_t* idx = 0, int base = 10)
{
	int result = std::stoi(str, idx, base);
	if (result > std::numeric_limits<char>::max()) {
		throw std::out_of_range("stouc");
	}
	return result;
}
/*----- atoi function -----*/
inline bool str_to_number(bool& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	try {
		out = atoi(in) != 0;
	} catch (std::out_of_range& e) {
		out = 0;
		return false;
	}
	return true;
}

inline bool str_to_bool(bool& out, const std::string& in)
{
	out = in.at(0) == '1';
	return true;
}

inline bool str_to_number(char& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = atoi(in);
	return true;
}

inline bool str_to_number(unsigned char& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = 0;

	return storm::ParseNumber(in, out);
}

inline bool str_to_number(int8_t & out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = 0;

	return storm::ParseNumber(in, out);
}

inline bool str_to_number(short& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = 0;

	return storm::ParseNumber(in, out);
}

inline bool str_to_number(unsigned short& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = 0;

	return storm::ParseNumber(in, out);
}

inline bool str_to_number(int& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = 0;

	return storm::ParseNumber(in, out);
}

inline bool str_to_number(unsigned int& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = 0;

	return storm::ParseNumber(in, out);
}

inline bool str_to_number(long& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = 0;

	return storm::ParseNumber(in, out);
}

inline bool str_to_number(unsigned long& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = 0;

	return storm::ParseNumber(in, out);
}

namespace m2
{
template <typename T> T minmax(T min, T value, T max)
{
	T tv;

	tv = (min > value ? min : value);
	return (max < tv) ? max : tv;
}
}

inline bool str_to_number(unsigned long long& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	try {
		out = std::stoull(in);
	} catch (std::out_of_range& e) {
		out = 0;
		return false;
	}
	return true;
}

inline bool str_to_number(float& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	try {
		out = std::stof(in);
	} catch (std::out_of_range& e) {
		out = 0.0f;
		return false;
	}
	return true;
}

inline bool str_to_number(double& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	try {
		out = std::stod(in);
	} catch (std::out_of_range& e) {
		out = 0.0;
		return false;
	}
	return true;
}

inline bool str_to_number(long long& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	try {
		out = std::stoll(in);
	} catch (std::out_of_range& e) {
		out = 0;
		return false;
	}
	return true;
}

#ifdef __FreeBSD__
inline bool str_to_number(long double& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = (long double)strtold(in, NULL);
	return true;
}
#endif

//@source: http://stackoverflow.com/a/2845275/2205532
inline bool is_positive_number(const std::string& str)
{
	if (str.empty() ||
	    ((!isdigit(str[0])) && (str[0] != '-') && (str[0] != '+')))
		return false;

	char* p;
	strtol(str.c_str(), &p, 10);

	// Finally, check that the referenced pointer points to the end of the
	// string. If that happens, said string is a number.
	return (*p == 0);
}

BOOST_FORCEINLINE bool is_number(std::string_view s)
{
    return !s.empty() && std::find_if(s.begin(), 
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}
/*----- atoi function -----*/

inline bool isNumeric(const std::string& input)
{
	return std::all_of(input.begin(), input.end(), ::isdigit);
}

template <typename T> struct wrapped_array {
	wrapped_array(T* first, T* last)
	    : begin_{first}
	    , end_{last}
	{
	}
	wrapped_array(T* first, std::ptrdiff_t size)
	    : wrapped_array{first, first + size}
	{
	}

	T* begin() const { return begin_; }
	T* end() const { return end_; }

	T* begin_;
	T* end_;
};

template <typename T> wrapped_array<T> wrap_array(T* first, std::ptrdiff_t size)
{
	return {first, size};
}


#endif	// __INC_UTILS_H__

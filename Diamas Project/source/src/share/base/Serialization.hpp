#pragma once

// Adapted from: http://rodgert.github.io/2014/09/09/type-driven-wire-protocols-with-boost-fusion-pt1/

#include <storm/String.hpp>

#include <optional>
#include <boost/asio/buffer.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/range/has_range_iterator.hpp>
#include <boost/fusion/include/tag_of.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/adapted/array.hpp>
#include <boost/fusion/include/array.hpp>

#include <boost/core/demangle.hpp>
#include <cstddef>
#include <type_traits>
#include <bitset>
#include <stdexcept>
#include <unordered_map>
#include <map>


namespace asio = boost::asio;
namespace fusion = boost::fusion;

template <typename T, size_t N = CHAR_BIT * sizeof(T)>
struct OptionalFieldSet
{
	using ValueType = T;
	using BitsType = std::bitset<N>;
};


template <typename T, size_t N>
struct OptionalField : std::optional<T>
{
	BOOST_CONSTEXPR static const size_t bit = N;

	OptionalField() = default;
	OptionalField(T v)
		: std::optional<T>(std::move(v))
	{
	}
};

using OptionalFields = OptionalFieldSet<uint16_t>;

class BadMessage : public std::runtime_error
{
	public:
		BadMessage()
			: runtime_error("message is invalid")
		{
			// ctor
		}

		BadMessage(const std::string& what)
			: runtime_error(what.c_str())
		{
			// ctor
		}
};

class MessageBufferTooSmall : public std::runtime_error
{
	public:
		MessageBufferTooSmall()
			: runtime_error("Message buffer too small")
		{
			// ctor
		}
};

template <class T>
struct is_map : public std::false_type {};

template <class K, class T, class Comp, class Alloc>
struct is_map<std::map<K, T, Comp, Alloc>> : public std::true_type {};

template <class K, class T, class Hash, class Comp, class Alloc>
struct is_map<std::unordered_map<K, T, Hash, Comp, Alloc>> : public std::true_type {};

template <typename T>
using is_adapted_struct=std::is_same<typename boost::fusion::traits::tag_of<T>::type,boost::fusion::struct_tag>;

struct Reader
{
	mutable asio::const_buffer m_buf;
	mutable std::optional<OptionalFields::BitsType> m_opts;

	explicit Reader(asio::const_buffer buf)
		: m_buf(std::move(buf))
	{
	}

	asio::const_buffer& buffer()
	{ return m_buf; }

	template <class T>
	auto operator()(T& val) const ->
	    typename std::enable_if<std::is_arithmetic<T>::value>::type
	{
		if (asio::buffer_size(m_buf) < sizeof(T)) {
			throw BadMessage(fmt::format("Not enough size for arithmetic value type {} size {} buffer size {}", boost::core::demangle(typeid(T).name()),  sizeof(T), asio::buffer_size(m_buf)));
		}

		val = *asio::buffer_cast<T const*>(m_buf);
		m_buf = m_buf + sizeof(T);
	}

	template <class T>
	auto operator()(T& val) const ->
	    typename std::enable_if<std::is_enum<T>::value>::type
	{
		typename std::underlying_type<T>::type v;
		(*this)(v);
		val = static_cast<T>(v);
	}

	template <class T, T v>
	void operator()(std::integral_constant<T, v>) const
	{
		typedef std::integral_constant<T, v> type;
		typename type::value_type val;

		(*this)(val);

		if (val != type::value)
			throw BadMessage("Failed to verify integral constant");
	}

	void operator()(std::string& val) const
	{
		uint16_t length = 0;
		(*this)(length);

		if (asio::buffer_size(m_buf) < length)
			throw BadMessage("Not enough length for std::string");

		val.assign(asio::buffer_cast<char const*>(m_buf), length);
		m_buf = m_buf + length;
	}

	void operator()(storm::StringRef& val) const
	{
		uint16_t length = 0;
		(*this)(length);

		if (asio::buffer_size(m_buf) < length)
			throw BadMessage("Not enough length for storm::StringRef");

		val = storm::StringRef(asio::buffer_cast<char const*>(m_buf), length);
		m_buf = m_buf + length;
	}

	template <class T>
	void operator()(std::vector<T>& val) const
	{
		uint16_t length = 0;
		(*this)(length);
		val.reserve(length);
		for (uint16_t i = 0; i < length; i++) {
			T v;
			(*this)(v);
			val.emplace_back(v);
		}
	}
        template <typename T, size_t N>
	void operator()(std::array<T, N>& val) const
	{
		uint16_t length = 0;
		(*this)(length);
		for (uint16_t i = 0; i < length; i++) {
			T v;
			(*this)(v);
			val[i] = v;
		}
	}
	void operator()(OptionalFields&) const
	{
		OptionalFields::ValueType val;
		(*this)(val);
		m_opts = OptionalFields::BitsType(val);
	}

	template <class T, size_t N>
	void operator()(OptionalField<T, N>& val) const
	{
		if (!m_opts)
			throw BadMessage("No m_opts set");

		if ((*m_opts)[N]) {
			T v;
			(*this)(v);
			val = OptionalField<T, N>(std::move(v));
		}
	}

	template <class T>
	auto operator()(T& val) const ->
	    typename std::enable_if<fusion::traits::is_sequence<T>::value>::type
	{
		fusion::for_each(val, std::ref(*this));
	}

	template <typename T, size_t N>
	void operator()(T (&array) [N]) const
	{
		for (size_t i = 0; i < N; i++) {
			(*this)(array[i]);
		}
	}

#ifdef VSTD_PLATFORM_WINDOWS
	// Boost bug #11787 workaround
	template <typename T, size_t N>
	void operator()(const T (&array) [N]) const
	{
		for (size_t i = 0; i < N; i++) {
			T& val = const_cast<T&>(array[i]);
			(*this)(val);
		}
	}
#endif

	template<typename T, typename U>
	void operator() (std::pair<T, U>& val) const
	{
		(*this)(val.first);
		(*this)(val.second);
	}

	template<class T>
	auto operator() (T& val) const ->
		typename std::enable_if<is_map<T>::value>::type
	{
		uint16_t length;
		(*this)(length);
		for (uint16_t i = 0; i < length; i++) {
			// We cant use T::value_type here as it's std::pair<const key_type, mapped_type>
			std::pair<typename T::key_type, typename T::mapped_type> pair;
			(*this)(pair);
			val.insert(pair);
		}
	}

	template <class T>
	auto operator()(T& val) const ->
	    typename std::enable_if<!fusion::traits::is_sequence<T>::value &&
	        std::is_empty<T>::value>::type
	{
		// do nothing
	}
};

struct Writer
{
	mutable asio::mutable_buffer m_buf;
	mutable OptionalFields::BitsType m_opts;
	mutable OptionalFields::ValueType* m_optv;

	explicit Writer(asio::mutable_buffer buf)
		: m_buf(std::move(buf))
		, m_optv(nullptr)
	{
	}

	template <class T>
	auto operator()(T const& val) const ->
	    typename std::enable_if<std::is_arithmetic<T>::value>::type
	{
		if (asio::buffer_size(m_buf) < sizeof(T))
			throw MessageBufferTooSmall();

		asio::buffer_copy(m_buf, asio::buffer(&val, sizeof(T)));
		m_buf = m_buf + sizeof(T);
	}

	template <class T>
	auto operator()(T const& val) const ->
	    typename std::enable_if<std::is_enum<T>::value>::type
	{
		using utype = typename std::underlying_type<T>::type;
		(*this)(static_cast<utype>(val));
	}

	template <class T, T v>
	void operator()(std::integral_constant<T, v>) const
	{
		typedef std::integral_constant<T, v> type;
		(*this)(type::value);
	}

	void operator()(storm::String const& val) const
	{
		(*this)(static_cast<uint16_t>(val.size()));

		if (asio::buffer_size(m_buf) < val.size())
			throw MessageBufferTooSmall();

		asio::buffer_copy(m_buf, asio::buffer(val.data(), val.size()));
		m_buf = m_buf + val.size();
	}

	void operator()(storm::StringRef const& val) const
	{
		(*this)(static_cast<uint16_t>(val.size()));

		if (asio::buffer_size(m_buf) < val.size())
			throw MessageBufferTooSmall();

		asio::buffer_copy(m_buf, asio::buffer(val.data(), val.size()));
		m_buf = m_buf + val.size();
	}

	template <class T>
	void operator()(std::vector<T> const& vals) const
	{
		(*this)(static_cast<uint16_t>(vals.size()));

		for (auto&& val : vals)
			(*this)(val);
	}

	template <class T, size_t N>
	void operator()(std::array<T, N> const& vals) const
	{
		(*this)(static_cast<uint16_t>(vals.size()));

		for (auto&& val : vals)
			(*this)(val);
	}

	void operator()(OptionalFields) const
	{
		m_opts.reset();

		if (asio::buffer_size(m_buf) < sizeof(OptionalFields::ValueType))
			throw MessageBufferTooSmall();

		m_optv = asio::buffer_cast<OptionalFields::ValueType*>(m_buf);
		m_buf = m_buf + sizeof(OptionalFields::ValueType);
	}

	template <class T, size_t N>
	void operator()(OptionalField<T, N> const& val) const
	{
		if (!m_optv)
			throw BadMessage();

		if (val) {
			m_opts.set(N);
			*m_optv = static_cast<OptionalFields::ValueType>(m_opts.to_ulong());

			(*this)(*val);
		}
	}

	template <class T>
	auto operator()(T const& val) const -> typename std::enable_if<
			fusion::traits::is_sequence<T>::value>::type
	{
		fusion::for_each(val, std::ref(*this));
	}

	template <typename T, size_t N>
	void operator()(const T (&array) [N]) const
	{
		//static_assert(!is_adapted_struct<T>(), "T is not an fusion adapted structure");
		for (size_t i = 0; i < N; i++) {
			(*this)(array[i]);
		}
	}

	template<class T, class U>
	void operator()(std::pair<T, U> const& val) const
	{
		(*this)(val.first);
		(*this)(val.second);
	}

	// Maps and such
	template<typename T>
	auto operator()(T const& vals) const ->
		typename std::enable_if<boost::has_range_const_iterator<T>::value>::type
	{
		auto length = std::distance(std::begin(vals), std::end(vals));

		if (length > std::numeric_limits<uint16_t>::max())
			throw BadMessage("Length is too big to fit into length variable");

		(*this)(static_cast<uint16_t>(length));

		for (auto& val : vals)
			(*this)(val);
	}

	template <class T>
	auto operator()(T const& val) const ->
	    typename std::enable_if<!fusion::traits::is_sequence<T>::value &&
	        std::is_empty<T>::value>::type
	{
		// do nothing
	}
};

struct ObjectSizer
{
	std::size_t m_size = 0;

	template <class T>
	auto operator()(T const& val) ->
	    typename std::enable_if<std::is_arithmetic<T>::value>::type
	{
		m_size += sizeof(T);
	}

	template <class T>
	auto operator()(T const& val) ->
	    typename std::enable_if<std::is_enum<T>::value>::type
	{
		using utype = typename std::underlying_type<T>::type;
		(*this)(static_cast<utype>(val));
	}

	template <class T, T v>
	void operator()(std::integral_constant<T, v>)
	{
		typedef std::integral_constant<T, v> type;
		(*this)(type::value);
	}

	void operator()(storm::String const& val)
	{
		(*this)(static_cast<uint16_t>(val.size()));
		m_size += val.size();
	}

	void operator()(storm::StringRef const& val)
	{
		(*this)(static_cast<uint16_t>(val.size()));
		m_size += val.size();
	}

	template <class T>
	void operator()(std::vector<T> const& vals)
	{
		(*this)(static_cast<uint16_t>(vals.size()));

		for (auto&& val : vals)
			(*this)(val);
	}

	void operator()(OptionalFields)
	{
		m_size += sizeof(OptionalFields::ValueType);
	}

	template <class T, size_t N>
	void operator()(OptionalField<T, N> const& val)
	{
		if (val)
			(*this)(*val);
	}

	template <class T>
	auto operator()(T const& val) -> typename std::enable_if<
			fusion::traits::is_sequence<T>::value>::type
	{
		fusion::for_each(val, std::ref(*this));
	}

	template <typename T, size_t N>
	void operator()(const T (&array) [N])
	{
		for (size_t i = 0; i < N; i++) {
			(*this)(array[i]);
		}
	}

	template<class T, class U>
	void operator()(std::pair<T, U> const& val)
	{
		(*this)(val.first);
		(*this)(val.second);
	}

	// Maps and such
	template<typename T>
	auto operator()(T const& vals) ->
		typename std::enable_if<boost::has_range_const_iterator<T>::value>::type
	{
		uint16_t length = 0;
		(*this)(length);

		for (auto& val : vals)
			(*this)(val);
	}

	template <class T>
	auto operator()(T const& val) ->
	    typename std::enable_if<!fusion::traits::is_sequence<T>::value &&
	        std::is_empty<T>::value>::type
	{
		// do nothing
	}
};

struct BufferSizer
{
	mutable asio::const_buffer m_buf;
	mutable std::optional<OptionalFields::BitsType> m_opts;

	explicit BufferSizer(asio::const_buffer buf)
		: m_buf(buf)
	{
	}

	uint16_t ReadLength() const
	{
		Reader r(m_buf);
		uint16_t res;
		r(res);
		m_buf = r.m_buf;
		return res;
	}

	template <class T>
	auto operator()(T const&) const ->
		typename std::enable_if<std::is_arithmetic<T>::value>::type
	{
		m_buf = m_buf + sizeof(T);
	}

	template <class T>
	auto operator()(T const&) const ->
	    typename std::enable_if<std::is_enum<T>::value>::type
	{
		typename std::underlying_type<T>::type v;
		(*this)(v);
	}

	template <class T, T v>
	void operator()(std::integral_constant<T, v>) const
	{
		typedef std::integral_constant<T, v> type;
		typename type::value_type val;
		(*this)(val);
	}

	void operator()(storm::String const&) const
	{
		const auto len = ReadLength();
		m_buf = m_buf + len;
	}

	void operator()(storm::StringRef const&) const
	{
		const auto len = ReadLength();
		m_buf = m_buf + len;
	}

	template <class T>
	void operator()(std::vector<T> const&) const
	{
		uint16_t length = ReadLength();
		for ( ; length; --length) {
			T v;
			(*this)(v);
		}
	}

        template <typename T, size_t N>
	void operator()(std::array<T, N> const&) const
	{
		uint16_t length = ReadLength();
		for ( ; length; --length) {
			T v;
			(*this)(v);
		}
	}

	void operator()(OptionalFields const&) const
	{
		Reader r(m_buf);
		OptionalFields::ValueType val;
		r(val);
		m_opts = OptionalFields::BitsType(val);
		m_buf = r.m_buf;
	}

	template <class T, size_t N>
	void operator()(OptionalField<T, N> const&) const
	{
		if (!m_opts)
			throw BadMessage();

		if ((*m_opts)[N]) {
			T v;
			(*this)(v);
		}
	}

	template <class T>
	auto operator()(const T& val) const -> typename std::enable_if<
	    fusion::traits::is_sequence<T>::value>::type
	{
		fusion::for_each(val, std::ref(*this));
	}

	template <typename T, size_t N>
	void operator()(T (&array) [N]) const
	{
		for (size_t i = 0; i < N; i++) {
			(*this)(array[i]);
		}
	}


#ifdef VSTD_PLATFORM_WINDOWS
	// Boost bug #11787 workaround
	template <typename T, size_t N>
	void operator()(const T (&array) [N]) const
	{
		for (size_t i = 0; i < N; i++) {
			T& val = const_cast<T&>(array[i]);
			(*this)(val);
		}
	}
#endif

	template<typename T, typename U>
	void operator() (std::pair<T, U>& val) const
	{
		(*this)(val.first);
		(*this)(val.second);
	}

	template<class T>
	auto operator() (T& val) const ->
	typename std::enable_if<is_map<T>::value>::type
	{
		uint16_t length;
		(*this)(length);

		for (; length > 0; length--) {
			// We cant use T::value_type here as it's std::pair<const key_type, mapped_type>
			std::pair<typename T::key_type, typename T::mapped_type> pair;
			(*this)(pair);
			val.insert(pair);
		}
	}

	template <class T>
	auto operator()(T& val) const ->
	    typename std::enable_if<!fusion::traits::is_sequence<T>::value &&
	        std::is_empty<T>::value>::type
	{
		// do nothing
	}
};

template <typename T>
std::pair<T, asio::const_buffer> Read(asio::const_buffer b)
{
	Reader r(std::move(b));
	T res;
	r(res);
	return std::make_pair(std::move(res), r.m_buf);
}

template <typename T>
asio::mutable_buffer Write(asio::mutable_buffer b, const T& val)
{
	Writer w(std::move(b));
	w(val);
	return w.m_buf;
}

template <typename T>
std::size_t GetObjectSize(const T& val)
{
	ObjectSizer s;
	s(val);
	return s.m_size;
}

template <class T>
std::size_t GetBufferSize(asio::const_buffer buf)
{
	BufferSizer s(buf);
	T v;
	s(v);
	return asio::buffer_size(buf) - asio::buffer_size(s.m_buf);
}


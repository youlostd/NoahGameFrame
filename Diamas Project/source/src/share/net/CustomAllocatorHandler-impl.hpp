#ifndef METIN2_NET_CUSTOMALLOCATORHANDLER_IMPL_HPP
#define METIN2_NET_CUSTOMALLOCATORHANDLER_IMPL_HPP

#ifdef BOOST_NO_CXX11_VARIADIC_TEMPLATES
	#include <boost/preprocessor/repetition/repeat.hpp>
#endif

#include <utility>



template <uint32_t Size>
HandlerAllocator<Size>::HandlerAllocator(const storm::Allocator& super)
	: m_super(super)
	, m_inUse(false)
{
	// ctor
}

template <uint32_t Size>
void* HandlerAllocator<Size>::Allocate(std::size_t size)
{
	if (!m_inUse && size < m_storage.size) {
		m_inUse = true;
		return m_storage.address();
	} else {
		return m_super.Allocate(size);
	}
}

template <uint32_t Size>
void HandlerAllocator<Size>::Deallocate(void* pointer)
{
	if (pointer == m_storage.address()) {
		STORM_ASSERT(m_inUse, "Not allocated");
		m_inUse = false;
	} else {
		m_super.Deallocate(pointer);
	}
}

template <uint32_t Size, typename Handler>
CustomAllocatorHandler<Size, Handler>::CustomAllocatorHandler(
		HandlerAllocator<Size>& allocator,
		const Handler& handler)
	: m_allocator(allocator)
	, m_handler(handler)
{
	// ctor
}

#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES

template <uint32_t Size, typename Handler>
template <typename... Args>
void CustomAllocatorHandler<Size, Handler>::operator()(Args&&... args)
{
	m_handler(std::forward<Args>(args)...);
}

#else

#define METIN2_FORMAT_ARGUMENT(z, n, unused) \
	BOOST_PP_COMMA_IF(n) std::forward<A ## n>(arg ## n)

template <uint32_t Size, typename Handler>
void CustomAllocatorHandler<Size, Handler>::operator()()
{
	m_handler();
}

#define BOOST_PP_LOCAL_MACRO(n) \
	template <uint32_t Size, typename Handler> \
	template <BOOST_PP_ENUM_PARAMS(n, typename A)> \
	void CustomAllocatorHandler<Size, Handler>:: \
		operator()(BOOST_PP_ENUM_BINARY_PARAMS(n, A, &&arg)) \
	{ \
		m_handler(BOOST_PP_REPEAT(n, METIN2_FORMAT_ARGUMENT, 0)); \
	}

#define BOOST_PP_LOCAL_LIMITS (1, 5)
#include BOOST_PP_LOCAL_ITERATE()

#undef METIN2_FORMAT_ARGUMENT
#undef BOOST_PP_LOCAL_MACRO
#undef BOOST_PP_LOCAL_LIMITS

#endif

template <uint32_t Size, typename Handler>
BOOST_FORCEINLINE CustomAllocatorHandler<Size, Handler>
	MakeCustomAllocatorHandler(HandlerAllocator<Size>& a, const Handler& h)
{
	return CustomAllocatorHandler<Size, Handler>(a, h);
}



#endif

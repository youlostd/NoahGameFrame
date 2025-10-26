#ifndef METIN2_NET_CUSTOMALLOCATORHANDLER_HPP
#define METIN2_NET_CUSTOMALLOCATORHANDLER_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/memory/Allocator.hpp>

#include <boost/noncopyable.hpp>
#include <boost/aligned_storage.hpp>

#ifdef BOOST_NO_CXX11_VARIADIC_TEMPLATES
	#include <boost/preprocessor/repetition/enum_binary_params.hpp>
	#include <boost/preprocessor/repetition/enum_params.hpp>
	#include <boost/preprocessor/iteration/local.hpp>
#endif



template <uint32_t Size>
class HandlerAllocator : private boost::noncopyable
{
	public:
		HandlerAllocator(const storm::Allocator& super);

		void* Allocate(std::size_t size);
		void Deallocate(void* pointer);

	private:
		storm::Allocator m_super;
		boost::aligned_storage<Size> m_storage;
		bool m_inUse;
};

template <uint32_t Size, typename Handler>
class CustomAllocatorHandler
{
	public:
		friend void* asio_handler_allocate(std::size_t size,
		                                   CustomAllocatorHandler<Size, Handler>* self)
		{
			return self->m_allocator.Allocate(size);
		}

		friend void asio_handler_deallocate(void* pointer,
		                                    std::size_t /*size*/,
		                                    CustomAllocatorHandler<Size, Handler>* self)
		{
			self->m_allocator.Deallocate(pointer);
		}

		CustomAllocatorHandler(HandlerAllocator<Size>& allocator,
		                       const Handler& handler);

#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES
		template <typename... Args>
		void operator()(Args&&... args);
#else
		void operator()();

#define BOOST_PP_LOCAL_MACRO(n) \
		template <BOOST_PP_ENUM_PARAMS(n, typename A)> \
		void operator()(BOOST_PP_ENUM_BINARY_PARAMS(n, A, &&arg));

#define BOOST_PP_LOCAL_LIMITS (1, 5)
#include BOOST_PP_LOCAL_ITERATE()

#undef BOOST_PP_LOCAL_MACRO
#undef BOOST_PP_LOCAL_LIMITS

#endif

	private:
		HandlerAllocator<Size>& m_allocator;
		Handler m_handler;
};

template <uint32_t Size, typename Handler>
BOOST_FORCEINLINE CustomAllocatorHandler<Size, Handler>
	MakeCustomAllocatorHandler(HandlerAllocator<Size>& a, const Handler& h);



#include <net/CustomAllocatorHandler-impl.hpp>

#endif

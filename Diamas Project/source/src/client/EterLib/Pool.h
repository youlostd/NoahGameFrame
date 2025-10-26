#pragma once

#include "../eterBase/Debug.h"
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <stack>
#include <tuple>
//#define DYNAMIC_POOL_STRICT

template<typename T>
class CDynamicPool
{
public:
	CDynamicPool()
	{
		m_uInitCapacity = 0;
		m_uUsedCapacity = 0;
	}

	void SetName(const char* c_szName)
	{
	}

	void Clear()
	{
		Destroy();
	}

	void Destroy()
	{
		// [tim] Not possible - pools are destroyed too late
		// std::for_each(m_kVct_pkData.begin(), m_kVct_pkData.end(), Delete);
		m_kVct_pkData.clear();
		m_kVct_pkFree.clear();
	}

	void Create(uint32_t uCapacity)
	{
		m_uInitCapacity = uCapacity;
		m_kVct_pkData.reserve(uCapacity);
		m_kVct_pkFree.reserve(uCapacity);
	}

	T* Alloc()
	{
		if (m_kVct_pkFree.empty())
		{
			auto* pkNewData = new T;
			m_kVct_pkData.push_back(pkNewData);
			++m_uUsedCapacity;
			return pkNewData;
		}

		T* pkFreeData = m_kVct_pkFree.back();
		m_kVct_pkFree.pop_back();
		return pkFreeData;
	}

	void Free(T* pkData)
	{
		m_kVct_pkFree.push_back(pkData);
	}

	void FreeAll()
	{
		m_kVct_pkFree = m_kVct_pkData;
	}

	uint32_t GetCapacity()
	{
		return m_kVct_pkData.size();
	}

protected:
	bool __IsValidData(T* pkData)
	{
		if (m_kVct_pkData.end() == std::find(m_kVct_pkData.begin(), m_kVct_pkData.end(), pkData))
			return false;
		return true;
	}

	bool __IsFreeData(T* pkData)
	{
		if (m_kVct_pkFree.end() == std::find(m_kVct_pkFree.begin(), m_kVct_pkFree.end(), pkData))
			return false;

		return true;
	}

	static void Delete(T* pkData)
	{
		delete pkData;
	}

protected:
	std::vector<T*> m_kVct_pkData;
	std::vector<T*> m_kVct_pkFree;

	uint32_t m_uInitCapacity;
	uint32_t m_uUsedCapacity;
};

template<typename T>
class CDynamicPoolEx
{
public:
	CDynamicPoolEx()
	{
		m_uInitCapacity = 0;
		m_uUsedCapacity = 0;
	}

	void Clear()
	{
		Destroy();
	}

	void Destroy()
	{
		// [tim] Not possible - pools are destroyed too late
		// std::for_each(m_kVct_pkData.begin(), m_kVct_pkData.end(), Delete);
		m_kVct_pkData.clear();
		m_kVct_pkFree.clear();
	}

	void Create(uint32_t uCapacity)
	{
		m_uInitCapacity = uCapacity;
		m_kVct_pkData.reserve(uCapacity);
		m_kVct_pkFree.reserve(uCapacity);
	}

	T* Alloc()
	{
		if (m_kVct_pkFree.empty())
		{
			T* pkNewData = New();
			m_kVct_pkData.push_back(pkNewData);
			++m_uUsedCapacity;
			return pkNewData;
		}

		T* pkFreeData = m_kVct_pkFree.back();
		m_kVct_pkFree.pop_back();
		return pkFreeData;
	}

	void Free(T* pkData)
	{
		m_kVct_pkFree.push_back(pkData);
	}

	void FreeAll()
	{
		m_kVct_pkFree = m_kVct_pkData;
	}

	uint32_t GetCapacity()
	{
		return m_kVct_pkData.size();
	}

protected:
	bool __IsValidData(T* pkData)
	{
		if (m_kVct_pkData.end() == std::find(m_kVct_pkData.begin(), m_kVct_pkData.end(), pkData))
			return false;
		return true;
	}

	bool __IsFreeData(T* pkData)
	{
		if (m_kVct_pkFree.end() == std::find(m_kVct_pkFree.begin(), m_kVct_pkFree.end(), pkData))
			return false;

		return true;
	}

	static T* New()
	{
		return (T*)::operator new(sizeof(T));
	}

	static void Delete(T* pkData)
	{
		::operator delete(pkData);
	}

protected:
	std::vector<T*> m_kVct_pkData;
	std::vector<T*> m_kVct_pkFree;

	uint32_t m_uInitCapacity;
	uint32_t m_uUsedCapacity;
};

template <class T>
class  CPooledObject
{
public:
	CPooledObject()
	{
	}
	virtual ~CPooledObject()
	{
	}

	void * operator new(std::size_t /*mem_size*/)
	{
		return ms_kPool.Alloc();
	}

	void operator delete(void* pT)
	{
		ms_kPool.Free((T*)pT);
	}


	static void DestroySystem()
	{
		ms_kPool.Destroy();
	}

	static void DeleteAll()
	{
		ms_kPool.FreeAll();
	}

protected:
	static CDynamicPoolEx<T> ms_kPool;
};

template <class T>
CDynamicPoolEx<T> CPooledObject<T>::ms_kPool;

// Base class for all poolable objects
class PoolableObject {
public:
	virtual ~PoolableObject() = default;
	virtual void reset() = 0;

protected:
	PoolableObject() = default;
};


template <typename T, typename... Args> class ObjectPool {
public:
	template <typename P> using pointer_type = std::unique_ptr<P, std::function<void(P*)>>;

	ObjectPool(std::size_t init_size = 0, std::size_t max_size = 10, Args&&... args)
		: _max_size{ max_size }, _available{ max_size }, _size{ 0 }, _args{ args... } {
		static_assert(std::is_base_of<PoolableObject, T>::value, "Must be poolable object");
		initialize(init_size);
	}

	pointer_type<T> get() {
		if (_pool.empty()) {
			if (_available == 0) {
				return nullptr;
			}
			add();
		}
		--_available;
		auto inst = std::move(_pool.top());
		_pool.pop();
		return std::move(inst);
	}

	std::size_t free() { return _available; }
	std::size_t max_size() { return _max_size; }
	std::size_t size() { return _size; }
	bool empty() { return _pool.empty(); }

private:
	// Adds a new object to the pool
	void add(T* ptr = nullptr) {
		if (ptr == nullptr) {
			ptr = create_with_params(std::index_sequence_for<Args...>());
			++_size;
		}
		else {
			ptr->reset();
			++_available;
		}

		pointer_type<T> inst(ptr, [this](T* ptr) {
			// This is the custom deleter of the unique_ptr.
			// When the object is deleted in the callers context, it will be
			// returned back to the pool by utilizing the add function
			add(ptr);
		});

		_pool.push(std::move(inst));
	}

	template <std::size_t... Is> T* create_with_params(const std::index_sequence<Is...>&) {
		return new T(std::get<Is>(_args)...);
	}

	// Initializes the pool
	void initialize(std::size_t init_size) {
		for (std::size_t i = 0; i < init_size; ++i) {
			add();
		}
	}

	std::size_t _max_size;
	std::size_t _available;
	std::size_t _size;
	std::stack<pointer_type<T>> _pool;
	std::tuple<Args...> _args;
};
#ifndef METIN2_SERVER_DB_CACHE_H
#define METIN2_SERVER_DB_CACHE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif



#include <chrono>
#include <unordered_set>
#include <unordered_map>
#include <game/DbPackets.hpp>
#include "old_cache.h"

#include <game/DungeonInfoTypes.hpp>

typedef struct SOfflineShopItem
{
	uint32_t	dwID;
        uint8_t		locked;
	TOfflineShopItemData	data;
} TOfflineShopItem;


namespace chrono = std::chrono;

template <class T>
class CachedObject
{
public:
	CachedObject(chrono::milliseconds expireTime);

	void Touch()
	{
		m_lastUse = chrono::steady_clock::now();
	}

	void TryFlush()
	{
		if (m_dirty) {
			static_cast<T*>(this)->Flush();
			m_dirty = false;
		}
	}

	bool Update(chrono::steady_clock::time_point now)
	{
		TryFlush();

		if (!m_locked && now >= m_lastUse + m_timeout)
			return false;

		return true;
	}

	void Lock() { m_locked = true; }
	void Unlock() { m_locked = false; }
	bool IsLocked() const { return m_locked; }

protected:
	bool m_dirty = false;
	bool m_locked = false;
	chrono::steady_clock::time_point m_lastUse =
		chrono::steady_clock::now();
	chrono::milliseconds m_timeout;
};

template <class T>
CachedObject<T>::CachedObject(chrono::milliseconds expireTime)
	: m_timeout(expireTime)
{
	// ctor
}

template <class T>
class CacheManager
{
public:
	using Key = typename T::Key;

	T* Get(const Key& key);
	T& ForceGet(const Key& key);
	void Remove(const Key& key);

	void TryFlushAll();
	void RemoveAll();

	void Update(chrono::steady_clock::time_point now);

private:
	std::unordered_map<Key, T> m_cache{};
	int m_count{};
	
};

template <class T>
T* CacheManager<T>::Get(const Key& key)
{
	const auto it = m_cache.find(key);
	if (it != m_cache.end())
		return &it->second;

	return nullptr;
}

template <class T>
T& CacheManager<T>::ForceGet(const Key& key)
{
	return m_cache[key];
}

template <class T>
void CacheManager<T>::Remove(const Key& key)
{
	const auto it = m_cache.find(key);
	if (it != m_cache.end())
		m_cache.erase(it);
}

template <class T>
void CacheManager<T>::TryFlushAll()
{
	for (auto& p : m_cache)
		p.second.TryFlush();
}

template <class T>
void CacheManager<T>::RemoveAll()
{
	m_cache.clear();
}

template <class T>
void CacheManager<T>::Update(chrono::steady_clock::time_point now)
{
	auto i = 0;
	for (auto it = m_cache.begin(), end = m_cache.end(); it != end; ) {
		if (!it->second.Update(now))
			it = m_cache.erase(it);
		else
			++it;
		++i;
	}
}
class CachedItem : public CachedObject<CachedItem>
{
public:
	typedef uint32_t Key;

	CachedItem();
	~CachedItem();

	const TPlayerItem& GetData() const { return m_item; }
	void SetData(const TPlayerItem& item, bool setDirty = true);

	void Flush();

private:
	TPlayerItem m_item;
};

bool IsOwnedByPlayer(const TPlayerItem& item);
bool IsOwnedByPlayer(CachedItem* item);

class CachedPlayer : public CachedObject<CachedPlayer>
{
public:
	typedef uint32_t Key;

	CachedPlayer();
	~CachedPlayer();


	const TPlayerTable& GetTable() const { return m_tab; }
	void SetTable(const TPlayerTable& table, bool setDirty = true);

	const std::vector<CachedItem*>& GetItems() const { return m_items; }
	void SetItems(std::vector<CachedItem*> items, bool setDirty = true);

	void RemoveItem(CachedItem* item);
	void AddItem(CachedItem* item);

	void Flush();

private:
	void FlushSkills();
	void FlushTitle();
	void FlushQuickslots();

	TPlayerTable m_tab;

	std::unordered_set<uint32_t> m_dirtySkills;
	std::unordered_set<uint32_t> m_dirtyQuickslots;
	bool m_dirtyTitle = false;

	/// All items currently held by this character only.
	/// This excludes shared items (e.g. MALL/SAFEBOX)
	std::vector<CachedItem*> m_items;
};


#ifdef __OFFLINE_SHOP__

class COfflineShopCache : public cache<TOfflineShopData>
{
public:
	COfflineShopCache();
	~COfflineShopCache();

	void OnFlush();

	bool ShopWereDeleted();

private:
	bool	m_bShopWereDeleted;
};

class COfflineShopItemCache : public cache<TOfflineShopItem>
{
public:
	COfflineShopItemCache();
	~COfflineShopItemCache();

	void Put(TOfflineShopItem* pkNew, uint32_t dwOwnerPID, bool bSkipQuery = false);
	void OnFlush();

	bool ItemWereDeleted();

private:
	uint32_t	m_dwOwnerPID;
	bool	m_bItemWereDeleted;
};

#endif

class CDungeonInfoCache : public cache<TPlayerDungeonInfo>
{
    public:
	CDungeonInfoCache();
	virtual ~CDungeonInfoCache();

	virtual void OnFlush();
};

#endif /* METIN2_SERVER_DB_CACHE_H */

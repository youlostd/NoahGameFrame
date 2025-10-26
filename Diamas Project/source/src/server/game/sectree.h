#ifndef METIN2_SERVER_GAME_SECTREE_H
#define METIN2_SERVER_GAME_SECTREE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <list>
#include <base/robin_hood.h>

#include "entity.h"

enum ESectree
{
    SECTREE_SIZE = 6400,
    SECTREE_HALF_SIZE = 3200,
    CELL_SIZE = 50
};

typedef struct sectree_coord
{
    unsigned x : 16;
    unsigned y : 16;
} SECTREE_COORD;

typedef union sectreeid
{
    uint32_t package;
    SECTREE_COORD coord;
} SECTREEID;

enum
{
    ATTR_BLOCK = (1 << 0),
    ATTR_WATER = (1 << 1),
    ATTR_BANPK = (1 << 2),
    ATTR_OBJECT = (1 << 7),
};

struct FCollectEntity
{
    void operator()(CEntity *entity)
    {
        result.push_back(entity);
    }

    template <typename F>
    void ForEach(F &f)
    {
        for(auto* entity : result)
        {
            f(entity);
        }
    }

    typedef std::vector<CEntity *> ListType;
    ListType result; // list collected
};

class CAttribute;

class SECTREE
{
public:
    friend class SECTREE_MANAGER;
    friend class SECTREE_MAP;

    template <class _Func>
    CEntity *find_if(_Func &func) const
    {
        auto it_tree = m_neighbor_list.begin();

        while (it_tree != m_neighbor_list.end())
        {
            auto it_entity = (*it_tree)->m_set_entity.begin();

            while (it_entity != (*it_tree)->m_set_entity.end())
            {
                if (func(*it_entity))
                    return (*it_entity);

                ++it_entity;
            }

            ++it_tree;
        }

        return nullptr;
    }

    template <class _Func>
    inline void ForEachAround(_Func &func)
    {
        for(const auto& neighbor : m_neighbor_list)
            neighbor->for_each_entity(func);
    }

    template <class _Func>
    void for_each_for_find_victim(_Func &func)
    {
        auto it_tree = m_neighbor_list.begin();

        while (it_tree != m_neighbor_list.end())
        {
            //첫번째를 찾으면 바로 리턴
            if ((*(it_tree++))->for_each_entity_for_find_victim(func))
                return;
        }
    }

    template <class _Func>
    bool for_each_entity_for_find_victim(_Func &func)
    {
        auto it = m_set_entity.begin();

        while (it != m_set_entity.end())
        {
            //정상적으로 찾으면 바로 리턴
            if (func(*it++))
                return true;
        }
        return false;
    }

public:
    SECTREE();
    ~SECTREE();

    void Initialize();
    void Destroy();

    SECTREEID GetID();

    bool InsertEntity(CEntity *ent);
    void RemoveEntity(CEntity *ent);

    bool Regen();

    void IncreasePC();
    void DecreasePC();

    void BindAttribute(CAttribute *pkAttribute);

    CAttribute *GetAttributePtr() { return m_pkAttribute; }

    uint32_t GetAttribute(long x, long y);
    bool IsAttr(long x, long y, uint32_t dwFlag);

    void CloneAttribute(SECTREE *tree); // private map 처리시 사용

    int GetEventAttribute(long x, long y); // 20050313 현재는 사용하지 않음

    void SetAttribute(uint32_t x, uint32_t y, uint32_t dwAttr);
    void RemoveAttribute(uint32_t x, uint32_t y, uint32_t dwAttr);

private:
    template <class _Fn1 > inline
    void for_each_entity(_Fn1 &func)
    {
        auto _First = m_set_entity.begin();
        for (; _First != m_set_entity.end(); ++_First)
		    func(*_First);
	}

    SECTREEID m_id;
    robin_hood::unordered_set<CEntity *> m_set_entity;
    std::list<SECTREE *> m_neighbor_list;
    int m_iPCCount;
    bool isClone;

    CAttribute *m_pkAttribute;
};

#endif /* METIN2_SERVER_GAME_SECTREE_H */

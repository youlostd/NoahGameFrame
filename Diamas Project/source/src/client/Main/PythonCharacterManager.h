#ifndef METIN2_CLIENT_MAIN_PYTHONCHARACTERMANAGER_H
#define METIN2_CLIENT_MAIN_PYTHONCHARACTERMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "InstanceBase.h"
#include "../GameLib/PhysicsObject.h"
#include <boost/pool/object_pool.hpp>

class CPythonCharacterManager : public CSingleton<CPythonCharacterManager>, public IObjectManager
{
public:
    // Character List
    typedef std::list<CInstanceBase *> TCharacterInstanceList;
    typedef std::map<uint32_t, CInstanceBase *> TCharacterInstanceMap;

    class CharacterIterator;

public:
    CPythonCharacterManager();

    virtual ~CPythonCharacterManager();

    void AdjustCollisionWithOtherObjects(CActorInstance *pInst) override;

    bool IsRegisteredVID(uint32_t dwVID);
    bool IsAliveVID(uint32_t dwVID);
    bool IsDeadVID(uint32_t dwVID);

    bool OLD_GetPickedInstanceVID(uint32_t *pdwPickedActorID) const;
    CInstanceBase *OLD_GetPickedInstancePtr() const;
    Vector2 &OLD_GetPickedInstPosReference();

    void InsertPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);
    void RemovePVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);
    void ChangeGVG(uint32_t dwSrcGuildID, uint32_t dwDstGuildID);

    void SetPVPTeam(DWORD dwVID, int iTeam);
    int GetPVPTeam(DWORD dwVID);
    bool HasPVPTeam(DWORD dwVID);

    void GetInfo(std::string *pstInfo) const;

    void ClearMainInstance();
    bool SetMainInstance(uint32_t dwVID);
    CInstanceBase *GetMainInstancePtr() const;

    void SetEmoticon(uint32_t dwVID, uint32_t eEmoticon);
    bool IsPossibleEmoticon(uint32_t dwVID);
    void ShowPointEffect(uint32_t dwVID, uint32_t ePoint);
    bool RegisterPointEffect(uint32_t ePoint, const char *c_szFileName);

    // System
    void Destroy();

    void DeleteAllInstances();

    void Update();
    void Deform();
    void Render();
    void RenderShadowMainInstance() const;
    void RenderShadowAllInstances();
    void RenderCollision();

    // Create/Delete Instance
    CInstanceBase *CreateInstance(const CInstanceBase::SCreateData &c_rkCreateData);
    CInstanceBase *RegisterInstance(uint32_t VirtualID);

    void DeleteInstance(uint32_t VirtualID);
    void DeleteInstance(CInstanceBase *ch);

    void DeleteInstanceByFade(uint32_t VirtualID);

    void DestroyAliveInstanceMap();
    void DestroyDeadInstanceList();

    CharacterIterator CharacterInstanceBegin()
    {
        return CharacterIterator(m_kAliveInstMap.begin());
    }

    CharacterIterator CharacterInstanceEnd()
    {
        return CharacterIterator(m_kAliveInstMap.end());
    }

    size_t GetAliveInstanceCount() const { return m_kAliveInstMap.size(); }

    // Access Instance
    void SelectInstance(uint32_t VirtualID);
    CInstanceBase *GetSelectedInstancePtr() const;

    CInstanceBase *GetInstancePtr(uint32_t VirtualID) const;
    CInstanceBase *GetInstancePtrByName(const char *name);

    // Pick		
    int PickAll();
    CInstanceBase *GetCloseInstance(CInstanceBase *pInstance);

    // Refresh TextTail
    void RefreshAllPCTextTail();
    void RefreshAllGuildMark();
    CInstanceBase *GetTabNextTargetPointer(CInstanceBase *pkInstMain);

    uint32_t GetTabNextTargetVectorIndex() const
    {
        return m_adwVectorIndexTabNextTarget;
    }

    void ResetTabNextTargetVectorIndex()
    {
        m_adwVectorIndexTabNextTarget = -1;
    }

protected:
    uint32_t m_adwVectorIndexTabNextTarget;
protected:
    void UpdateTransform();
    void UpdateDeleting();

    void __Initialize();

    void __DeleteBlendOutInstance(CInstanceBase *pkInstDel);

    void Pick();

    void __UpdateSortPickedActorList();
    void __UpdatePickedActorList();
    void __SortPickedActorList();

    void __RenderSortedAliveActorList();
    void __RenderSortedDeadActorList();

protected:
    CInstanceBase *m_pkInstMain = nullptr;
    CInstanceBase *m_pkInstPick = nullptr;
    CInstanceBase *m_pkInstBind = nullptr;
    Vector2 m_v2PickedInstProjPos;

    TCharacterInstanceMap m_kAliveInstMap;
    TCharacterInstanceList m_kDeadInstList;
    std::map<DWORD, int> m_map_pvpTeam;

    std::vector<CInstanceBase *> m_kVct_pkInstPicked;
    uint32_t m_adwPointEffect[POINT_MAX_NUM]{};

public:
    class CharacterIterator
    {
    public:
        CharacterIterator()
        {
        }

        CharacterIterator(const TCharacterInstanceMap::iterator &it)
            : m_it(it)
        {
        }

        CInstanceBase *operator *() const
        {
            return m_it->second;
        }

        CharacterIterator &operator ++()
        {
            ++m_it;
            return *this;
        }

        CharacterIterator operator ++(int)
        {
            auto new_it = *this;
            ++(*this);
            return new_it;
        }

        CharacterIterator &operator =(const CharacterIterator &rhs)
        {
            m_it = rhs.m_it;
            return (*this);
        }

        bool operator ==(const CharacterIterator &rhs) const
        {
            return m_it == rhs.m_it;
        }

        bool operator !=(const CharacterIterator &rhs) const
        {
            return m_it != rhs.m_it;
        }

    private:
        TCharacterInstanceMap::iterator m_it;
    };
};
#endif /* METIN2_CLIENT_MAIN_PYTHONCHARACTERMANAGER_H */

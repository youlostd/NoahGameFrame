#ifndef METIN2_CLIENT_GAMELIB_DUNGEONBLOCK_H
#define METIN2_CLIENT_GAMELIB_DUNGEONBLOCK_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../EterLib/ResourceManager.h"
#include "../EterLib/GrpObjectInstance.h"
#include "../EterGrnLib/ModelInstance.h"
#include "../EterGrnLib/Thing.h"

class CDungeonModelInstance;

class CDungeonBlock : public CGraphicObjectInstance
{
public:
    typedef std::vector<CDungeonModelInstance *> TModelInstanceContainer;

    enum
    {
        ID = THING_OBJECT
    };

    int GetType() const
    {
        return ID;
    }

public:
    CDungeonBlock();
    virtual ~CDungeonBlock();

    void Destroy();

    void BuildBoundingSphere();
    bool Load(const char *c_szFileName);
    void ReloadTexture(); //2014-06-27 map tool ?? ???? ?? ??

    bool Intersect(float *pfu, float *pfv, float *pft);
    void GetBoundBox(Vector3 *pv3Min, Vector3 *pv3Max);

    void Update();
    void Render();

    bool GetBoundingSphere(Vector3 &v3Center, float &fRadius);
    void OnUpdateCollisionData(const CStaticCollisionDataVector *pscdVector);
    void OnUpdateHeighInstance(CAttributeInstance *pAttributeInstance);
    bool OnGetObjectHeight(float fX, float fY, float *pfHeight);

    void OnRender()
    {
    }

    void OnBlendRender()
    {
    }

    void OnRenderToShadowMap(bool showPart)
    {
    }

    void OnRenderShadow();

    void OnRenderPCBlocker()
    {
    }

protected:
    void __Initialize();

protected:
    Vector3 m_v3Center = {};
    float m_fRadius = 0.0f;

    CGraphicThing::Ptr m_pThing = nullptr;
    TModelInstanceContainer m_ModelInstanceContainer;
};
#endif /* METIN2_CLIENT_GAMELIB_DUNGEONBLOCK_H */

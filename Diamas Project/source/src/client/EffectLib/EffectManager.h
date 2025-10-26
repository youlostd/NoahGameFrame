#ifndef METIN2_CLIENT_EFFECTLIB_EFFECTMANAGER_H
#define METIN2_CLIENT_EFFECTLIB_EFFECTMANAGER_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "EffectInstance.h"
#include <game/EffectConstants.hpp>

class CEffectManager : public CScreen, public CSingleton<CEffectManager>
{
public:
    enum EEffectType
    {
        EFFECT_TYPE_NONE = 0,
        EFFECT_TYPE_PARTICLE = 1,
        EFFECT_TYPE_ANIMATION_TEXTURE = 2,
        EFFECT_TYPE_MESH = 3,
        EFFECT_TYPE_SIMPLE_LIGHT = 4,

        EFFECT_TYPE_MAX_NUM = 4,
    };

    typedef std::map<uint32_t, CEffectData *> TEffectDataMap;
    typedef std::map<uint32_t, CEffectInstance *> TEffectInstanceMap;

public:
    CEffectManager();
    virtual ~CEffectManager();

    void Destroy();

    void UpdateSound();
    void Update();
    void Render();
    bool RegisterEffect(const char *filename,
                        uint32_t *id = nullptr,
                        bool overwrite = false);
    void GetInfo(std::string *pstInfo) const;

    bool IsAliveEffect(uint32_t dwInstanceIndex);

    // Register
    int CreateEffect(const std::string &filename);

    void DeleteAllInstances();

    // Usage
    int CreateEffect(uint32_t id, float scale = 1.0f, uint32_t effectKind = EFFECT_KIND_NONE);
    bool CreateEffectInstance(uint32_t index, uint32_t id);

    bool SelectEffectInstance(uint32_t dwInstanceIndex);
    bool DestroyEffectInstance(uint32_t dwInstanceIndex);
    void DeactiveEffectInstance(uint32_t dwInstanceIndex);

    void SetEffectTextures(uint32_t dwID, const std::vector<std::string> &textures);
    void SetEffectInstancePosition(const Vector3 &c_rv3Position);
    void SetEffectInstanceRotation(const Vector3 &c_rv3Rotation);

    void ShowEffect();
    void HideEffect();

    // Temporary function
    int GetEmptyIndex();
    bool GetEffectData(uint32_t dwID, CEffectData **ppEffect);
    bool GetEffectData(uint32_t dwID, const CEffectData **c_ppEffect);

    // Area�� ���� ��� Effect�� �Լ�... EffectInstance�� Pointer�� ��ȯ�Ѵ�.
    // EffectManager ���� EffectInstanceMap�� �̿����� �ʴ´�.
    void CreateUnsafeEffectInstance(uint32_t dwEffectDataID, CEffectInstance **ppEffectInstance);
    bool DestroyUnsafeEffectInstance(CEffectInstance *pEffectInstance);
    CEffectInstance *GetEffectInstance(uint32_t index);

    int GetRenderingEffectCount();

protected:
    void __Initialize();

    void __DestroyEffectInstanceMap();
    void __DestroyEffectCacheMap();
    void __DestroyEffectDataMap();

protected:
    bool m_isDisableSortRendering;
    TEffectDataMap m_kEftDataMap;
    TEffectInstanceMap m_kEftInstMap;
    TEffectInstanceMap m_kEftCacheMap;

    CEffectInstance *m_pSelectedEffectInstance;
};
#endif /* METIN2_CLIENT_EFFECTLIB_EFFECTMANAGER_H */

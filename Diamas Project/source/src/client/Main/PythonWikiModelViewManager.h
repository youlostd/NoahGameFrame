#pragma once
#include <Config.hpp>

#include "InstanceBase.h"
#include "../EterPythonLib/PythonWindow.h"

class CPythonWikiModelView : public CScreen
{
public:
    CPythonWikiModelView(size_t addID);
    ~CPythonWikiModelView();


    void SetModel(DWORD vnum);
    void Drag(int32_t x, int32_t y);
    void SetModelZoom(bool zoom);
    void SetModelHair(DWORD vnum, bool isItem = true);
    void SetModelForm(DWORD vnum, bool isItem = true);
    void SetWeaponModel(DWORD vnum);
    void RenderModel();
    void DeformModel();
    void UpdateModel();

    void SetShow(bool bShow)
    {
        m_bShow = bShow;
    }

    void RegisterWindow(UI::CWindow *hWnd);

    void ClearInstance();

    const size_t GetID() const
    {
        return m_modulID;
    }

    void SetRotationStatus(int val)
    {
        m_enableRotation = val;
        m_modelRotation = 0.0f;
    };

private:
    CGraphicThingInstance *m_pWeaponModel;
    CInstanceBase *m_pModelInstance;
    DWORD m_dwHairNum;
    bool m_bShow;
    float m_modelRotation;
    float m_fov = 10.0f;
    float m_targetY = 0.0f;
    bool m_enableRotation = false;
    float m_zoom = 0.0f;
    float m_minZoom = 0.0f;
    float m_fModelZoomSpeed = 0.0f;
    float m_rotSpeed = 100.0f;
    float m_baseRotation = 0.0f;
    float m_modelHeight = 0.0f;
    size_t m_modulID;
    UI::CRenderTarget *m_pyWindow;
};

class CPythonWikiModelViewManager : public CSingleton<CPythonWikiModelViewManager>
{
public:
    CPythonWikiModelViewManager();
    virtual ~CPythonWikiModelViewManager();

    void AddView(DWORD addID);
    void RemoveView(DWORD addID);
    DWORD GetFreeID();
    CPythonWikiModelView *GetModule(DWORD moduleID);
    void DeformModel();
    void RenderModel();
    void UpdateModel();
    void ClearInstances();

    void SetShow(bool bShow)
    {
        m_bShow = bShow;
    }

private:
    typedef std::vector<CPythonWikiModelView*> TModelView;
    TModelView m_vecModelView;
    bool m_bShow;
};

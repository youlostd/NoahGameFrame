#ifndef METIN2_CLIENT_MAIN_PYTHONMINIMAP_H
#define METIN2_CLIENT_MAIN_PYTHONMINIMAP_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "PythonBackground.h"

class CPythonMiniMap : public CScreen, public CSingleton<CPythonMiniMap>
{
public:
#ifdef ENABLE_ADMIN_MANAGER
		enum
		{
			ADMIN_MANAGER_SHOW_OBSERVING_PLAYER = 1 << 0,
			ADMIN_MANAGER_SHOW_PC = 1 << 1,
			ADMIN_MANAGER_SHOW_MOB = 1 << 2,
			ADMIN_MANAGER_SHOW_STONE = 1 << 3,
			ADMIN_MANAGER_SHOW_NPC = 1 << 4,
			ADMIN_MANAGER_SHOW_ORE = 1 << 5,
		};
#endif
    enum
    {
        EMPIRE_NUM = 4,

        MINI_WAYPOINT_IMAGE_COUNT = 12,
        WAYPOINT_IMAGE_COUNT = 15,
        TARGET_MARK_IMAGE_COUNT = 2,
    };

    enum
    {
        TYPE_OPC,
        TYPE_OPCPVP,
        TYPE_OPCPVPSELF,
        TYPE_NPC,
        TYPE_MONSTER,
        TYPE_WARP,
        TYPE_WAYPOINT,
        TYPE_PARTY,
        TYPE_EMPIRE,
        TYPE_EMPIRE_END = TYPE_EMPIRE + EMPIRE_NUM,
        TYPE_TARGET,
        TYPE_SHOP,
        TYPE_BUFFBOT,
        TYPE_COUNT,
    };

public:
    CPythonMiniMap();
    virtual ~CPythonMiniMap();

    void Destroy();
    bool Create();

    bool IsAtlas();
    bool CanShow();
    bool CanShowAtlas();

    void SetMiniMapSize(float fWidth, float fHeight);
    void SetScale(float fScale);
    void ScaleUp();
    void ScaleDown();

    void SetCenterPosition(float fCenterX, float fCenterY);

    void Update(float fCenterX, float fCenterY);
    void Render(float fScreenX, float fScreenY);

    void Show();
    void Hide();

    bool GetPickedInstanceInfo(float fScreenX, float fScreenY, std::string &rReturnName, float *pReturnPosX,
                               float *pReturnPosY, uint32_t *pdwTextColor);

    // Atlas
    bool LoadAtlas();
    void UpdateAtlas();
    void RenderAtlas(float fScreenX, float fScreenY);
    void ShowAtlas();
    void HideAtlas();

#ifdef ENABLE_ADMIN_MANAGER
		bool LoadAdminManagerAtlas();
		void ConvertGlobalPositionToAdminManagerAtlasData(long lX, long lY, float& fRetX, float& fRetY);
		void ConvertPositionToAdminManagerAtlasData(long lX, long lY, float& fRetX, float& fRetY);
		void ConvertAdminManagerAtlasDataToPosition(float fX, float fY, float& lRetX, float& lRetY);
		void UpdateAdminManagerAtlas();
		void RenderAdminManagerAtlas(float fScreenX, float fScreenY, float fMaxWidth, float fMaxHeight);
		bool IsAdminManagerAtlasRotated() const;
		bool GetAdminManagerAtlasInfo(float fScreenX, float fScreenY, std::string & rReturnString, float * pReturnPosX, float * pReturnPosY, uint32_t * pdwTextColor, uint32_t * pdwStoneDropVnum);
		bool GetAdminManagerAtlasInfoNew(float fScreenX, float fScreenY, float * fReturnPosX, float * fReturnPosY);

		void ShowAdminManagerAtlas();
		void HideAdminManagerAtlas();
		bool CanShowAdminManagerAtlas() const;
		void ShowAdminManagerFlag(uint32_t dwFlag);
		void HideAdminManagerFlag(uint32_t dwFlag);
		bool IsAdminManagerFlagShown(uint32_t dwFlag) const;
#endif

    bool ToggleAtlasMarker(int type);

    bool GetAtlasInfo(float fScreenX, float fScreenY, std::string &rReturnString, float *pReturnPosX,
                      float *pReturnPosY, uint32_t *pdwTextColor, uint32_t *pdwGuildID);
    bool GetAtlasSize(float *pfSizeX, float *pfSizeY);

    void AddObserver(uint32_t dwVID, float fSrcX, float fSrcY);
    void MoveObserver(uint32_t dwVID, float fDstX, float fDstY);
    void RemoveObserver(uint32_t dwVID);

    void AddPartyMember(uint32_t dwPID, const char *szName);
    void MovePartyMember(uint32_t dwPID, long lX, long lY, float fRot);
    void RemovePartyMember(uint32_t dwPID);
    void ClearPartyMember();

    // WayPoint
    void AddWayPoint(uint8_t byType, uint32_t dwID, float fX, float fY, std::string strText, uint32_t dwChrVID = 0);
    void RemoveWayPoint(uint32_t dwID);

    // SignalPoint
    void AddSignalPoint(float fX, float fY);
    void ClearAllSignalPoint();

    void RegisterAtlasWindow(PyObject *poHandler);
    void UnregisterAtlasWindow();
    void OpenAtlasWindow();
    void SetAtlasCenterPosition(int x, int y);

    // NPC List
    void ClearAtlasMarkInfo();
    void ClearAtlasShopInfo();
    void RegisterAtlasMark(uint8_t byType, const char *c_szName, long lx, long ly);

    // Guild
    void ClearGuildArea();
    void RegisterGuildArea(uint32_t dwID, uint32_t dwGuildID, long x, long y, long width, long height);
    void UpdateGuildArea(uint32_t updateID, uint32_t updatedGuild);
    uint32_t GetGuildAreaID(uint32_t x, uint32_t y);

    // Target
    void CreateTarget(int iID, const char *c_szName);
    void CreateTarget(int iID, const char *c_szName, uint32_t dwVID);
    void UpdateTarget(int iID, int ix, int iy);
    void DeleteTarget(int iID);

protected:
    void __Initialize();
    void __SetPosition();
    void __LoadAtlasMarkInfo();

    void __RenderWayPointMark(int ixCenter, int iyCenter);
    void __RenderMiniWayPointMark(int ixCenter, int iyCenter);
    void __RenderTargetMark(int ixCenter, int iyCenter);

    void __GlobalPositionToAtlasPosition(long lx, long ly, float *pfx, float *pfy);

protected:
    // Atlas
    typedef struct
    {
        uint8_t m_byType;
        uint32_t m_dwID; // For WayPoint
        float m_fX;
        float m_fY;
        float m_fScreenX;
        float m_fScreenY;
        float m_fMiniMapX;
        float m_fMiniMapY;
        uint32_t m_dwChrVID;
        std::string m_strText;
    } TAtlasMarkInfo;

    // GuildArea
    typedef struct
    {
        uint32_t dwID;
        uint32_t dwGuildID;
        long lx, ly;
        long lwidth, lheight;

        float fsxRender, fsyRender;
        float fexRender, feyRender;
    } TGuildAreaInfo;

    struct SObserver
    {
        float fCurX;
        float fCurY;
        float fSrcX;
        float fSrcY;
        float fDstX;
        float fDstY;

        uint32_t dwSrcTime;
        uint32_t dwDstTime;
    };

    typedef struct
    {
        uint32_t pid;
        char szName[CHARACTER_NAME_MAX_LEN + 1];
        long lX;
        long lY;
        float fScreenX;
        float fScreenY;
        float fRotation;
    } TPartyMember;

    // ĳ���� ����Ʈ
    typedef struct
    {
        UINT m_bType;
        float m_fX;
        float m_fY;
        UINT m_eNameColor;
    } TMarkPosition;

    typedef std::vector<TMarkPosition> TInstanceMarkPositionVector;
    typedef TInstanceMarkPositionVector::iterator TInstancePositionVectorIterator;

protected:
    bool __GetWayPoint(uint32_t dwID, TAtlasMarkInfo **ppkInfo);
    void __UpdateWayPoint(TAtlasMarkInfo *pkInfo, int ix, int iy);

protected:
    float m_fWidth;
    float m_fHeight;

    float m_fScale;

    float m_fCenterX;
    float m_fCenterY;

    float m_fCenterCellX;
    float m_fCenterCellY;

    float m_fScreenX;
    float m_fScreenY;

    float m_fMiniMapRadius;

    // �� �׸�...
    LPDIRECT3DTEXTURE9 m_lpMiniMapTexture[AROUND_AREA_NUM];

    // �̴ϸ� Ŀ��
    CGraphicImageInstance m_MiniMapFilterGraphicImageInstance;
    CGraphicExpandedImageInstance m_MiniMapCameraraphicImageInstance;

    // ĳ���� ��ũ
    CGraphicExpandedImageInstance m_PlayerMark;
    CGraphicImageInstance m_WhiteMark;

    std::vector<TMarkPosition> m_MinimapPosVector;
    std::map<uint32_t, SObserver> m_kMap_dwVID_kObserver;
    std::map<uint32_t, TPartyMember> m_kMap_dwPID_kPartyMember;

    bool m_bAtlas;
#ifdef ENABLE_ADMIN_MANAGER
		bool							m_bAdminManagerAtlas;
#endif
    bool m_bShow;
#ifdef ENABLE_ADMIN_MANAGER
		bool							m_bShowAdminManagerAtlas;
		uint32_t							m_dwShowAdminManagerFlag;
#endif
    VertexBufferHandle m_VertexBuffer;
    IndexBufferHandle m_IndexBuffer;

    Matrix m_matIdentity;
    Matrix m_matWorld;
    Matrix m_matMiniMapCover;
#ifdef ENABLE_ADMIN_MANAGER
		CGraphicExpandedImageInstance	m_AdminManagerAtlasImageInstance;
		Matrix						m_matWorldAdminManagerAtlas;
		CGraphicExpandedImageInstance	m_AdminManagerAtlasPlayerMark;
		CGraphicExpandedImageInstance	m_AdminManagerAtlasOtherPlayerMark;
#endif
    bool m_bShowAtlas;
    bool m_bAtlasRenderShops;
    bool m_bAtlasRenderNpc;
    bool m_bAtlasRenderWarp;
    bool m_bAtlasRenderWaypoint;

    CGraphicImageInstance m_AtlasImageInstance;
    Matrix m_matWorldAtlas;
    CGraphicExpandedImageInstance m_AtlasPlayerMark;
    CGraphicExpandedImageInstance m_AtlasPartyPlayerMark;

    float m_fAtlasScreenX;
    float m_fAtlasScreenY;
#ifdef ENABLE_ADMIN_MANAGER
		float							m_fAdminManagerAtlasScreenX;
		float							m_fAdminManagerAtlasScreenY;
#endif

    float m_fAtlasMaxX;
    float m_fAtlasMaxY;
#ifdef ENABLE_ADMIN_MANAGER
		float							m_fAdminManagerAtlasMaxX;
		float							m_fAdminManagerAtlasMaxY;
#endif

    float m_fAtlasImageSizeX;
    float m_fAtlasImageSizeY;
#ifdef ENABLE_ADMIN_MANAGER
		float							m_fAdminManagerAtlasImageSizeX;
		float							m_fAdminManagerAtlasImageSizeY;
		float							m_fAdminManagerAtlasImageSizeScale;
#endif
    typedef std::vector<TAtlasMarkInfo> TAtlasMarkInfoVector;
    typedef TAtlasMarkInfoVector::iterator TAtlasMarkInfoVectorIterator;
    typedef std::vector<TGuildAreaInfo> TGuildAreaInfoVector;
    typedef TGuildAreaInfoVector::iterator TGuildAreaInfoVectorIterator;
    TAtlasMarkInfoVectorIterator m_AtlasMarkInfoListIterator;
    TAtlasMarkInfoVector m_AtlasNPCInfoVector;
    TAtlasMarkInfoVector m_AtlasShopInfoVector;
    TAtlasMarkInfoVector m_AtlasWarpInfoVector;

    // WayPoint
    CGraphicExpandedImageInstance m_MiniWayPointGraphicImageInstances[MINI_WAYPOINT_IMAGE_COUNT];
    CGraphicExpandedImageInstance m_WayPointGraphicImageInstances[WAYPOINT_IMAGE_COUNT];
    CGraphicExpandedImageInstance m_TargetMarkGraphicImageInstances[TARGET_MARK_IMAGE_COUNT];
    CGraphicImageInstance m_GuildAreaFlagImageInstance;
    TAtlasMarkInfoVector m_AtlasWayPointInfoVector;
    TGuildAreaInfoVector m_GuildAreaInfoVector;

    // SignalPoint
    struct TSignalPoint
    {
        Vector2 v2Pos;
        unsigned int id;
    };

    std::vector<TSignalPoint> m_SignalPointVector;

    PyObject *m_poHandler;
};
#endif /* METIN2_CLIENT_MAIN_PYTHONMINIMAP_H */

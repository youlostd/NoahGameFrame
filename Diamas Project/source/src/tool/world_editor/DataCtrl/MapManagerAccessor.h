#ifndef METIN2_TOOL_WORLDEDITOR_DATACTRL_MAPMANAGERACCESSOR_HPP
#define METIN2_TOOL_WORLDEDITOR_DATACTRL_MAPMANAGERACCESSOR_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif
#define WORLD_EDITOR

#include <GameLib/MapManager.h>

#include <vstl/string.hpp>

METIN2_BEGIN_NS

class CShadowRenderHelper;
class CPropertyTreeControler;
class CMapOutdoor;
class CMapOutdoorAccessor;
class CTerrainAccessor;

struct AreaObjectRef
{
	uint32_t area;
	int32_t index;

	operator bool() const { return -1 != index; }
};

class CMapManagerAccessor : public CMapManager
{
	public:
		class CHeightObserver
		{
			public:
				CHeightObserver(CMapOutdoor * pMapOutdoor) : m_pMap(pMapOutdoor) {}
				~CHeightObserver(){}

				float GetHeight(float fx, float fy);
				CMapOutdoor * m_pMap;
		};

	public:
		CMapManagerAccessor();
		virtual ~CMapManagerAccessor();

		void				Initialize();
		void				Destroy();

		void				DestroyShadowTexture();
		void				RecreateShadowTexture();

		virtual void		Clear();
		virtual CMapOutdoor*	AllocMap();
		virtual bool		LoadMap(const std::string & c_rstrMapName);

		void				UpdateEditing();

		void				ToggleWireframe();
		void				SetWireframe(bool isWireframe);

		// Cursor
		void				SetEditingCursorPosition(const D3DXVECTOR3 & c_rv3Position);

		////////////////////////////////////////////
		// About Brush
		// NOTE : Replace to the CSceneMap
		void				SetBrushShape(uint32_t dwShape);
		void				SetBrushType(uint32_t dwType);

		BYTE				GetBrushSize() { return m_byBrushSize; }
		void				SetBrushSize(BYTE ucBrushSize);
		void				SetMaxBrushSize(BYTE ucMaxBrushSize);
		BYTE				GetBrushStrength() { return m_byBrushStrength; }
		void				SetBrushStrength(BYTE ucBrushSize);
		void				SetMaxBrushStrength(BYTE ucMaxBrushStrength);
 		long				GetBrushWaterHeight() { return m_lBrushWaterHeight; }
 		void				SetBrushWaterHeight(long wBrushWaterHeight);

		void				SetTextureBrushVector(std::vector<BYTE> & rTextureBrushNumberVector);

		void				SetSelectedAttrFlag(BYTE bFlag);
		BYTE 				GetSelectedAttrFlag() { return m_bySelectedAttrFlag; }

		void				GetEditArea(int * iEditX,
										int * iEditY,
										unsigned char * usSubEditX,
										unsigned char * usSubEditY,
										WORD * usTerrainNumX,
										WORD * usTerrainNumY);

		uint32_t				GetBrushShape()							{ return m_dwBrushShape;		}

		void				EditingStart();
		void				EditingEnd();

		void				SetTerrainModified();

		void				SetHeightEditing(bool bOn)				{ m_bHeightEditingOn = bOn;		}
		void				SetTextureEditing(bool bOn)				{ m_bTextureEditingOn = bOn;	}
		void				SetWaterEditing(bool bOn)				{ m_bWaterEditingOn = bOn;		}
		void				SetAttrEditing(bool bOn)				{ m_bAttrEditingOn = bOn;		}
		bool			isHeightEditing()						{ return m_bHeightEditingOn;	}
		bool			isTextureEditing()						{ return m_bTextureEditingOn;	}
		bool			isWaterEditing()						{ return m_bWaterEditingOn;		}
		bool			isAttrEditing()							{ return m_bAttrEditingOn;		}
		////////////////////////////////////////////

		////////////////////////////////////////////
		// Texture Set
		bool	AddTerrainTexture(const char * pFilename);
		bool	RemoveTerrainTexture(long lTexNum);
		void	ResetTerrainTexture();
		bool	SaveTerrainTextureSet();
		// Texture Set
		////////////////////////////////////////////

		////////////////////////////////////////////
		// Base Texture
		void	SetInitTextureBrushVector(std::vector<BYTE> & rTextureBrushNumberVector);
		bool	InitBaseTexture(const char * c_szMapName = NULL);
		// Base Texture
		////////////////////////////////////////////

		////////////////////////////////////////////
		// About Object Picking & Control
		AreaObjectRef GetPickedObjectIndex();
		bool IsSelectedObject(const AreaObjectRef& ref);
		int GetSelectedObjectCount();
		void SelectObject(const AreaObjectRef& ref);
		void DeselectObject(const AreaObjectRef& ref);
		bool Picking();

		void RenderSelectedObject();
		void CancelSelect();
		bool SelectObject(float fxStart, float fyStart, float fxEnd, float fyEnd, SHORT sSpecialKeys = 0);
		bool DeselectObject(float fxStart, float fyStart, float fxEnd, float fyEnd);

		void DeleteSelectedObject();
		void MoveSelectedObject(float fx, float fy);
		void MoveSelectedObjectHeight(float fHeight);
		void AddSelectedAmbienceScale(int iAddScale);
		void AddSelectedAmbienceMaxVolumeAreaPercentage(float fPercentage);
		void AddSelectedObjectRotation(float fYaw, float fPitch, float fRoll);
		void SetSelectedObjectPortalNumber(int iID);
		void DelSelectedObjectPortalNumber(int iID);
		void CollectPortalNumber(std::set<int> * pkSet_iPortalNumber);
		void EnablePortal(bool bFlag);
		bool IsSelected();

		void InsertObject(float fx, float fy, float fz, int iRotation, uint32_t dwCRC);
		void InsertObject(float fx, float fy, float fz, float fYaw, float fPitch, float fRoll, uint32_t dwScale, uint32_t dwCRC);
		void RefreshObjectHeight(float fx, float fy, float fHalfSize);

		bool IsAddedSelectObject();
		void ClearAddedSelectObject();
		void ClearDeselectObject();

		bool GetObjectDataPointer(const AreaObjectRef& ref,
		                          const CArea::TObjectData ** ppObjectData) const;
		bool GetObjectInstancePointer(const AreaObjectRef& ref,
		                              const CArea::TObjectInstance ** ppObjectInstance) const;

		//외부에서 오브젝트의 Height 값을 알수있도록 인터페이스 제공을 위해 추가.
		bool GetObjectHeight(float fX, float fY, float* fHeight);


		// About Object Picking
		////////////////////////////////////////////

		////////////////////////////////////////////
		// For Undo System
		void BackupObject();
		void BackupObjectCurrent();
		void BackupTerrain();
		void BackupTerrainCurrent();
		////////////////////////////////////////////

		////////////////////////////////////////////
		bool SaveMapProperty(storm::String c_rstrFolder);		// 전체 맵 Property를 저장한다.
		bool SaveMapSetting(storm::String c_rstrFolder);		// 전체 맵 Setting을 저장한다.
		////////////////////////////////////////////

		////////////////////////////////////////////
		// 새로운 맴버들
		bool NewMap(const std::string& name, uint32_t w, uint32_t h,
		            const std::string& textureSet);
		bool SaveMap(const char * c_szMapName = NULL);				// 전체 맵 관련 데이타를 저장한다.
		bool SaveTerrains();										// 현재 메모리에 있는 최대 9개 Terrain 전체를 저장한다.
		bool SaveAreas();											// 현재 메모리에 있는 최대 9개 Area 전체를 저장한다.

		bool GetEditArea(CAreaAccessor ** ppAreaAccessor);
		bool GetArea(uint8_t c_ucAreaNum, CAreaAccessor ** ppAreaAccessor);

		bool GetEditTerrain(CTerrainAccessor ** ppTerrainAccessor);
		bool GetTerrain(uint8_t c_ucTerrainNum, CTerrainAccessor ** ppTerrainAccessor);

		const BYTE GetTerrainNum(float fx, float fy);
		const BYTE GetEditTerrainNum();

		void RefreshArea();
		////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// Attr
		void SetEraseAttr(const bool & bErase) { m_bEraseAttr = bErase;}
		void RenderAttr();
		bool ResetToDefaultAttr();
		void RenderObjectCollision();
		// Attr
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// Water
		void SetEraseWater(const bool & bErase) { m_bEraseWater = bErase;}
		void PreviewEditWater();
		void CurCellEditWater();	//현재 cell 의 height 값을 알수 있도록 하기위해 추가.
		// Water
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// Texture
		void SetEraseTexture(const bool & bErase) { m_bEraseTexture = bErase;}
		void SetDrawOnlyOnBlankTile(const bool & bOn) { m_bDrawOnlyOnBlankTile = bOn;}
		// Texture
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// Shadow
		void UpdateTerrainShadowMap();
		void ReloadTerrainShadowTexture(bool bAllArea = true);
		// Shadow
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// MiniMap
		void SaveMiniMap();
		void SaveAtlas();
		// MiniMap
		//////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////
		// ETC
		void LoadProperty(CPropertyTreeControler * pTreeControler);

		void UpdateHeightFieldEditingPt(D3DXVECTOR3 * v3IntersectPt);
		CHeightObserver * GetHeightObserverPointer();

		void RenderGuildArea();
		void UpdateMapInfo();
		CMapOutdoorAccessor * GetMapOutdoorPtr();
		////////////////////////////////////////////

	protected:
		void EditTerrain();
		void EditTerrainTexture();
		void EditWater();
		void EditAttr();
		void __RefreshMapID(const char * c_szMapName);

	protected:
		uint32_t m_dwBrushShape;
		uint32_t m_dwBrushType;

		BYTE m_byBrushSize;
		BYTE m_byBrushStrength;
		long m_lBrushWaterHeight;

		BYTE m_byMAXBrushSize;
		BYTE m_byMAXBrushStrength;

		std::vector<BYTE> m_TextureBrushNumberVector;
		std::vector<BYTE> m_InitTextureBrushNumberVector;

		int m_ixEdit;
		int m_iyEdit;

		BYTE m_bySubCellX;
		BYTE m_bySubCellY;
		WORD m_wEditTerrainNumX;
		WORD m_wEditTerrainNumY;

		bool m_bEditingInProgress;
		bool m_bHeightEditingOn;
		bool m_bTextureEditingOn;
		bool m_bWaterEditingOn;
		bool m_bAttrEditingOn;

		// For storing pointers...
		CMapOutdoorAccessor *	m_pMapAccessor;
		CHeightObserver *		m_pHeightObserver;

		WORD					m_wOldEditX, m_wOldEditY;

		// Attr
		BYTE					m_bySelectedAttrFlag;
		bool					m_bEraseAttr;

		// Water
		bool					m_bEraseWater;

		// Texture
		bool					m_bEraseTexture;
		bool					m_bDrawOnlyOnBlankTile;

		// CursorPosition
		D3DXVECTOR3				m_v3EditingCursorPosition;

		std::map<std::string, int>				m_kMap_strMapName_iID;
};

METIN2_END_NS

#endif

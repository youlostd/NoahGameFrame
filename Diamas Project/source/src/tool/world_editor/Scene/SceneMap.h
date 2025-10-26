#ifndef METIN2_TOOL_WORLDEDITOR_SCENE_SCENEMAP_HPP
#define METIN2_TOOL_WORLDEDITOR_SCENE_SCENEMAP_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "SceneMapCursor.h"

#include <gamelib/SnowEnvironment.h>

METIN2_BEGIN_NS

class CMapManagerAccessor;

class CSceneMap : public CSceneBase
{
	public:
		enum
		{
			EDITING_MODE_NONE,
			EDITING_MODE_TERRAIN,
			EDITING_MODE_OBJECT,
			EDITING_MODE_ENVIRONMENT,
			EDITING_MODE_ATTRIBUTE,
		};

		enum
		{
			BRUSH_TYPE_CIRCLE,
			BRUSH_TYPE_SQUARE,
		};

	public:
		CSceneMap();
		virtual ~CSceneMap();

		void Initialize();
		void CreateEnvironment();
		void SetMapManagerAccessor(CMapManagerAccessor * pMapManagerAccessor);

		void SetEditingMode(int iMode);

		void RefreshArea();

		/////////////////////////////////////////////////////////////////
		// Set Cursor
		CCursorRenderer * GetCursorRenererPointer();
		void SetObjectBrushType(int iType);
		void SetObjectBrushScale(int iScale);
		void SetObjectBrushDensity(int iDensity);
		void SetObjectBrushRandom(int iRandom);
		void SetCheckingNearObject(bool bFlag);
		void SetRandomRotation(bool bFlag);

		void SetCursorYaw(float fYaw);
		void SetCursorPitch(float fPitch);
		void SetCursorRoll(float fRoll);
		void SetCursorYawPitchRoll(const float fYaw, const float fPitch, const float fRoll);
		void SetCursorScale(uint32_t dwScale);

		void SetGridMode(BYTE byGridMode);
		void SetGridDistance(float fDistance);
		void SetObjectHeight(float fHeight);

		void RefreshCursor();
		void ClearCursor();
		void ChangeCursor(uint32_t dwCRC);

		float GetCursorYaw();
		float GetCursorPitch();
		float GetCursorRoll();
		float GetCursorObjectHeight();
		uint32_t GetCursorScale();
		/////////////////////////////////////////////////////////////////
		void AddSelectedObjectRotation(const float fYaw, const float fPitch, const float fRoll);
		void MoveSelectedObjectHeight(const float fHeight);

		void SetPatchGrid(bool bOn)					{ m_bPatchGridOn = bOn;					}
		bool GetPatchGrid()							{ return m_bPatchGridOn;				}
		void SetMapBoundGrid(bool bOn)				{ m_bMapBoundGridOn = bOn;				}
		bool GetMapBoundGrid()						{ return m_bMapBoundGridOn;				}

		void SetCompass(bool bOn)					{ m_bCompassOn = bOn;					}
		void SetMeterGrid(bool bOn)					{ m_bMeterGridOn = bOn;					}

		void SetCharacterRendering(bool bOn)		{ m_bCharacterRenderingOn = bOn;		}
		void SetWaterRendering(bool bOn)			{ m_bWaterRenderingOn = bOn;			}
		void SetObjectRendering(bool bOn)			{ m_bObjectRenderingOn = bOn;			}
		void SetObjectCollisionRendering(bool bOn)	{ m_bObjectCollisionRenderingOn = bOn;	}
		void SetTerrainRendering(bool bOn)			{ m_bTerrainRenderingOn = bOn;			}
		void SetObjectShadowRendering(bool bOn);
		void SetGuildAreaRendering(bool bOn);

		D3DXVECTOR3 GetMouseMapIntersect();
		void OnMovePosition(float fx, float fy);

		void LightPositionEditingStart()
		{
			m_bLightPositionEditingInProgress = true;
		}

		void LightPositionEditingEnd()
		{
			m_bLightPositionEditingInProgress = false;
		}

		void SetLightPositionEditing(bool value)
		{
			m_bLightPositionEditingOn = value;
		}

	protected:
		void OnUpdate();
		void OnRender(BOOL bClear);
		void OnRenderUI(float fx, float fy);
		void OnRenderLightDirection();
		void OnRenderEnvironmentMap();

		void OnRenderSceneAttribute();

		void OnSetCamera();

		void OnKeyDown(int iChar);
		void OnKeyUp(int iChar);
		void OnSysKeyDown(int iChar);		//sys key check 를 위해 추가
		void OnSysKeyUp(int iChar);			//sys key check 를 위해 추가
		void OnMouseMove(long x, long y);
		void OnLButtonDown(UINT nFlags, CPoint point);
		void OnLButtonUp();
		void OnRButtonDown();
		void OnRButtonUp();
		BOOL OnMouseWheel(short zDelta);

		void OnRenderCenterCursor();
		void OnRenderCompass();
		void OnRenderTerrainEditingArea();
		void OnRenderSelectedObject();
		void OnRenderObjectSettingArea();

		void OnRenderPatchGrid();
		void OnRenderMeterGrid();
		void OnRenderMapBoundGrid();
		void OnRenderCharacter();
		void OnRenderObjectCollision();

		void OnLightMove(const long & c_rlx, const long & c_rly);

		void __ClearCursor();

	private:
		void ObjectSelectAndDeselectStart(SHORT sAddedSelectKey, SHORT sDeselectKey);
		void ObjectSelectAndDeselectEnd(SHORT sAddedSelectKey, SHORT sDeselectKey);
		void InsertObjectOnTheSceneStart();
		void InsertObjectOnTheSceneEnd();

		void ClearRotation();
		void ClearHeightBias();

		//Focus
		void RotationYawFocus();
		void RotationPitchFocus();
		void RotationRollFocus();
		void HeightBiasFocus();

		//Render Enable
		void ObjectRenderEnable();
		void ObjectCollisionRenderEnable();

		//shadow
		void TerrainShadowMapAndMiniMapUpdate();

		//Water
		void WaterEraserEnable(bool bFlag);	//alt키 입력시 water brush의 eraser 를 활성화 시키기 위해 추가.

		//Attribue
		void AttributeEraserEnable(bool bFlag);
		void AttributeRenderEnable();				//다른탭에서도 속성을 볼수 있도록 하기 위해 추가

	public:
		//Water
		void WaterBrushCheck();		//에디트 모드(터레인) water brush 활성화.

		//Snap
		void SnapEnable();

		//Detach
		// 로드시 호출하기위해 public 으로 변경
		void DetachObject();

		//특정상황에서의 Undo/Redo 기능을 막기 위한 검사함수
		bool EditingModeCheckForUndoRedo();

	protected:
		CMapManagerAccessor *					m_pMapManagerAccessor;
		CMapManagerAccessor::CHeightObserver *	m_pHeightObserver;
		CMapOutdoorAccessor *		m_pMapAccessor;
		D3DXCOLOR					m_ClearColor;

		int							m_iEditingMode;
		D3DXVECTOR3					m_vecMouseMapIntersectPosition;
		D3DXCOLOR					m_EditCenterColor;
		D3DXCOLOR					m_EditablePointColor;
		D3DXCOLOR					m_PickingPointColor;

		// About Object Cursor
		BOOL						m_bObjectIsMoved;
		uint32_t						m_dwCursorObjectCRC;
		CGraphicTextInstance		m_TextInstance[3];

		bool						m_isCheckingNearObject;
		bool						m_isRandomRotation;

		bool						m_bCursorYawPitchChange;
		POINT						m_poCursorMouse;
		float						m_fBaseYaw;
		float						m_fBasePitch;
		CCursorRenderer				m_CursorRenderer;

		bool						m_bCompassOn;
		bool						m_bMeterGridOn;
		bool						m_bPatchGridOn;
		bool						m_bMapBoundGridOn;
		bool						m_bCharacterRenderingOn;
		bool						m_bWaterRenderingOn;
		bool						m_bObjectRenderingOn;
		bool						m_bObjectCollisionRenderingOn;
		bool						m_bTerrainRenderingOn;
		bool						m_bShadowRenderingOn;
		bool						m_bGuildAreaRenderingOn;
		LPDIRECT3DVERTEXBUFFER9		m_pBigSquareVB;

		bool						m_bLightPositionEditingInProgress;
		bool						m_bLightPositionEditingOn;

		CPoint						m_ptClick;
		long						m_loldX;
		long						m_loldY;

		CGraphicImageInstance		m_pCompasGraphicImageInstance;

		// UI
		CGraphicTextInstance		m_textInstanceSplatTextureCount;
		CGraphicTextInstance		m_textInstanceSplatMeshCount;
		CGraphicTextInstance		m_textInstanceSplatMeshPercentage;
		CGraphicTextInstance		m_textInstancePatchSplatTileCount;
		CGraphicTextInstance		m_textInstanceTexture0Count;

		// Snow
		CSnowEnvironment			m_kSnowEnvironment;

		// Visible
		TOutdoorMapCoordinate		m_kPrevCoordinate;

		// Attribute
		bool						m_bAttrRenderingOn;

		// ShadowMap & MiniMap
		//bool						m_bTerrainShadowUpdateNeeded;
		bool						m_bTerrainShadowMapAndMiniMapUpdateNeeded;
		bool						m_bTerrainShadowMapAndMiniMapUpdateAll;		// 추가 : 전체영역, 현재영역 둘다 가능하게끔 하기위해

		//hold
		bool						m_bHoldOn;

	public:
		// 추가
		void SetTerrainShadowMapAndMiniMapUpdateNeeded(bool bFlag) { m_bTerrainShadowMapAndMiniMapUpdateNeeded = bFlag; }
		void SetTerrainShadowMapAndMiniMapUpdateAll(bool bFlag) { m_bTerrainShadowMapAndMiniMapUpdateAll = bFlag; }

		// (목)
		// Hold 기능 추가
		bool GetHoldOn() { return m_bHoldOn; }
		void SetHoldOn(bool bFlag) { m_bHoldOn = bFlag; }
		void SwapHoldOn();
		void HoldStart();
		void HoldEnd();

	//(금) Gizmo 관련
	private:
		CPickingArrows m_cPickingArrows;
		bool m_bGizmoOn;
		AreaObjectRef m_pickedObject;
		int m_iGizmoSelectedAxis;

	private: // Hold 일때만 사용되어서 private 로...
		bool GizmoChecking(AreaObjectRef& ref);
		void GizmoUpdate(const AreaObjectRef& ref);

	public:
		void GizmoOff();
};

METIN2_END_NS

#endif

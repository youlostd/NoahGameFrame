#ifndef METIN2_TOOL_WORLDEDITOR_DATACTRL_MAPACCESSORAREA_HPP
#define METIN2_TOOL_WORLDEDITOR_DATACTRL_MAPACCESSORAREA_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif
#define WORLD_EDITOR

#include <GameLib/Area.h>

#include <vstl/string.hpp>

#include <set>

METIN2_BEGIN_NS

class CAreaAccessor : public CArea
{
	public:
		struct SSelectObject
		{
			uint32_t dwIndex;
			uint32_t dwCRC32;
		};
		typedef std::list<SSelectObject> TSelectObjectList;

		CAreaAccessor();
		virtual ~CAreaAccessor();

		void Clear();
		void UndoClear();

		bool Save(const std::string & c_rstrMapName);

		void RefreshArea();
		void UpdateObject(uint32_t dwIndex, const TObjectData * c_pObjectData); // Height, Rotation

		void AddObject(const TObjectData * c_pObjectData);
		bool GetObjectPointer(uint32_t dwIndex, TObjectData ** ppObjectData);

		int GetPickedObjectIndex();
		BOOL IsSelectedObject(uint32_t dwIndex);
		void SelectObject(uint32_t dwIndex);
		void DeselectObject(uint32_t dwIndex);

		int GetSelectedObjectCount();
		const CArea::TObjectData* GetLastSelectedObjectData() const;		// 가장 마지막에 선택한 오브젝트의 정보(위치, 회전 등..)를 리턴

		BOOL Picking();

		void RenderSelectedObject();
		void CancelSelect();
		BOOL SelectObject(float fxStart, float fyStart, float fxEnd, float fyEnd, SHORT sSpecialKeys = 0);
		BOOL DeselectObject(float fxStart, float fyStart, float fxEnd, float fyEnd);

		void DeleteSelectedObject();
		void MoveSelectedObject(float fx, float fy);
		void MoveSelectedObjectHeight(float fHeight);
		void AddSelectedAmbienceScale(int iAddScale);
		void AddSelectedAmbienceMaxVolumeAreaPercentage(float fPercentage);
		void AddSelectedObjectRotation(float fYaw, float fPitch, float fRoll);
		void SetSelectedObjectPortalNumber(int iID);
		void DelSelectedObjectPortalNumber(int iID);
		void CollectPortalNumber(std::set<int> * pkSet_iPortalNumber);
		BOOL IsSelected();

		//////////////////////////////////////////////////////////////////////////
		// Shadow Map
		void RenderToShadowMap();

		// 건물 텍스처 리로딩
		void ReloadBuildingTexture();
		// Dungeon Block 텍스처 리로딩
		void ReloadDungeonBlockTexture();

		// 추가
		// 오브젝트를 추가 선택하는 기능을 위해 추가
		bool	IsAddedSelectObject(uint32_t dwNum);

		//오브젝트 선택과 관련
		// down ~ up 시점까지 추가된 오브젝트가 있는지 검사하기 위한용도
		// down 시점에서 m_bAddedSelectObject를 false로 셋팅
		// up 시점에서 IsAddedSelectObject()로 체크
		bool	IsAddedSelectObject()	{ return m_bAddedSelectObject; }
		void	SetAddedSelectObject(bool bFlag) { m_bAddedSelectObject = bFlag; }
		void	ClearAddedSelectObject();

		// 오브젝트를 해제 하는 기능을 위해 추가
		bool	IsDeselectObject(uint32_t dwNum);
		void	ClearDeselectObject();

		//외부에서 오브젝트의 Height 값을 알수있도록 인터페이스를 제공하기위해 추가.
		bool GetObjectHeight(float fX, float fY, float* fHeight);

		void CopyObjectAttr(uint8_t* attr);

	protected:
		bool CheckInstanceIndex(uint32_t dwIndex);
		bool GetInstancePointer(uint32_t dwIndex, TObjectInstance ** ppObjectInstance);

		bool __SaveObjects(const char * filename);
		bool __SaveAmbiences(const char * c_szOtherPathName, const char * c_szFileName);

		void __ClickObject(uint32_t dwIndex);
		void __RefreshSelectedInfo();
		void __RefreshObjectPosition(float fx, float fy, float fz);

		TSelectObjectList							m_SelectObjectList;
		std::list<uint32_t>							m_AddedSelectObjectList;
		std::list<uint32_t>							m_DeselectObjectList;
		bool										m_bAddedSelectObject;
};

METIN2_END_NS

#endif

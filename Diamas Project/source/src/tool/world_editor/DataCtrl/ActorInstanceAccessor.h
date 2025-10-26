#ifndef METIN2_TOOL_WORLDEDITOR_DATACTRL_ACTORINSTANCEACCESSOR_HPP
#define METIN2_TOOL_WORLDEDITOR_DATACTRL_ACTORINSTANCEACCESSOR_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif
#define WORLD_EDITOR

#include <gamelib/ActorInstance.h>

#include <vstl/string.hpp>

METIN2_BEGIN_NS

class CActorInstanceAccessor : public CActorInstance
{
	public:
		CActorInstanceAccessor();
		virtual ~CActorInstanceAccessor();

		void ClearModel();
		void ClearMotion();
		void ClearAttachingEffect();

		BOOL IsModelThing();
		BOOL IsMotionThing();

		BOOL SetAccessorModel(CGraphicThing::Ptr pThing);
		BOOL SetAccessorMotion(CGraphicThing::Ptr pThing);

		void SetMotion();
		void SetMotionData(CRaceMotionData * pMotionData);
		float GetMotionDuration();

		uint32_t GetBoneCount();
		void SetLocalTime(float fLocalTime);
		BOOL GetBoneName(uint32_t dwIndex, std::string * pstrBoneName);

		BOOL GetBoneMatrix(uint32_t dwBoneIndex, D3DXMATRIX ** ppMatrix);
		BOOL GetBoneIndexByName(const char * c_szBoneName, int * pBoneIndex) const;

		void ClearAttachingObject();
		void AttachObject(const char * c_szFileName, const char * c_szBoneName);
		void UpdateAttachingObject();
		void RenderAttachingObject();

	protected:
		CGraphicThing::Ptr m_pModelThing;
		CGraphicThing::Ptr m_pMotionThing;

		struct SAttachingModelInstance
		{
			std::string strBoneName;
			CGraphicThing::Ptr pThing;
			CGrannyModelInstance * pModelInstance;
		};

		std::list<SAttachingModelInstance> m_AttachingObjectList;
};

METIN2_END_NS

#endif


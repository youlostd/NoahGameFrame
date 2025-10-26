#ifndef METIN2_TOOL_WORLDEDITOR_DATACTRL_MODELINSTANCEACCESSOR_HPP
#define METIN2_TOOL_WORLDEDITOR_DATACTRL_MODELINSTANCEACCESSOR_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <vstl/string.hpp>

METIN2_BEGIN_NS

class CModelInstanceAccessor : public CGrannyModelInstance
{
	public:
		CModelInstanceAccessor();
		virtual ~CModelInstanceAccessor();

		void ClearModel();
		void ClearMotion();
		void ReleaseArtificialMotion();

		void UpdateLocalTime(float fLocalTime);
		float GetLocalTime();
		float GetDuration();

		BOOL SetAccessorModel(CGraphicThing::Ptr pThing);
		BOOL SetAccessorMotion(CGraphicThing::Ptr pThing);
		void SetArtificialMotion(const CGrannyMotion * pMotion);

		// Interceptor Functions
		void Deform(const D3DXMATRIX* c_pWorldMatrix);
		void UpdateWorldPose(const D3DXMATRIX* c_pWorldMatrix);

		// Access Data
		BOOL IsModelThing();
		BOOL IsMotionThing();
		uint32_t GetBoneCount();
		BOOL GetBoneName(uint32_t dwIndex, std::string * pstrBoneName);

	protected:
		float m_fLocalTime;

		CGraphicThing::Ptr m_modelThing;
		CGraphicThing::Ptr m_pcurMotionThing;

		// NOTE : Motion을 Release 시켜버리면 Granny 내부 애니메이션 처리시 충돌 발생
		//        아마도 Motion이 끝난 이후에도 MotionFileData 자체를 참고 할때가 있는 듯..
		//        모아 두웠다 프로그램 종료시 클리어 한다. - [levites]
		std::list<CGraphicThing::Ptr> m_pMotionBackupList;

		int m_boneCount;
		granny_local_pose *	m_pgrnLocalPose;
};

METIN2_END_NS

#endif

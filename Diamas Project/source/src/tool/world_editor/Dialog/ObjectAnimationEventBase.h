#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_OBJECTANIMATIONEVENTBASE_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_OBJECTANIMATIONEVENTBASE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../DataCtrl/ObjectAnimationAccessor.h"

METIN2_BEGIN_NS

class CObjectAnimationEventBase : public CDialog
{
	public:
		CObjectAnimationEventBase(UINT nIDTemplate, CWnd* pParentWnd = NULL) : CDialog(nIDTemplate, pParentWnd) {}
		virtual ~CObjectAnimationEventBase(){}

		virtual BOOL Create(CWnd * pParent, const CRect & c_rRect) = 0;
		virtual void Close() = 0;

		virtual void GetData(CRaceMotionDataAccessor::TMotionEventData * pData) = 0;
		virtual void SetData(const CRaceMotionDataAccessor::TMotionEventData * c_pData) = 0;
};

METIN2_END_NS

#endif

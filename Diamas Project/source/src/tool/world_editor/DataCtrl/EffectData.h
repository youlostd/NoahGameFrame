#ifndef METIN2_TOOL_WORLDEDITOR_DATACTRL_EFFECTDATA_HPP
#define METIN2_TOOL_WORLDEDITOR_DATACTRL_EFFECTDATA_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CEffectData : public CEffectAccessor
{
	public:
		CEffectData();
		virtual ~CEffectData();

		void Initialize();
};

METIN2_END_NS

#endif


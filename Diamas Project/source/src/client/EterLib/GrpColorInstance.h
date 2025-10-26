#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#undef GetCurrentTime

#include "GrpColor.h"
#include "Pool.h"

class CGraphicColorInstance
{
	public:
		CGraphicColorInstance();
		virtual ~CGraphicColorInstance();

		void Clear();

		void SetColorReference(const CGraphicColor& c_rSrcColor);
		void BlendColorReference(uint64_t blendTime, const CGraphicColor& c_rDstColor);

		void Update();

		const CGraphicColor& GetCurrentColorReference() const;

	protected:
		uint64_t GetCurrentTime();

	protected:
		CGraphicColor m_srcColor;
		CGraphicColor m_dstColor;
		CGraphicColor m_curColor;

		uint64_t m_baseTime;
		uint64_t m_blendTime;
};

using TGraphicColorInstancePool = CDynamicPool<CGraphicColorInstance>;

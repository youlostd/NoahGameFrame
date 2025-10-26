#include "StdAfx.h"
#include "Font.hpp"
#include "FontManager.hpp"
#include "Engine.hpp"

#include <storm/StringUtil.hpp>
#include <storm/Log.hpp>

METIN2_BEGIN_NS

Font::Font()
{
}

Font::~Font()
{
}

bool Font::Create(const storm::StringRef& name, uint8_t size, bool italic, bool bold)
{
	m_fontName = name.to_string();

	// format
	// 굴림     "굴림" 폰트 기본 사이즈 12 로 로딩
	// 굴림:18  "굴림" 폰트 사이즈 18 로 로딩
	// 굴림:14i "굴림" 폰트 사이즈 14 & 이탤릭으로 로딩
	storm::StringRef realName;
	const auto pos = name.find(':');
	if (pos != storm::StringRef::npos) {
		realName = name.substr(0, pos);

		storm::StringRef sizeStr = name.substr(pos + 1);

		if (sizeStr.back() == 'i') {
			italic = true;
			sizeStr.remove_suffix(1);
		}

		if (sizeStr.back() == 'b') {
			bold = true;
			sizeStr.remove_suffix(1);
		}

		storm::ParseNumber(sizeStr, size);
	} else {
		realName = name;
	}

	STORM_DLOG(Info, "Creating font {0} size {1} italic {2}",
	           realName, size, italic);

	if (!m_fontTexture.Create(realName, size, italic, bold))
		return false;

	return true;
}

void DestroyReferenceCounted(Font* font)
{
	STORM_DLOG(Info, "Destroying font {0}", font->m_fontName);
	GetFontManager().DestroyFont(font);
}

METIN2_END_NS

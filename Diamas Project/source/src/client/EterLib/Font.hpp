#ifndef METIN2_CLIENT_ETERLIB_FONT_HPP
#define METIN2_CLIENT_ETERLIB_FONT_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GrpFontTexture.h"

#include <base/RefCounted.hpp>

#include <boost/smart_ptr/intrusive_ptr.hpp>

METIN2_BEGIN_NS

class Font : public RefCounted<Font>
{
	friend void DestroyReferenceCounted(Font* font);

	public:
		typedef boost::intrusive_ptr<Font> Ptr;

		Font();
		~Font();

		bool Create(const storm::StringRef& name,
		            uint8_t size = 12,
		            bool italic = false,
		            bool bold = false);

		const storm::String& GetName() const;

		CGraphicFontTexture& GetTexturePointer();

	private:
		storm::String m_fontName;
		CGraphicFontTexture m_fontTexture;
};

BOOST_FORCEINLINE const storm::String& Font::GetName() const
{ return m_fontName; }

BOOST_FORCEINLINE CGraphicFontTexture& Font::GetTexturePointer()
{ return m_fontTexture; }

METIN2_END_NS

#endif

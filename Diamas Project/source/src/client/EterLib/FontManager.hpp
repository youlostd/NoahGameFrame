#ifndef METIN2_CLIENT_ETERLIB_FONTMANAGER_HPP
#define METIN2_CLIENT_ETERLIB_FONTMANAGER_HPP
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GrpText.h"

#include <boost/unordered_map.hpp>
#include <unordered_map>

#include "base/GroupTextTree.hpp"

class FontManager
{
public:
    FontManager();
    ~FontManager();

    Font *LoadFont(const std::string &name);
    void RegisterFont(const std::string &filename);

    void SetDefaultFont(const std::string &normal,
                        const std::string &italic,
						const std::string &bold);

    Font *GetDefaultFont();
    Font *GetDefaultItalicFont();

private:
    typedef std::map<
        std::string,
        Font *, ci_less> FontMap;

    void DestroyFont(Font *font);

    FontMap m_fontMap;
    FontMap m_fontFamilyMap;
	std::map<std::string, std::string> m_fontFiles;
	std::multimap<std::string, FT_Face> m_faces;
    std::map<std::string,  std::unique_ptr<VfsFile>> m_mappedFiles;

    std::string m_defaultFont;
    std::string m_defaultItalicFont;
    std::string m_defaultBoldFont;

    FT_Library m_library;
};

#endif

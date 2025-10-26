#include "StdAfx.h"
#include "FontManager.hpp"

#include <stdexcept>
#include <knownfolders.h>
#include <shlobj_core.h>
#include <fmt/format.h>

#include "../eterBase/Stl.h"
#include <pak/Vfs.hpp>

inline std::string WToStr(const std::wstring &s)
{
    std::string temp(s.length(), ' ');
    std::copy(s.begin(), s.end(), temp.begin());
    return temp;
}
std::string getFontPath()
{
    wchar_t *pOut;
    SHGetKnownFolderPath(FOLDERID_Fonts, 0, 0, &pOut);
    std::string res = WToStr(pOut);
    CoTaskMemFree(pOut);
    return res;
}

struct FontData
{
    std::string name;
    std::string style;
    uint8_t size;
};

FontData GetFontDataByDefinition(const std::string &name)
{
    std::string nameCopy = name;
    std::string styleName = "Regular";
    uint8_t size = 12;

    std::string realName;
    const auto pos = nameCopy.find(':');
    if (pos != std::string::npos)
    {
        realName = nameCopy.substr(0, pos);

        if (nameCopy.back() == 'i')
        {
            styleName = "Italic";
            nameCopy.pop_back();
        }

        if (nameCopy.back() == 'b')
        {
            styleName = "Bold";
            nameCopy.pop_back();
        }

        if (nameCopy.back() == 's')
        {
            styleName = "SemiBold";
            nameCopy.pop_back();
        }

        if (nameCopy.back() == 'l')
        {
            styleName = "Light";
            nameCopy.pop_back();
        }

        
        if (nameCopy.back() == 'm')
        {
            styleName = "Medium";
            nameCopy.pop_back();
        }
        const std::string sizeStr(nameCopy.c_str() + pos + 1);
        size = atoi(sizeStr.c_str());
    }
    else
    {
        realName = name;
    }

    return {realName, styleName, size};
}

FontManager::FontManager()
{
    const auto error = FT_Init_FreeType(&m_library);
    if (error)
    {
        throw new std::runtime_error("Could not initialize freetype");
    }

    const char *paths[] = {"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts",
                           "SOFTWARE\\Microsoft\\WindowsNT\\CurrentVersion\\Fonts",
                           "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts"};
    for (int i = 0; i < 3; ++i)
    {
        HKEY fontsKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, paths[i], 0, KEY_READ, &fontsKey) == ERROR_SUCCESS)
        {
            std::string fontPath = getFontPath();

            const int BufSize = 32000;
            char value[BufSize];
            DWORD valueSize = BufSize;
            char data[BufSize];
            DWORD dataSize = BufSize;
            DWORD type;
            int i = 0;
            while (RegEnumValueA(fontsKey, i, value, &valueSize, 0, &type, (LPBYTE)data, &dataSize) !=
                   ERROR_NO_MORE_ITEMS)
            {
                valueSize = BufSize;
                dataSize = BufSize;
                ++i;

                std::string v = value;
                int pos = v.find("(TrueType)");
                if (pos == std::string::npos || type != REG_SZ)
                {
                    continue;
                }

                v = v.substr(0, pos - 1);
                m_fontFiles[v] = fontPath + "\\" + data;
            }

            break;
        }
    }
}

FontManager::~FontManager()
{
    for (auto i = m_faces.begin(); i != m_faces.end(); ++i)
    {
        FT_Done_Face(i->second);
    }

    stl_wipe_second(m_fontMap);

    FT_Done_FreeType(m_library);
}

Font *FontManager::LoadFont(const std::string &fileName)
{
    const auto it = m_fontMap.find(fileName);
    if (it != m_fontMap.end())
        return it->second;

    auto fontData = GetFontDataByDefinition(fileName);

    auto r = m_faces.equal_range(fontData.name);

    // If no such font is loaded...
    if (r.first == r.second)
    {
        auto it = m_fontFiles.find(fontData.name);
        if (it != m_fontFiles.end())
        {
            RegisterFont(it->second);
            r = m_faces.equal_range(fontData.name);
        }
    }

    FT_Face f = nullptr;

    for (auto i = r.first; i != r.second; ++i)
    {
        if (i->second->style_name == fontData.style)
        {
            f = i->second;
            break;
        }
    }

    if (!f && r.first != r.second)
    {
        // Just default to the first of what we found
        f = r.first->second;
    }
    else if (!f)
    {
        SPDLOG_ERROR("Unable to find specified font face");
        return nullptr;
    }

    int error = 0;
    error = FT_Set_Pixel_Sizes(f, 0, fontData.size);

    if (error)
    {
        SPDLOG_ERROR("Specified font size not supported by font");
        return nullptr;
    }

    auto p = new Font();
    if (!p->Create(f, fontData.size))
        return nullptr;

    m_fontMap.emplace(fileName, p);
    return p;
}

void FontManager::RegisterFont(const std::string &filename)
{
    SPDLOG_DEBUG("Registering Font {}", filename.c_str());
    const auto it = m_fontMap.find(filename);
    if (it != m_fontMap.end())
        return;

    auto fp = GetVfs().Open(filename, kVfsOpenFullyBuffered);

    if (!fp)
        return;

    const auto size = fp->GetSize();

    storm::View data(storm::GetDefaultAllocator());
    fp->GetView(0, data, size);

    FT_Face face;
    auto error = FT_New_Memory_Face(m_library, (const FT_Byte *)data.GetData(), size, 0, &face);
    if (error)
    {
        SPDLOG_ERROR("Could not load face for font %s", filename.c_str());
        return;
    }

    m_mappedFiles.emplace(filename, std::move(fp));

    error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
    if (error)
    {
        SPDLOG_ERROR("Could not set encoding while processing face for font %s", filename.c_str());
        return;
    }

    std::string familyName = face->family_name;

    m_faces.insert(make_pair(familyName, face));

    int numFaces = face->num_faces;
    for (int i = 1; i < numFaces; ++i)
    {
        error = FT_New_Memory_Face(m_library, (const FT_Byte *)data.GetData(), size, i, &face);

        if (error)
        {
            continue;
        }

        std::string familyName = face->family_name;

        m_faces.insert(make_pair(familyName, face));
    }
}

void FontManager::SetDefaultFont(const std::string &normal, const std::string &italic, const std::string &bold)
{
    m_defaultFont = normal;
    m_defaultItalicFont = italic;
    m_defaultBoldFont = bold;
}

Font *FontManager::GetDefaultFont() { return LoadFont(m_defaultFont); }

Font *FontManager::GetDefaultItalicFont() { return LoadFont(m_defaultItalicFont); }

void FontManager::DestroyFont(Font *CGraphicText)
{
    assert(!CGraphicText->GetName().empty() && "CGraphicText name empty");
    m_fontMap.erase(CGraphicText->GetName());
    delete CGraphicText;
}

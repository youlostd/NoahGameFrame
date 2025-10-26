#ifndef METIN2_SERVER_GAME_LOCALE_HPP
#define METIN2_SERVER_GAME_LOCALE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <optional>
#include <base/robin_hood.h>
class LocaleStringTable
{
public:
    LocaleStringTable() = default;

    LocaleStringTable(LocaleStringTable &&other) noexcept;

    LocaleStringTable(const LocaleStringTable &other) = delete;
    LocaleStringTable &operator=(const LocaleStringTable &other) = delete;

    bool Load(const std::string &filename, const std::string &localeCode);

    const char *Translate(const std::string &input) const;

private:
    typedef robin_hood::unordered_map<
        std::string, std::string
    > LocaleMap;

    std::string m_content;
    std::string m_localeCode;

    // Strings are owned by |m_content|
    LocaleMap m_strings;
};

struct Locale
{
    std::string name;
    std::string path;
    LocaleStringTable stringTable;
    LocaleStringTable oxTable;
};

class LocaleService
{
public:
    using LocaleMap = std::unordered_map<std::string, Locale>;

    void AddLocale(const std::string &name);
    const Locale *GetLocale(const std::string &name);

    void SetDefaultLocale(const std::string &name);
    const Locale &GetDefaultLocale() const;

    const LocaleMap &GetLocales() const { return m_locales; }

private:
    LocaleMap m_locales;
    Locale *m_defaultLocale = nullptr;
};

extern LocaleService &GetLocaleService();

#define LC_TEXT(text) GetLocaleService().GetDefaultLocale().stringTable.Translate(text)
#define LC_TEXT_LC(text, locale) locale->stringTable.Translate(text)
uint8_t GetLanguageIDByName(const std::string &lang);
std::optional<std::string> GetLanguageNameByID(uint8_t code);

#endif /* METIN2_SERVER_GAME_LOCALE_HPP */

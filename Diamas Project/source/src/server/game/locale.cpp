#include "locale_service.h"
#include <boost/range/adaptor/indexed.hpp>
#include <boost/tokenizer.hpp>
#include <fstream>
#include "config.h"
#include <optional>
#include "locale.hpp"

LocaleStringTable::LocaleStringTable(LocaleStringTable &&other) noexcept
    : m_content(std::move(other.m_content))
      , m_strings(std::move(other.m_strings))
{
    //
}

bool LocaleStringTable::Load(const std::string &filename, const std::string &localeCode)
{
    std::ifstream t(filename);
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());

    m_content = str;
    m_localeCode = localeCode;

    typedef boost::tokenizer<boost::char_separator<char>, std::string::const_iterator, std::string> tokenizer;
    boost::char_separator<char> sep{"\n"};
    tokenizer tok{m_content, sep};
    std::vector<std::string> args;

    for (const auto &t : boost::adaptors::index(tok, 0))
    {
        args.clear();

        boost::char_separator<char> sepi{"\t"};
        tokenizer token{t.value(), sepi};
        args.assign(token.begin(), token.end());

        if (args.empty())
            continue;

        switch (args.size())
        {
        case 2:
            m_strings[args[0]] = args[1];
            break;
        case 1:
            m_strings[args[0]].clear();
            break;

        default: {
            SPDLOG_ERROR("locale_string '%s' line %d has %d tokens",
                         filename.c_str(), t.index(), args.size());
            break;
        }
        }
    }

    return true;
}

const char *LocaleStringTable::Translate(const std::string &input) const
{
    const auto it = m_strings.find(input);
    if (it != m_strings.end())
        return it->second.c_str();

    if (gConfig.testServer)
        SPDLOG_ERROR("Missing translation in language {0} for {1}", m_localeCode.c_str(), input);
    return input.c_str();
}

void LocaleService::AddLocale(const std::string &name)
{
    const auto p = m_locales.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple());
    if (!p.second)
        return;

    auto &locale = p.first->second;
    locale.name = name;
    locale.path = "data/locale/" + locale.name;

    locale.stringTable.Load(locale.path + "/locale_string.txt", locale.name);
    locale.oxTable.Load(locale.path + "/locale_ox.txt", locale.name);
}

const Locale *LocaleService::GetLocale(const std::string &name)
{
    const auto it = m_locales.find(name);
    if (it != m_locales.end())
        return &it->second;

    return nullptr;
}

void LocaleService::SetDefaultLocale(const std::string &name)
{
    const auto it = m_locales.find(name);
    if (it != m_locales.end())
        m_defaultLocale = &it->second;
    else
        m_defaultLocale = nullptr;
}

const Locale &LocaleService::GetDefaultLocale() const
{
    assert(m_defaultLocale && "Precondition not satisfied");
    return *m_defaultLocale;
}

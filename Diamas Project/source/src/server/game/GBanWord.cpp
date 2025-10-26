#include "constants.h"
#include "GBanWord.h"

#include <absl/strings/match.h>
#include <boost/algorithm/string.hpp>

#include "config.h"

extern void SendLog(const char *c_pszBuf); // 운영자에게만 공지

bool CBanwordManager::Initialize(TBanwordTable *p, uint16_t wSize)
{
    m_hashmap_words.clear();

    for (uint16_t i = 0; i < wSize; ++i, ++p)
        m_hashmap_words[p->szWord] = true;

    SendLog(fmt::format("Banword reloaded! (total {} banwords)", m_hashmap_words.size()).c_str());
    return true;
}

bool CBanwordManager::Find(const char *c_pszString)
{
    return m_hashmap_words.end() != m_hashmap_words.find(c_pszString);
}

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++)
    {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

bool CBanwordManager::CheckString(const char *c_pszString, uint32_t len)
{
    if (m_hashmap_words.empty())
        return false;

    auto it = m_hashmap_words.begin();
    while (it != m_hashmap_words.end())
    {
        const auto &r = it->first;

        const char *tmp = c_pszString;
        int32_t len2 = len;

        while (len2 > 0)
        {
            if (!strcicmp(tmp, r.c_str()))
                return true;

            ++tmp;
            --len2;
        }

        it++;
    }

    return false;
}

bool CBanwordManager::CheckString(const std::string &string)
{
    if (m_hashmap_words.empty())
        return false;

    for (const auto &elem : m_hashmap_words)
    {
        const auto &r = elem.first;

        if (string.find(r) != std::string::npos)
            return true;
    }

    return false;
}

void CBanwordManager::ConvertString(char *c_pszString, uint32_t len)
{
    auto it = m_hashmap_words.begin();
    while (it != m_hashmap_words.end())
    {
        const auto &r = it->first;

        char *tmp = c_pszString;
        int32_t len2 = len;

        while (len2 > 0)
        {
            if (*tmp == '*')
            {
                ++tmp;
                --len2;
                continue;
            }

            if (!strncmp(tmp, r.c_str(), r.size()))
            {
                memset(tmp, '*', r.size());
                tmp += r.size();
                len2 -= r.size();
            }
            else
            {
                ++tmp;
                --len2;
            }
        }

        it++;
    }
}

void CBanwordManager::ConvertString(std::string& string)
{
    std::vector<std::string> word_list;  
    boost::split(word_list, string, boost::is_any_of("\\ +"));
            std::string stars; 

    auto it = m_hashmap_words.begin();
    while (it != m_hashmap_words.end())
    {
        stars.clear();

        const auto &r = it->first;
        
        for (int i = 0; i < r.size(); i++) 
            stars += '*';

        if(absl::StrContains(string, r)) {
            boost::replace_all(string, r, stars);
        }

        ++it;
    }
}

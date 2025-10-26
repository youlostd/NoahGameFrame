#include "StdAfx.h"
#include "parser.h"

#include <boost/locale/utf.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/token_functions.hpp>
#include <boost/tokenizer.hpp>

namespace
{
typedef boost::locale::utf::utf_traits<char> Utf8;

std::size_t ExtractBox(std::string &source)
{
    assert(source[0] == '[' && "Requirement");

    auto boxEnd = source.find(']');
    if (boxEnd == std::string::npos)
    {
        SPDLOG_ERROR(
            "Syntax error: EOF while looking for ']' in {0}",
            source);
        return {};
    }

    source = source.substr(1, boxEnd - 1);
    boost::trim(source);

    return boxEnd + 1;
}
}

uint32_t ScriptGroup::Create(const std::string &source)
{
    m_cmdList.clear();

    auto lineCount = 1;

    if (source.empty())
        return 0;

    for (std::size_t pos = 0, size = source.size(); pos != size;)
    {
        SCmd cmd;

        if (source[pos] == '[')
        {
            std::string box = source.substr(pos);
            pos += ExtractBox(box);

            auto space = box.find(' ');
            if (space == std::string::npos)
            {
                cmd.name = box;
                m_cmdList.push_back(std::move(cmd));
                continue;
            }

            cmd.name = box.substr(0, space);
            if (!ParseArguments(box.substr(space),
                                cmd.argList))
                return 0;
        }
        else if (source[pos] == '\r' || source[pos] == '\n')
        {
            if (source[pos] == '\n')
                ++lineCount;
            ++pos;
            continue;
        }
        else
        {
            auto it = source.begin() + pos;

            auto c = Utf8::decode(it, source.end());
            if (c == boost::locale::utf::illegal || c == boost::locale::utf::incomplete)
            {
                SPDLOG_ERROR("Invalid UTF-8 seq. in {0}", source);
                return -1;
            }

            const auto val = source.substr(pos, (it - source.begin()) - pos);

            cmd.name = "LETTER";
            cmd.argList.push_back(SArgument("value", val));
            pos += val.size();
        }

        m_cmdList.push_back(std::move(cmd));
    }

    return lineCount;
}

bool ScriptGroup::GetCmd(SCmd &cmd)
{
    if (m_cmdList.empty())
        return false;

    cmd = m_cmdList.front();
    m_cmdList.pop_front();
    return true;
}

bool ScriptGroup::ReadCmd(SCmd &cmd)
{
    if (m_cmdList.empty())
        return false;

    cmd = m_cmdList.front();
    return true;
}

bool ScriptGroup::ParseArguments(const std::string &source,
                                 TArgList &argList)
{
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;
    const boost::char_separator<char> sep{"|"};
    Tokenizer tok{source, sep};
    for (auto &t : tok)
    {
        std::string name, value, cur = t;

        auto nameValueSep = cur.find(';');
        if (nameValueSep == std::string::npos)
        {
            name = cur;
        }
        else
        {
            name = cur.substr(0, nameValueSep);
            value = cur.substr(nameValueSep + 1);
        }

        boost::trim(name);
        boost::trim(value);

        argList.push_back(SArgument(name, value));
    }

    return true;
}

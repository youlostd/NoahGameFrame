/*
 * Project     : Inferna
 * Author      : ymir | GSeMinZ
 * Date        : 2008.11.17
 */

#ifndef SITokenParser_h_
#define SITokenParser_h_
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdio>

class SITokenParser
{
public:
    SITokenParser() { m_tokens.clear(); }

    int ReadLine(const std::string &buf)
    {
        m_tokens.clear();
        using boost::tokenizer;
        using boost::escaped_list_separator;
        typedef tokenizer<escaped_list_separator<char>> so_tokenizer;

        std::string separator1("");     //dont let quoted arguments escape themselves
        std::string separator2("\t ");  //split on spaces and tabs
        std::string separator3("\"\'"); //let it have quoted arguments

        auto els = escaped_list_separator<char>(separator1, separator2, separator3);

        so_tokenizer tok(buf, els);
        for (auto &token : tok) { if (!token.empty()) { m_tokens.push_back(token); } }

        return m_tokens.size();
    }

    int GetTokenNum() { return m_tokens.size(); }

    const char *GetToken(int idx) { return __GetToken(idx).c_str(); }

    const int GetTokenInt(int idx)
    {
        auto Token = __GetToken(idx);
        if (Token.empty())
            return 0;

        if (!is_positive_number(Token))
            return 0;

        int32_t val = 0;
        storm::ParseNumber(Token.c_str(), val);

        return val;
    }

    const long long GetTokenLongLong(int idx)
    {
        auto Token = __GetToken(idx);
        if (Token.empty())
            return 0;

        if (!is_positive_number(Token))
            return 0;

        int64_t val = 0;
        storm::ParseNumber(Token.c_str(), val);

        return val;
    }

    const float GetTokenFloat(int idx)
    {
        auto Token = __GetToken(idx);
        if (Token.empty())
            return 0;
        float val = 0;
        storm::ParseNumber(Token.c_str(), val);

        return val;
    }

    std::string GetTokenString(int idx)
    {
        auto Token = __GetToken(idx);
        if (Token.empty())
            return std::string();
        return __GetToken(idx);
    }

    const int GetTokenHex(int idx)
    {
        auto Token = __GetToken(idx);
        if (Token.empty())
            return 0;
        return std::stoi(Token, nullptr, 16);
    }

    const bool GetTokenBool(int idx)
    {
        auto Token = __GetToken(idx);
        if (Token.empty())
            return 0;
        uint8_t val = 0;
        storm::ParseNumber(Token.c_str(), val);

        return val;
    }

    bool CompareToken(int idx, const char *Buf)
    {
        if (idx >= m_tokens.size())
            return false;

        return boost::iequals(m_tokens.at(idx), Buf);
    }

    bool CompareCaseToken(int idx, const char *Buf)
    {
        if (idx >= m_tokens.size())
            return false;

        return boost::equals(m_tokens.at(idx), Buf);
    }

private:
    std::string __GetToken(int idx)
    {
        if (idx >= m_tokens.size())
            return "";

        return m_tokens.at(idx);
    }

    std::vector<std::string> m_tokens;
};

#endif	/* _SITokenParser_h_ */

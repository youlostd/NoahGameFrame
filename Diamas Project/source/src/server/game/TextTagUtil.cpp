#include "TextTagUtil.hpp"

namespace TextTag
{
std::string emote(const std::string &emote) { return fmt::format("|E{0}|e", emote); }

std::string color(const std::string &color, const std::string &text) { return fmt::format("|c{0}{1}|r", color, text); }

std::string hyperlink(const std::string &type, const std::string &data, const std::string &text)
{
    return fmt::format("|H{0}:{1}|h{2}|h", type, data, text);
}

std::string itemname(uint32_t vnum) { return fmt::format("|Hitemname:{0}|h|h", vnum); }

std::string mobname(uint32_t vnum) { return fmt::format("|Hmobname:{0}|h|h", vnum); }

bool GetTextTag(std::string_view str, TextTag &tag)
{
    if (str.length() < 2 || str[0] != '|')
        return false;

    tag.length = 2;

    switch (str[1])
    {
    case 'c': {
        if (str.length() < 10)
            return false;

        tag.type = TEXT_TAG_COLOR;
        tag.length += 8;
        tag.content = str.substr(2, 8);
        break;
    }

    case '|': {
        tag.type = TEXT_TAG_TAG;
        break;
    }

    case 'r': {
        tag.type = TEXT_TAG_RESTORE_COLOR;
        break;
    }

    case 'H': {
        tag.type = TEXT_TAG_HYPERLINK_START;

        const auto end = str.find("|h");
        if (end == std::string_view::npos)
            return false;

        // We need to skip the terminal |h
        tag.length = end + 2;
        tag.content = str.substr(2, end - 2);
        break;
    }

    case 'h': {
        tag.type = TEXT_TAG_HYPERLINK_END;
        break;
    }

    default:
        return false;
    }

    return true;
}

bool GetTextTag(const std::string &str, TextTag &tag)
{
    if (str.length() < 2 || str[0] != '|')
        return false;

    tag.length = 2;

    switch (str[1])
    {
    case 'c': {
        if (str.length() < 10)
            return false;

        tag.type = TEXT_TAG_COLOR;
        tag.length += 8;
        tag.content = str.substr(2, 8);
        break;
    }

    case '|': {
        tag.type = TEXT_TAG_TAG;
        tag.content.clear();
        break;
    }

    case 'r': {
        tag.type = TEXT_TAG_RESTORE_COLOR;
        tag.content.clear();
        break;
    }

    case 'H': {
        tag.type = TEXT_TAG_HYPERLINK_START;

        const auto end = str.find("|h");
        if (end == storm::StringRef::npos)
            return false;

        // We need to skip the terminal |h
        tag.length = end + 2;
        tag.content = str.substr(2, end - 2);
        break;
    }

    case 'h': {
        tag.type = TEXT_TAG_HYPERLINK_END;
        tag.content.clear();

        break;
    }

    default:
        return false;
    }

    return true;
}

uint32_t FindTextTagEnd(const std::string &str, const TextTag &in)
{
    for (std::size_t p = 0, l = str.size(); p != l;)
    {
        TextTag tag;
        if (!GetTextTag(str.substr(p), tag))
        {
            ++p;
            continue;
        }

        p += tag.length;

        // Tags are nested... bad
        if (tag.type == TEXT_TAG_HYPERLINK_START ||
            tag.type == TEXT_TAG_COLOR)
        {
            p += FindTextTagEnd(str.substr(p), tag);
            continue;
        }

        if (in.type == TEXT_TAG_HYPERLINK_START &&
            tag.type == TEXT_TAG_HYPERLINK_END)
            return p;

        if (in.type == TEXT_TAG_COLOR &&
            tag.type == TEXT_TAG_RESTORE_COLOR)
            return p;
    }

    // Unterminated text tag
    return str.length();
}

std::pair<uint32_t, uint32_t> GetTextTagBoundary(const std::string &str,
                                                 uint32_t pos)
{
    for (std::size_t p = 0, l = str.size(); p != l;)
    {
        TextTag tag;
        if (!GetTextTag(str.substr(p), tag))
        {
            ++p;
            continue;
        }

        // Handle nested text-tags correctly
        if (tag.type == TEXT_TAG_HYPERLINK_START ||
            tag.type == TEXT_TAG_COLOR) { tag.length += FindTextTagEnd(str.substr(p + tag.length), tag); }

        if (pos >= p && pos < p + tag.length)
            return std::make_pair(p, p + tag.length);

        p += tag.length;
    }

    // Not inside a text-tag
    return std::make_pair(pos, pos);
}

storm::String StripTextTags(const std::string &str)
{
    storm::String res;

    for (std::size_t p = 0, l = str.size(); p != l;)
    {
        TextTag tag;
        if (GetTextTag(str.substr(p), tag))
        {
            p += tag.length;
            continue;
        }

        res.append(1, str[p++]);
    }

    return res;
}
}

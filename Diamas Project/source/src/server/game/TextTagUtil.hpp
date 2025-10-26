#ifndef METIN2_SERVER_GAME_TEXTTAGUTIL_HPP
#define METIN2_SERVER_GAME_TEXTTAGUTIL_HPP

#include <game/length.h>
#include <fmt/format.h>
#include <storm/String.hpp>
#include <utility>

class CHARACTER;

namespace TextTag
{
std::string emote(const std::string &emote);
std::string color(const std::string &color, const std::string &text);
std::string hyperlink(const std::string &type, const std::string &data, const std::string &text);
std::string itemname(uint32_t vnum);
std::string mobname(uint32_t vnum);

enum
{
    TEXT_TAG_PLAIN,
    TEXT_TAG_TAG,
    // ||
    TEXT_TAG_COLOR,
    // |cffffffff
    TEXT_TAG_HYPERLINK_START,
    // |H
    TEXT_TAG_HYPERLINK_END,
    // |h ex) |Hitem:1234:1:1:1|h
    TEXT_TAG_RESTORE_COLOR,
};

struct TextTag
{
    uint32_t type;

    uint32_t length;

    // For colors: hex value
    // For hyperlinks: item:1234...
    std::string content;
};

bool GetTextTag(const std::string &str, TextTag &tag);
bool GetTextTag(std::string_view str, TextTag &tag);

uint32_t FindTextTagEnd(const std::string &str, const TextTag &in);

// If |pos| is inside a text-tag, the result pair contains the text-tag's
// boundary positions, that is [first, second).
//
// Otherwise this function returns |pos| twice.
std::pair<uint32_t, uint32_t> GetTextTagBoundary(const std::string &str,
                                                 uint32_t pos);
// Returns a string without text tags
storm::String StripTextTags(const std::string &str);
}

#endif /* METIN2_SERVER_GAME_TEXTTAGUTIL_HPP */

#include <random>
#include <boost/math/constants/constants.hpp>
#include <storm/io/StreamUtil.hpp>
#include <storm/io/File.hpp>
#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp>

static time_t global_time_gap = 0;

time_t get_global_time() { return time(nullptr) + global_time_gap; }

void set_global_time(time_t t) { global_time_gap = t - time(nullptr); }

void skip_spaces(const char **string) { for (; **string != '\0' && isspace(**string); ++(*string)); }

const char *one_argument(const char *argument, char *first_arg, size_t first_size)
{
    char mark = false;
    size_t first_len = 0;

    if (!argument || 0 == first_size)
    {
        SPDLOG_ERROR("one_argument received a NULL pointer!");
        *first_arg = '\0';
        return nullptr;
    }

    --first_size;

    skip_spaces(&argument);

    while (*argument && first_len < first_size)
    {
        if (*argument == '\"')
        {
            mark = !mark;
            ++argument;
            continue;
        }

        if (!mark && static_cast<uint8_t>(isspace(*argument)))
            break;

        *(first_arg++) = *argument;
        ++argument;
        ++first_len;
    }

    *first_arg = '\0';

    skip_spaces(&argument);
    return (argument);
}

const char *two_arguments(const char *argument, char *first_arg, size_t first_size, char *second_arg,
                          size_t second_size)
{
    return (one_argument(one_argument(argument, first_arg, first_size), second_arg, second_size));
}

const char *three_arguments(const char *argument, char *first_arg, size_t first_size, char *second_arg,
                            size_t second_size, char *third_arg, size_t third_size)
{
    return (one_argument(one_argument(one_argument(argument, first_arg, first_size), second_arg, second_size),
                         third_arg, third_size));
}

void split_argument(const char *argument, std::vector<std::string> & vecArgs)
{
	std::string arg = argument;
	boost::split(vecArgs, arg, boost::is_any_of(" "), boost::token_compress_on);
}

const char *first_cmd(const char *argument, char *first_arg, size_t first_arg_size, size_t *first_arg_len_result)
{
    size_t cur_len = 0;
    skip_spaces(&argument);

    first_arg_size -= 1;

    while (*argument && !isspace(static_cast<uint8_t>(*argument)) && cur_len < first_arg_size)
    {
        *(first_arg++) = LOWER(*argument);
        ++argument;
        ++cur_len;
    }

    *first_arg_len_result = cur_len;
    *first_arg = '\0';
    return (argument);
}

int parse_time_str(const char *str)
{
    int tmp = 0;
    int secs = 0;

    while (*str != 0)
    {
        switch (*str)
        {
        case 'm':
        case 'M':
            secs += tmp * 60;
            tmp = 0;
            break;

        case 'h':
        case 'H':
            secs += tmp * 3600;
            tmp = 0;
            break;

        case 'd':
        case 'D':
            secs += tmp * 86400;
            tmp = 0;
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            tmp *= 10;
            tmp += (*str) - '0';
            break;

        case 's':
        case 'S':
            secs += tmp;
            tmp = 0;
            break;
        default:
            return -1;
        }
        ++str;
    }

    return secs + tmp;
}

bool WildCaseCmp(const char *w, const char *s)
{
    for (;;)
    {
        switch (*w)
        {
        case '*':
            if (!w[1])
                return true;
            {
                uint sLength = strlen(s);
                for (size_t i = 0; i <= sLength; ++i)
                {
                    if (true == WildCaseCmp(w + 1, s + i))
                        return true;
                }
            }
            return false;

        case '?':
            if (!*s)
                return false;

            ++w;
            ++s;
            break;

        default:
            if (*w != *s)
            {
                if (tolower(*w) != tolower(*s))
                    return false;
            }

            if (!*w)
                return true;

            ++w;
            ++s;
            break;
        }
    }

    return false;
}

bool CopyStringSafe(char *dst, const std::string &src, uint32_t size)
{
    if (size != 0)
    {
        auto cnt = std::min<uint32_t>(src.length(), size - 1);
        std::memcpy(dst, src.data(), cnt);
        dst[cnt] = '\0';
        return cnt != size - 1;
    }
    else
    {
        dst[0] = '\0';
        return true;
    }
}

size_t str_lower(const char *src, char *dest, size_t dest_size)
{
    size_t len = 0;

    if (!dest || dest_size == 0)
        return len;

    if (!src)
    {
        *dest = '\0';
        return len;
    }

    // \0 �ڸ� Ȯ��
    --dest_size;

    while (*src && len < dest_size)
    {
        *dest = LOWER(*src); // LOWER ��ũ�ο��� ++�� --�ϸ� �ȵ�!!

        ++src;
        ++dest;
        ++len;
    }

    *dest = '\0';
    return len;
}

#pragma pack(push)
#pragma pack(1)
enum TgaTypes
{
    TGA_NODATA = 0,
    TGA_INDEXED = 1,
    TGA_RGB = 2,
    TGA_GRAYSCALE = 3,
    TGA_INDEXED_RLE = 9,
    TGA_RGB_RLE = 10,
    TGA_GRAYSCALE_RLE = 11
};

struct TargaHeader
{
    struct ColorMapSpecification
    {
        uint16_t firstEntryIndex;
        uint16_t length;
        uint8_t entrySize;
    };

    struct Specification
    {
        uint16_t xOrigin;
        uint16_t yOrigin;
        uint16_t width;
        uint16_t height;
        uint8_t bpp;
        uint8_t imageDesc;
    };

    BOOST_FORCEINLINE bool IsUpDownOriented() const { return !!(spec.imageDesc & 20); }

    BOOST_FORCEINLINE bool IsLeftOriented() const { return !(spec.imageDesc & 10); }

    BOOST_FORCEINLINE uint8_t GetAlpha() const { return spec.imageDesc & 0x0F; }

    uint8_t idLength;
    uint8_t colorMapType;
    uint8_t imageTypeCode;
    ColorMapSpecification colorMapSpec;
    Specification spec;
};
#pragma pack(pop)

bool SaveTga(const std::string_view &filename,
             uint16_t width, uint16_t height,
             const uint32_t *pixels)
{
    TargaHeader header = {};
    header.imageTypeCode = TGA_RGB;
    header.spec.width = width;
    header.spec.height = height;
    header.spec.bpp = 32;         // pixel32
    header.spec.imageDesc = 0x20; // origin is top-left

    storm::File output;
    bsys::error_code ec;
    output.Open(filename, ec,
                storm::AccessMode::kWrite,
                storm::CreationDisposition::kCreateAlways,
                storm::ShareMode::kNone,
                storm::UsageHint::kSequential);

    if (ec)
    {
        SPDLOG_ERROR("Failed to open output file '{0}' with '{1}'",
                     filename, ec);
        return 1;
    }

    storm::WriteExact(output, &header, sizeof(header), ec);
    storm::WriteExact(output, pixels, width * height * sizeof(uint32_t), ec);
    return !ec;
}

int GetSecondsTillNextMonth()
{
    time_t iTime;
    time(&iTime);
    struct tm endTime = *localtime(&iTime);

    int iCurrentMonth = endTime.tm_mon;

    endTime.tm_hour = 0;
    endTime.tm_min = 0;
    endTime.tm_sec = 0;
    endTime.tm_mday = 1;

    if (iCurrentMonth == 12)
    {
        endTime.tm_mon = 0;
        endTime.tm_year = endTime.tm_year + 1;
    }
    else { endTime.tm_mon = iCurrentMonth + 1; }

    int seconds = difftime(mktime(&endTime), iTime);

    return seconds;
}

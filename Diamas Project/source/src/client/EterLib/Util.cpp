#include "StdAfx.h"
#include <pak/Vfs.hpp>
#include <storm/io/File.hpp>
#include <storm/io/Format.hpp>
#include <storm/io/StreamUtil.hpp>

#include <storm/StringUtil.hpp>

#include "TextFileLoader.h"
#include "../EterBase/MappedFile.h"

#include <storm/io/View.hpp>
#include <storm/math/AngleUtil.hpp>

static UINT Get_TexCoord_Size_From_FVF(uint32_t FVF, int tex_num)
{
    return (((((FVF) >> (16 + (2 * (tex_num)))) + 1) & 0x03) + 1);
}

void PrintfTabs(storm::File &File, int iTabCount, const char *c_szString, ...)
{
    va_list args;
    va_start(args, c_szString);

    static char szBuf[1024];
    _vsnprintf(szBuf, sizeof(szBuf), c_szString, args);
    va_end(args);

    bsys::error_code ec;
    for (int i = 0; i < iTabCount; ++i)
        storm::WriteString(File, ec, "    ");

    storm::WriteString(File, ec, "{0}", szBuf);
}

void PrintfTabs(FILE *File, int iTabCount, const char *c_szString, ...)
{
    va_list args;
    va_start(args, c_szString);

    static char szBuf[1024];
    _vsnprintf(szBuf, sizeof(szBuf), c_szString, args);
    va_end(args);

    for (int i = 0; i < iTabCount; ++i)
        fprintf(File, "    ");

    fprintf(File, szBuf);
}

bool LoadTextData(const char *c_szFileName, CTokenMap &rstTokenMap)
{
    auto fp = GetVfs().Open(c_szFileName, kVfsOpenFullyBuffered);
    if (!fp)
        return false;

	const auto size = fp->GetSize();

    storm::View data(storm::GetDefaultAllocator());
	fp->GetView(0, data, size);

    CMemoryTextFileLoader textFileLoader;
    CTokenVector stTokenVector;

    textFileLoader.Bind(std::string_view(reinterpret_cast<const char *>(data.GetData()), size));

    for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
    {
        if (!textFileLoader.SplitLine(i, &stTokenVector))
            continue;

        if (2 != stTokenVector.size())
            return false;

        stl_lowers(stTokenVector[0]);
        stl_lowers(stTokenVector[1]);

        rstTokenMap[stTokenVector[0]] = stTokenVector[1];
    }

    return true;
}

bool LoadMultipleTextData(const char *c_szFileName, CTokenVectorMap &rstTokenVectorMap)
{
    auto fp = GetVfs().Open(c_szFileName, kVfsOpenFullyBuffered);
    if (!fp)
        return false;

	const auto size = fp->GetSize();

    storm::View data(storm::GetDefaultAllocator());
	fp->GetView(0, data, size);

    CMemoryTextFileLoader textFileLoader;
    CTokenVector stTokenVector;

    textFileLoader.Bind(std::string_view(reinterpret_cast<const char *>(data.GetData()), size));

    uint32_t i;

    for (i = 0; i < textFileLoader.GetLineCount(); ++i)
    {
        if (!textFileLoader.SplitLine(i, &stTokenVector))
            continue;

        stl_lowers(stTokenVector[0]);

        // Start or End
        if (0 == stTokenVector[0].compare("start"))
        {
            CTokenVector stSubTokenVector;

            stl_lowers(stTokenVector[1]);
            std::string key = stTokenVector[1];
            stTokenVector.clear();

            for (i = i + 1; i < textFileLoader.GetLineCount(); ++i)
            {
                if (!textFileLoader.SplitLine(i, &stSubTokenVector))
                    continue;

                stl_lowers(stSubTokenVector[0]);

                if (0 == stSubTokenVector[0].compare("end"))
                {
                    break;
                }

                for (uint32_t j = 0; j < stSubTokenVector.size(); ++j)
                {
                    stTokenVector.push_back(stSubTokenVector[j]);
                }
            }

            rstTokenVectorMap.insert(CTokenVectorMap::value_type(key, stTokenVector));
        }
        else
        {
            std::string key = stTokenVector[0];
            stTokenVector.erase(stTokenVector.begin());
            rstTokenVectorMap.insert(CTokenVectorMap::value_type(key, stTokenVector));
        }
    }

    return true;
}

Vector3 TokenToVector(CTokenVector &rVector)
{
    if (3 != rVector.size())
    {
        assert(!"Size of token vector which will be converted to vector is not 3");
        return Vector3(0.0f, 0.0f, 0.0f);
    }
    Vector3 v;
    storm::ParseNumber(rVector[0], v.x);
    storm::ParseNumber(rVector[1], v.y);
    storm::ParseNumber(rVector[2], v.z);

    return v;
}

DirectX::SimpleMath::Color TokenToColor(CTokenVector &rVector)
{
    if (4 != rVector.size())
    {
        assert(!"Size of token vector which will be converted to color is not 4");
        return DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f);
    }
    DirectX::SimpleMath::Color v;
    storm::ParseNumber(rVector[0], v.x);
    storm::ParseNumber(rVector[1], v.y);
    storm::ParseNumber(rVector[2], v.z);
    storm::ParseNumber(rVector[3], v.w);

    return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::pair<float, float> GetKeyMovementDirection(bool isLeft, bool isRight, bool isUp, bool isDown)
{
    float x = 0.0f;
    float y = 0.0f;

    if (isLeft)
        x = 0.0f;
    else if (isRight)
        x = 1.0f;
    else
        x = 0.5f;

    if (isUp)
        y = 0.0f;
    else if (isDown)
        y = 1.0f;
    else
        y = 0.5f;

    return std::make_pair(x, y);
}

float GetKeyMovementRotation(float x, float y, float w, float h)
{
    const auto radAngle = std::atan2(x * w - w / 2.0f, -(y * h - h / 2.0f));
    const auto degAngle = storm::DegreesFromRadians(radAngle);
    return 360.0f - storm::SignedDegreeToUnsignedDegree(degAngle);
}

#include "Stdafx.h"
#include "Type.h"
#include "SoundManager.h"

#include "../EterLib/TextFileLoader.h"
#include "../EterLib/Util.h"

bool LoadSoundInformationPiece(const char *c_szFileName,
                               TSoundDataVector &rSoundDataVector,
                               const char *c_szPathHeader)
{
    CTextFileLoader *pkTextFileLoader = CTextFileLoader::Cache(c_szFileName);
    if (!pkTextFileLoader)
        return false;

    CTextFileLoader &rkTextFileLoader = *pkTextFileLoader;
    if (rkTextFileLoader.IsEmpty())
        return false;

    rkTextFileLoader.SetTop();

    int iCount;
    if (!rkTextFileLoader.GetTokenInteger("sounddatacount", &iCount))
    {
        SPDLOG_ERROR("{0}: no SoundDataCount", c_szFileName);
        return false;
    }

    rSoundDataVector.clear();
    rSoundDataVector.resize(iCount);

    char szSoundDataHeader[32 + 1];
    for (uint32_t i = 0; i < rSoundDataVector.size(); ++i)
    {
        _snprintf(szSoundDataHeader, sizeof(szSoundDataHeader), "sounddata%02d", i);
        CTokenVector *pTokenVector;
        if (!rkTextFileLoader.GetTokenVector(szSoundDataHeader, &pTokenVector))
        {
            SPDLOG_ERROR("{0}: no {1}", c_szFileName, szSoundDataHeader);
            return false;
        }

        if (2 != pTokenVector->size())
        {
            SPDLOG_ERROR("{0}: {1} has wrong size {2}", c_szFileName,
                          szSoundDataHeader, pTokenVector->size());
            return false;
        }

        rSoundDataVector[i].fTime = (float)atof(pTokenVector->at(0).c_str());
        if (c_szPathHeader)
        {
            rSoundDataVector[i].strSoundFileName = c_szPathHeader;
            rSoundDataVector[i].strSoundFileName += pTokenVector->at(1).c_str();
        }
        else
        {
            rSoundDataVector[i].strSoundFileName = pTokenVector->at(1).c_str();
        }
    }

    return true;
}

bool SaveSoundInformationPiece(const char *c_szFileName, TSoundDataVector &rSoundDataVector)
{
    /*storm::String realFilename;
    GetVfs().GetPathTranslator().Translate(c_szFileName, realFilename);

    if (rSoundDataVector.empty()) // 데이터가 없으면 성공으로 간주
    {
        ::DeleteFileA(realFilename.c_str());
        return true;
    }

    storm::File File;

    bsys::error_code ec;
    File.Open(realFilename, ec,
              storm::AccessMode::kWrite,
              storm::CreationDisposition::kCreateAlways,
              storm::ShareMode::kNone,
              storm::UsageHint::kSequential);

    if (ec) {
        SPDLOG_ERROR("Failed to open {0} for writing with {1}",
                  realFilename, ec);
        return false;
    }

    PrintfTabs(File, 0, "ScriptType        CharacterSoundInformation\n");
    PrintfTabs(File, 0, "\n");

    PrintfTabs(File, 0, "SoundDataCount    %d\n", rSoundDataVector.size());

    for (uint32_t i = 0; i < rSoundDataVector.size(); ++i)
    {
        const auto & rSoundData = rSoundDataVector[i];
        PrintfTabs(File, 0, "SoundData%02d       %f \"%s\"\n",
                   i, rSoundData.fTime,
                   rSoundData.strSoundFileName.c_str());
    }

    */
    return true;
}

void DataToInstance(const TSoundDataVector &c_rSoundDataVector, TSoundInstanceVector *pSoundInstanceVector)
{
    if (c_rSoundDataVector.empty())
        return;

    const float c_fFrameTime = 1.0f / 60.0f;

    pSoundInstanceVector->clear();
    pSoundInstanceVector->resize(c_rSoundDataVector.size());
    for (uint32_t i = 0; i < c_rSoundDataVector.size(); ++i)
    {
        const TSoundData &c_rSoundData = c_rSoundDataVector[i];
        TSoundInstance &rSoundInstance = pSoundInstanceVector->at(i);

        rSoundInstance.dwFrame = (uint32_t)(c_rSoundData.fTime / c_fFrameTime);
        rSoundInstance.strSoundFileName = c_rSoundData.strSoundFileName;
    }
}

void UpdateSoundInstance(uint32_t frame, const TSoundInstanceVector &sounds,
                         float fx, float fy, float fz, bool bCheckFrequency)
{
    auto &snd = CSoundManager::Instance();
    for (const auto &instance : sounds)
    {
        if (instance.dwFrame != frame)
            continue;

        snd.PlayCharacterSound3D(fx, fy, fz,
                                 instance.strSoundFileName,
                                 bCheckFrequency);
    }
}

void UpdateSoundInstance(uint32_t frame, const TSoundInstanceVector &sounds)
{
    for (const auto &instance : sounds)
    {
        if (instance.dwFrame == frame)
            CSoundManager::Instance().PlaySound2D(instance.strSoundFileName);
    }
}

#ifndef METIN2_CLIENT_MILESLIB_TYPE_HPP
#define METIN2_CLIENT_MILESLIB_TYPE_HPP

#pragma once

#include <vector>

typedef struct SSoundData
{
    float fTime;
    std::string strSoundFileName;
} TSoundData;

typedef struct SSoundInstance
{
    uint32_t dwFrame;
    std::string strSoundFileName;
} TSoundInstance;

typedef std::vector<TSoundData> TSoundDataVector;
typedef std::vector<TSoundInstance> TSoundInstanceVector;

bool LoadSoundInformationPiece(const char *c_szFileName,
                               TSoundDataVector &rSoundDataVector,
                               const char *c_szPathHeader = NULL);
bool SaveSoundInformationPiece(const char *c_szFileName,
                               TSoundDataVector &rSoundDataVector);

void DataToInstance(const TSoundDataVector &c_rSoundDataVector,
                    TSoundInstanceVector *pSoundInstanceVector);

void UpdateSoundInstance(uint32_t frame, const TSoundInstanceVector &sounds,
                         float fx, float fy, float fz, bool bCheckFrequency);

void UpdateSoundInstance(uint32_t frame, const TSoundInstanceVector &sounds);

#endif

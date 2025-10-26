#ifndef METIN2_CLIENT_MAIN_PYTHONCUBE_H
#define METIN2_CLIENT_MAIN_PYTHONCUBE_H

#include <Config.hpp>

#include <game/CubeTypes.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CPythonCube : public CSingleton<CPythonCube>
{
public:
    bool Initialize();

    CubeProto *GetCubeDataByVnum(GenericVnum vnum);
    const std::vector<CubeProto> &GetCubeProto() const;

private:
    std::unordered_map<uint32_t, CubeProto> m_cubeProtoByVnum;
    std::vector<CubeProto> m_cubeProto;
};

#endif

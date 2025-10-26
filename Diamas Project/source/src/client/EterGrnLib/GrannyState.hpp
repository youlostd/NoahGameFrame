#ifndef METIN2_CLIENT_ETERGRNLIB_GRANNYSTATE_HPP
#define METIN2_CLIENT_ETERGRNLIB_GRANNYSTATE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif
#include <unordered_map>

#include <base/Singleton.hpp>
#include "GrannyLibrary.hpp"

#include "ModelShader.h"

class GrannyState : public CSingleton<GrannyState>
{
public:

    GrannyState() = default;
    ~GrannyState();

    bool InitGrannyShader();

    [[nodiscard]] CGrannyModelShader* GetModelShader() { return &m_shader; }

    granny_mesh_deformer *GetMeshDeformerByVertexType(granny_data_type_definition *type);

    // Register a new VertexType and create a mesh deformer for it
    void RegisterVertexType(granny_data_type_definition *type);

private:
    std::unordered_map<granny_data_type_definition *, granny_mesh_deformer *> m_meshDeformers;
    const granny_data_type_definition *m_outputVertexType = GrannyPNT332VertexType;
    CGrannyModelShader m_shader;
};

#endif

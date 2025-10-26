#include "GrannyState.hpp"
#include "ModelShader.h"

GrannyState::~GrannyState()
{
    for (auto elem : m_meshDeformers)
        GrannyFreeMeshDeformer(elem.second);
}

bool GrannyState::InitGrannyShader()
{
    return m_shader.Init();
}

granny_mesh_deformer *GrannyState::GetMeshDeformerByVertexType(granny_data_type_definition *type)
{
    if (auto it = m_meshDeformers.find(type); it != m_meshDeformers.end())
        return it->second;

    RegisterVertexType(type);

    if (auto it = m_meshDeformers.find(type); it != m_meshDeformers.end())
        return it->second;

    return nullptr;
}

void GrannyState::RegisterVertexType(granny_data_type_definition *type)
{
    if (m_meshDeformers.find(type) != m_meshDeformers.end())
        return;

    const auto meshDeformer = GrannyNewMeshDeformer(type,
                                                    m_outputVertexType,
                                                    GrannyDeformPositionNormal,
                                                    GrannyDontAllowUncopiedTail);

    m_meshDeformers.emplace(type, meshDeformer);
}

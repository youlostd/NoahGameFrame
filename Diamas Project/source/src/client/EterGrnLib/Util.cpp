#include "Util.h"
#include "../eterLib/Util.h"
#include "StdAfx.h"

granny_data_type_definition PNT3322VertexType[] =
{
    {GrannyReal32Member, GrannyVertexPositionName, 0, 3},
    {GrannyReal32Member, GrannyVertexNormalName, 0, 3},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "0", 0, 2},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "1", 0, 2},

    {GrannyEndMember},
};

granny_data_type_definition GrannyPNGBT333322VertexType[7] = {
    {GrannyReal32Member, GrannyVertexPositionName, 0, 3},
    {GrannyReal32Member, GrannyVertexNormalName, 0, 3},
    {GrannyReal32Member, GrannyVertexTangentName, 0, 3},
    {GrannyReal32Member, GrannyVertexBinormalName, 0, 3},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "0", 0, 2},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "1", 0, 2},
    {GrannyEndMember}
};

granny_data_type_definition GrannyPWNGBT3433322VertexType[9] =
{
    {GrannyReal32Member, GrannyVertexPositionName, 0, 3},
    {GrannyNormalUInt8Member, GrannyVertexBoneWeightsName, 0, 4},
    {GrannyUInt8Member, GrannyVertexBoneIndicesName, 0, 4},
    {GrannyReal32Member, GrannyVertexNormalName, 0, 3},
    {GrannyReal32Member, GrannyVertexTangentName, 0, 3},
    {GrannyReal32Member, GrannyVertexBinormalName, 0, 3},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "0", 0, 2},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "1", 0, 2},

    {GrannyEndMember},
};

granny_data_type_definition GrannyPNGBT33332222VertexType[8] = {
    {GrannyReal32Member, GrannyVertexPositionName, 0, 3},
    {GrannyReal32Member, GrannyVertexNormalName, 0, 3},
    {GrannyReal32Member, GrannyVertexTangentName, 0, 3},
    {GrannyReal32Member, GrannyVertexBinormalName, 0, 3},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "0", 0, 2},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "1", 0, 2},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "2", 0, 2},

    {GrannyEndMember}
};
granny_data_type_definition GrannyPNT33222VertexType[8] = {
    {GrannyReal32Member, GrannyVertexPositionName, 0, 3},
    {GrannyReal32Member, GrannyVertexNormalName, 0, 3},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "0", 0, 2},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "1", 0, 2},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "2", 0, 2},

    {GrannyEndMember}
};
std::string GetGrannyTypeString(const granny_data_type_definition *type)
{
    assert(type && "Type is null");

    std::string s;
    for (; type->Type != GrannyEndMember; ++type)
    {
        s.append(type->Name ? type->Name : "<no name>");
        s.append(1, '[');
        s.append(std::to_string(type->ArrayWidth));
        s.append(1, ']');
        s.append(1, ',');
    }

    return s;
}


bool FindBestRigidVertexFormat(const granny_model *model, const granny_data_type_definition *&type, bool& isMultiTexCoord)
{
    assert(model != nullptr && "Parameter check");

    // This is our default type - suitable for most models/meshes.
    type = GrannyPNGBT33332VertexType;

    for (int i = 0; i < model->MeshBindingCount; ++i)
    {
        auto mesh = model->MeshBindings[i].Mesh;

        // Ignore deforming meshes
        if (!GrannyMeshIsRigid(mesh))
            continue;

        auto currentType = GrannyGetMeshVertexType(mesh);
        if (!currentType)
        {
            SPDLOG_ERROR("Mesh {0} has no vertex type", mesh->Name);
            continue;
        }

        if (GrannyDataTypesAreEqual(PNT3322VertexType, currentType))
        {
            type = currentType;
            SPDLOG_INFO("Mesh {0} uses PNT3322VertexType", mesh->Name);
            isMultiTexCoord = true;

            break;
        }

        // This type has an additional texture position
        // It is therefore preferred, since we'd otherwise lose
        // this information.
        if (GrannyDataTypesAreEqual(GrannyPNGBT333322VertexType, currentType))
        {
            type = currentType;
            SPDLOG_INFO("Mesh {0} uses PNGBT333322", mesh->Name);
            isMultiTexCoord = true;

            break;
        }

        if (GrannyDataTypesAreEqual(GrannyPNGBT33332222VertexType, currentType))
        {
            type = currentType;
            SPDLOG_INFO("Mesh {0} uses PNGBT33332222", mesh->Name);
            isMultiTexCoord = true;
            break;
        }

        if (GrannyDataTypesAreEqual(GrannyPNT33222VertexType, currentType))
        {
            type = currentType;
            SPDLOG_INFO("Mesh {0} uses PNT33222VertexType", mesh->Name);
            isMultiTexCoord = true;
            break;
        }
        
        // We only support these two types, so if this fails, we encountered an
        // unknown one.
        if (!GrannyDataTypesAreEqual(GrannyPNT332VertexType, currentType))
        {
            SPDLOG_DEBUG("Ignoring unknown vertex format type: {0}", GetGrannyTypeString(currentType).c_str());
        }
    }

    return true;
}

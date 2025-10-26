#include "../EterLib/Engine.hpp"
#include "GrannyState.hpp"
#include "Model.h"
#include "ModelInstance.h"
#include "ModelShader.h"
#include "StdAfx.h"

#include "../Eterlib/StateManager.h"
#include "base/Remotery.h"

void CGrannyModelInstance::DeformNoSkin(const Matrix *c_pWorldMatrix, float allowedError)
{
    if (IsEmpty())
        return;

    UpdateWorldPose(allowedError, c_pWorldMatrix);
    UpdateWorldMatrices(c_pWorldMatrix);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//// Render
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// With One Texture
void CGrannyModelInstance::RenderWithOneTexture()
{
    // FIXME : Deform, Render, BlendRender를 묶어 상위에서 걸러주는 것이 더 나을 듯 - [levites]
    if (IsEmpty())
        return;
    rmt_ScopedCPUSample(InstRenderWithOneTexture, RMTSF_Aggregate);

    RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT);
    RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
}

void CGrannyModelInstance::BlendRenderWithOneTexture()
{
    if (IsEmpty())
        return;

    RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT);
    RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
}

// With Two Texture
void CGrannyModelInstance::RenderWithTwoTexture()
{
    // FIXME: Better to filter Deform, Render, BlendRender from the top-[levites]
    if (IsEmpty())
        return;

    RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT);
    RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
}

void CGrannyModelInstance::BlendRenderWithTwoTexture()
{
    if (IsEmpty())
        return;

    RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT);
    RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
}

void CGrannyModelInstance::RenderWithoutTexture()
{
    if (IsEmpty())
        return;

    // STATEMANAGER.SetTexture(0, NULL);
    // STATEMANAGER.SetTexture(1, NULL);

    RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT);
    RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT);

    RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
    RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//// Render Mesh List
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// With One Texture
void CGrannyModelInstance::RenderMeshNodeListWithOneTexture(CGrannyMesh::EType eMeshType,
                                                            CGrannyMaterial::EType eMtrlType)
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** RenderMeshNodeListWithOneTexture **");

    assert(m_model != NULL);

    auto *shader = GrannyState::instance().GetModelShader();
    if (!shader)
        return;

    auto technique = shader->GetShader()->GetTechniqueByIndex(eMeshType);

    if (eMeshType == CGrannyMesh::TYPE_RIGID)
    {
        shader->SetRigidVertexFormat();
    }
    else
    {

        shader->SetSkinningVertexFormat();
    }

    const CGrannyModel::TMeshNode *pMeshNode = m_model->GetMeshNodeList(eMeshType, eMtrlType);
    while (pMeshNode)
    {
        const CGrannyMesh *pMesh = pMeshNode->pMesh;
        const auto meshBinding = m_vct_pgrnMeshBinding[pMeshNode->iMesh];

        Engine::GetDevice().SetIndexBuffer(pMesh->GetIndexBuffer());
        Engine::GetDevice().SetVertexBuffer(pMesh->GetVertexBuffer(), 0, 0);

        if (eMeshType != CGrannyMesh::TYPE_RIGID)
        {
            int const NumMeshBones = GrannyGetMeshBindingBoneCount(meshBinding);
            shader->SetSkinningMatrices(m_meshToMatrices[pMeshNode->iMesh].data(), NumMeshBones);
        }
        else
        {
            shader->SetWorldMatrix((Matrix const &)m_meshMatrices[pMeshNode->iMesh]);
        }

        const CGrannyMesh::TTriGroupNode *pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
        int vtxCount = pMesh->GetVertexCount();
        while (pTriGroupNode)
        {
            ms_faceCount += pTriGroupNode->triCount;

            CGrannyMaterial &rkMtrl = m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);
            // rkMtrl.ApplyRenderState();
            rkMtrl.ApplyRenderState();

            shader->SetDiffuseTexture(rkMtrl.GetD3DTexture(0));

            UINT uiPass, uiPassCount;
            technique.Begin(&uiPassCount);
            technique.BeginPass(shader->GetPassIndex());
            technique.CommitChanges();
            Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos,
                                                     pTriGroupNode->triCount);
            technique.EndPass();

            technique.End();

            rkMtrl.RestoreRenderState();

            pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
        }

        pMeshNode = pMeshNode->pNextMeshNode;
    }
    Engine::GetDevice().ShaderOff();

    D3DPERF_EndEvent();
}

// With Two Texture
void CGrannyModelInstance::RenderMeshNodeListWithTwoTexture(CGrannyMesh::EType eMeshType,
                                                            CGrannyMaterial::EType eMtrlType)
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 100, 0), L"** RenderMeshNodeListWithTwoTexture **");
    assert(m_model != NULL);

  
    auto *shader = GrannyState::instance().GetModelShader();
    if (!shader)
        return;

    auto technique = shader->GetShader()->GetTechniqueByIndex(2);

        shader->SetRigidWithTwoTextureVertexFormat();


    const CGrannyModel::TMeshNode *pMeshNode = m_model->GetMeshNodeList(eMeshType, eMtrlType);
    while (pMeshNode)
    {
        const CGrannyMesh *pMesh = pMeshNode->pMesh;
        const auto meshBinding = m_vct_pgrnMeshBinding[pMeshNode->iMesh];

        Engine::GetDevice().SetIndexBuffer(pMesh->GetIndexBuffer());
        Engine::GetDevice().SetVertexBuffer(pMesh->GetVertexBuffer(), 0, 0);

        if (eMeshType != CGrannyMesh::TYPE_RIGID)
        {
            int const NumMeshBones = GrannyGetMeshBindingBoneCount(meshBinding);
            shader->SetSkinningMatrices(m_meshToMatrices[pMeshNode->iMesh].data(), NumMeshBones);
        }
        else
        {
            shader->SetWorldMatrix((Matrix const &)m_meshMatrices[pMeshNode->iMesh]);
        }

        const CGrannyMesh::TTriGroupNode *pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
        int vtxCount = pMesh->GetVertexCount();
        while (pTriGroupNode)
        {
            ms_faceCount += pTriGroupNode->triCount;

            CGrannyMaterial &rkMtrl = m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);
            // rkMtrl.ApplyRenderState();
            rkMtrl.ApplyRenderState();

            shader->SetDiffuseTexture(rkMtrl.GetD3DTexture(0));
            shader->SetOpacityTexture(rkMtrl.GetD3DTexture(1));

            UINT uiPass, uiPassCount;
            technique.Begin(&uiPassCount);
            technique.BeginPass(0);
            technique.CommitChanges();
            Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos,
                                                     pTriGroupNode->triCount);
            technique.EndPass();

            technique.End();

            rkMtrl.RestoreRenderState();

            pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
        }

        pMeshNode = pMeshNode->pNextMeshNode;
    }
    Engine::GetDevice().ShaderOff();

    D3DPERF_EndEvent();
}

void CGrannyModelInstance::RenderMeshNodeListWithThreeTexture(CGrannyMesh::EType eMeshType,
                                                              CGrannyMaterial::EType eMtrlType)
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 100, 0), L"** RenderMeshNodeListWithTwoTexture **");
    assert(m_model != NULL);
    return;

    auto *shader = GrannyState::instance().GetModelShader();
    if (!shader)
        return;

    const CGrannyModel::TMeshNode *pMeshNode = m_model->GetMeshNodeList(eMeshType, eMtrlType);
    while (pMeshNode)
    {
        const CGrannyMesh *pMesh = pMeshNode->pMesh;
        const auto meshBinding = m_vct_pgrnMeshBinding[pMeshNode->iMesh];
        uint32_t techniqueIndex = 0;

        if (shader)
        {
            if (pMesh->IsRigidMesh())
            {
                shader->SetRigidVertexFormat();
                shader->SetWorldMatrix((Matrix const &)m_meshMatrices[pMeshNode->iMesh]);
                techniqueIndex = 0;
            }
            else
            {
                int const NumMeshBones = GrannyGetMeshBindingBoneCount(meshBinding);

                shader->SetSkinningVertexFormat();

                shader->SetSkinningMatrices((Matrix const *)m_meshMatrices, NumMeshBones);
                techniqueIndex = 1;
            }
            Engine::GetDevice().SetIndexBuffer(pMesh->GetIndexBuffer());
            Engine::GetDevice().SetVertexBuffer(pMesh->GetVertexBuffer(), 0, 0);
        }

        // STATEMANAGER.SetTransform(D3DTS_WORLD, &m_meshMatrices[pMeshNode->iMesh]);

        const CGrannyMesh::TTriGroupNode *pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
        int vtxCount = pMesh->GetVertexCount();
        while (pTriGroupNode)
        {
            ms_faceCount += pTriGroupNode->triCount;

            CGrannyMaterial &rkMtrl = m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);
            rkMtrl.ApplyRenderState();

            shader->SetDiffuseTexture(const_cast<CGraphicTexture *>(rkMtrl.GetDiffuseTexture()));

            auto technique = shader->GetShader()->GetTechniqueByIndex(techniqueIndex);

            UINT uiPass, uiPassCount;
            technique.Begin(&uiPassCount);
            for (uiPass = 0; uiPass < uiPassCount; ++uiPass)
            {
                technique.BeginPass(uiPass);

                Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos,
                                                         pTriGroupNode->triCount);
                technique.EndPass();
            }
            technique.End();

            rkMtrl.RestoreRenderState();

            pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
        }

        pMeshNode = pMeshNode->pNextMeshNode;
    }
    Engine::GetDevice().ShaderOff();

    D3DPERF_EndEvent();
}

// Without Texture
void CGrannyModelInstance::RenderMeshNodeListWithoutTexture(CGrannyMesh::EType eMeshType,
                                                            CGrannyMaterial::EType eMtrlType)
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 140, 140, 0), L"** RenderMeshNodeListWithoutTexture **");

    auto *shader = GrannyState::instance().GetModelShader();
    if (!shader)
        return;

    const CGrannyModel::TMeshNode *pMeshNode = m_model->GetMeshNodeList(eMeshType, eMtrlType);
    while (pMeshNode)
    {
        const CGrannyMesh *pMesh = pMeshNode->pMesh;
        const auto meshBinding = m_vct_pgrnMeshBinding[pMeshNode->iMesh];
        uint32_t techniqueIndex = 0;

        if (shader)
        {
            if (pMesh->IsRigidMesh())
            {
                shader->SetRigidVertexFormat();
                shader->SetWorldMatrix((Matrix const &)m_meshMatrices[pMeshNode->iMesh]);
                techniqueIndex = 0;
            }
            else
            {
                int const NumMeshBones = GrannyGetMeshBindingBoneCount(meshBinding);

                shader->SetSkinningVertexFormat();

                shader->SetSkinningMatrices(m_meshToMatrices[pMeshNode->iMesh].data(), NumMeshBones);
                techniqueIndex = 1;
            }
            Engine::GetDevice().SetIndexBuffer(pMesh->GetIndexBuffer());
            Engine::GetDevice().SetVertexBuffer(pMesh->GetVertexBuffer(), 0, 0);
        }

        // STATEMANAGER.SetTransform(D3DTS_WORLD, &m_meshMatrices[pMeshNode->iMesh]);

        const CGrannyMesh::TTriGroupNode *pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
        int vtxCount = pMesh->GetVertexCount();
        while (pTriGroupNode)
        {
            ms_faceCount += pTriGroupNode->triCount;

            CGrannyMaterial &rkMtrl = m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);
            rkMtrl.ApplyRenderState();

            shader->SetDiffuseTexture((LPDIRECT3DTEXTURE9) nullptr);

            auto technique = shader->GetShader()->GetTechniqueByIndex(techniqueIndex);

            UINT uiPass, uiPassCount;
            technique.Begin(&uiPassCount);
            technique.BeginPass(shader->GetPassIndex());
            technique.CommitChanges();
            Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos,
                                                     pTriGroupNode->triCount);
            technique.EndPass();

            technique.End();

            rkMtrl.RestoreRenderState();

            pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
        }

        pMeshNode = pMeshNode->pNextMeshNode;
    }
    Engine::GetDevice().ShaderOff();

    D3DPERF_EndEvent();
}

#include "stdafx.h"
#include "SkyBox.h"

#include "../EterBase/StepTimer.h"
#include "Camera.h"
#include "Engine.hpp"
#include "GrpDevice.h"
#include "StateManager.h"
#include "ResourceManager.h"

#include "../eterBase/Timer.h"

//////////////////////////////////////////////////////////////////////////
// CSkyObjectQuad
//////////////////////////////////////////////////////////////////////////

CSkyObjectQuad::CSkyObjectQuad()
{
    // Index buffer
    m_Indices[0] = 0;
    m_Indices[1] = 2;
    m_Indices[2] = 1;
    m_Indices[3] = 3;

    for (unsigned char uci = 0; uci < 4; ++uci)
        std::memset(&m_Vertex[uci], 0, sizeof(m_Vertex[uci]));
}

void CSkyObjectQuad::Clear(uint8_t vertexIdx, const DirectX::SimpleMath::Color &color)
{
    if (vertexIdx > 3)
        return;

    m_Helper[vertexIdx].Clear(color.x, color.y, color.z, color.w);
}

void CSkyObjectQuad::SetSrcColor(uint8_t vertexIdx, const DirectX::SimpleMath::Color &color)
{
    if (vertexIdx > 3)
        return;

    m_Helper[vertexIdx].SetSrcColor(color.x, color.y, color.z, color.w);
}

void CSkyObjectQuad::SetTransition(uint8_t vertexIdx, const DirectX::SimpleMath::Color &color, uint32_t duration)
{
    if (vertexIdx > 3)
        return;

    m_Helper[vertexIdx].SetTransition(color.x, color.y, color.z, color.w, duration);
}

void CSkyObjectQuad::SetVertex(uint8_t vertexIdx, const TPDTVertex &vertex)
{
    if (vertexIdx > 3)
        return;

    m_Vertex[m_Indices[vertexIdx]] = vertex;
}

void CSkyObjectQuad::StartTransition()
{
    for (unsigned char uci = 0; uci < 4; ++uci)
        m_Helper[uci].StartTransition();
}

bool CSkyObjectQuad::Update()
{
    bool bResult = false;
    for (unsigned char uci = 0; uci < 4; ++uci)
    {
        bResult = m_Helper[uci].Update() || bResult;
        m_Vertex[m_Indices[uci]].diffuse = m_Helper[uci].GetCurColor();
    }
    return bResult;
}

void CSkyObjectQuad::Render()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CSkyObjectQuad::Render **");

    Engine::GetDevice().SetFvF(TPDTVertex::kFVF);
    Engine::GetDevice().DrawPDTVertices(D3DPT_TRIANGLESTRIP, 2, m_Vertex, 4);
    //Engine::GetDevice().DrawIndexedPrimitiveUP(D3DPT_TRIANGLESTRIP, 0, 4, 2, m_Indices, D3DFMT_INDEX16, &m_Vertex, sizeof(TPDTVertex));
    D3DPERF_EndEvent();
}

//////////////////////////////////////////////////////////////////////////
// CSkyObject
/////////////////////////////////////////////////////////////////////////
CSkyObject::CSkyObject()
    : m_dwlastTime(DX::StepTimer::instance().GetTotalMillieSeconds())
      , m_v3Position(0.0f, 0.0f, 0.0f)
      , m_fScaleX(1.0f)
      , m_fScaleY(1.0f)
      , m_fScaleZ(1.0f)
{
    m_matWorld = Matrix::Identity;
    m_matTranslation = Matrix::Identity;
    m_matTextureCloud = Matrix::Identity;

    m_fCloudPositionU = 0.0f;
    m_fCloudPositionV = 0.0f;

    m_bTransitionStarted = false;
    m_bSkyMatrixUpdated = false;
}

CSkyObject::~CSkyObject()
{
    // dtor
}

void CSkyObject::Update()
{
    Vector3 v3Eye = CCameraManager::Instance().GetCurrentCamera()->GetEye();

    if (m_v3Position == v3Eye)
        if (m_bSkyMatrixUpdated == false)
            return;

    m_v3Position = v3Eye;

    m_matWorld._41 = m_v3Position.x;
    m_matWorld._42 = m_v3Position.y;
    m_matWorld._43 = m_v3Position.z;

    m_matWorldCloud._41 = m_v3Position.x;
    m_matWorldCloud._42 = m_v3Position.y;
    m_matWorldCloud._43 = m_v3Position.z + m_fCloudHeight;

    if (m_bSkyMatrixUpdated)
        m_bSkyMatrixUpdated = false;
}

void CSkyObject::Render()
{
}

std::unique_ptr<CGraphicImageInstance> CSkyObject::GenerateTexture(std::string_view filename)
{
    if (filename.empty())
        return nullptr;

    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(filename);
    if (!r)
        return nullptr;

    auto pImageInstance = std::make_unique<CGraphicImageInstance>(filename);
    pImageInstance->SetImagePointer(r);
    return pImageInstance;
}

void CSkyObject::StartTransition()
{
}

//////////////////////////////////////////////////////////////////////////
// CSkyObject::TSkyObjectFace
//////////////////////////////////////////////////////////////////////////

CSkyObject::TSkyObjectFace::TSkyObjectFace()
{
    // ctor
}

CSkyObject::TSkyObjectFace::~TSkyObjectFace()
{
}

void CSkyObject::TSkyObjectFace::StartTransition()
{
    for (auto &quad : m_SkyObjectQuadVector)
        quad.StartTransition();
}

bool CSkyObject::TSkyObjectFace::Update()
{
    bool result = false;
    for (auto &quad : m_SkyObjectQuadVector)
        result = quad.Update() || result;

    return result;
}

void CSkyObject::TSkyObjectFace::Render()
{
    for (auto &quad : m_SkyObjectQuadVector)
        quad.Render();
}

//////////////////////////////////////////////////////////////////////////
// CSkyBox
//////////////////////////////////////////////////////////////////////////

CSkyBox::CSkyBox()
    : m_ucVirticalGradientLevelUpper(0)
      , m_ucVirticalGradientLevelLower(0)
{
    // ctor
}

CSkyBox::~CSkyBox()
{
    // dtor
}

void CSkyBox::SetSkyBoxScale(const Vector3 &c_rv3Scale)
{
    m_fScaleX = c_rv3Scale.x;
    m_fScaleY = c_rv3Scale.y;
    m_fScaleZ = c_rv3Scale.z;

    m_bSkyMatrixUpdated = true;
    m_matWorld = Matrix::CreateScale(m_fScaleX, m_fScaleY, m_fScaleZ);
}

void CSkyBox::SetGradientLevel(uint8_t byUpper, uint8_t byLower)
{
    m_ucVirticalGradientLevelUpper = byUpper;
    m_ucVirticalGradientLevelLower = byLower;
}

void CSkyBox::SetFaceTexture(std::string_view filename, int iFaceIndex)
{
    if (iFaceIndex < 0 || iFaceIndex > 5)
        return;

    auto &face = m_Faces[iFaceIndex];

    if (filename == face.texturePath)
        return;

    face.texturePath = filename;
    face.texture = GenerateTexture(filename);
}

void CSkyBox::SetCloudTexture(std::string_view filename)
{
    if (filename == m_FaceCloud.texturePath)
        return;

    m_FaceCloud.texturePath = filename;
    m_FaceCloud.texture = GenerateTexture(filename);
}

void CSkyBox::SetCloudScale(const Vector2 &c_rv2CloudScale)
{
    m_fCloudScaleX = c_rv2CloudScale.x;
    m_fCloudScaleY = c_rv2CloudScale.y;

    m_matWorldCloud = Matrix::CreateScale(m_fCloudScaleX, m_fCloudScaleY, 1.0f);
}

void CSkyBox::SetCloudHeight(float fHeight)
{
    m_fCloudHeight = fHeight;
}

void CSkyBox::SetCloudTextureScale(const Vector2 &c_rv2CloudTextureScale)
{
    m_fCloudTextureScaleX = c_rv2CloudTextureScale.x;
    m_fCloudTextureScaleY = c_rv2CloudTextureScale.y;

    m_matTextureCloud._11 = m_fCloudTextureScaleX;
    m_matTextureCloud._22 = m_fCloudTextureScaleY;
}

void CSkyBox::SetCloudScrollSpeed(const Vector2 &c_rv2CloudScrollSpeed)
{
    m_fCloudScrollSpeedU = c_rv2CloudScrollSpeed.x;
    m_fCloudScrollSpeedV = c_rv2CloudScrollSpeed.y;
}

void CSkyBox::SetSkyObjectQuadVertical(TSkyObjectQuadVector *pSkyObjectQuadVector, const Vector2 *c_pv2QuadPoints)
{
    TPDTVertex aPDTVertex;

    uint32_t dwIndex = 0;

    pSkyObjectQuadVector->clear();
    pSkyObjectQuadVector->resize(m_ucVirticalGradientLevelUpper + m_ucVirticalGradientLevelLower);

    unsigned char ucY;
    for (ucY = 0; ucY < m_ucVirticalGradientLevelUpper; ++ucY)
    {
        CSkyObjectQuad &rSkyObjectQuad = pSkyObjectQuadVector->at(dwIndex++);

        aPDTVertex.position.x = c_pv2QuadPoints[0].x;
        aPDTVertex.position.y = c_pv2QuadPoints[0].y;
        aPDTVertex.position.z = 1.0f - (float)(ucY + 1) / (float)(m_ucVirticalGradientLevelUpper);
        aPDTVertex.texCoord.x = 0.0f;
        aPDTVertex.texCoord.y = (float)(ucY + 1) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
        rSkyObjectQuad.SetVertex(0, aPDTVertex);
        aPDTVertex.position.x = c_pv2QuadPoints[0].x;
        aPDTVertex.position.y = c_pv2QuadPoints[0].y;
        aPDTVertex.position.z = 1.0f - (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper);
        aPDTVertex.texCoord.x = 0.0f;
        aPDTVertex.texCoord.y = (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
        rSkyObjectQuad.SetVertex(1, aPDTVertex);
        aPDTVertex.position.x = c_pv2QuadPoints[1].x;
        aPDTVertex.position.y = c_pv2QuadPoints[1].y;
        aPDTVertex.position.z = 1.0f - (float)(ucY + 1) / (float)(m_ucVirticalGradientLevelUpper);
        aPDTVertex.texCoord.x = 1.0f;
        aPDTVertex.texCoord.y = (float)(ucY + 1) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
        rSkyObjectQuad.SetVertex(2, aPDTVertex);
        aPDTVertex.position.x = c_pv2QuadPoints[1].x;
        aPDTVertex.position.y = c_pv2QuadPoints[1].y;
        aPDTVertex.position.z = 1.0f - (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper);
        aPDTVertex.texCoord.x = 1.0f;
        aPDTVertex.texCoord.y = (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
        rSkyObjectQuad.SetVertex(3, aPDTVertex);
    }
    for (ucY = 0; ucY < m_ucVirticalGradientLevelLower; ++ucY)
    {
        CSkyObjectQuad &rSkyObjectQuad = pSkyObjectQuadVector->at(dwIndex++);

        aPDTVertex.position.x = c_pv2QuadPoints[0].x;
        aPDTVertex.position.y = c_pv2QuadPoints[0].y;
        aPDTVertex.position.z = -(float)(ucY + 1) / (float)(m_ucVirticalGradientLevelLower);
        aPDTVertex.texCoord.x = 0.0f;
        aPDTVertex.texCoord.y = 0.5f + (float)(ucY + 1) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
        rSkyObjectQuad.SetVertex(0, aPDTVertex);
        aPDTVertex.position.x = c_pv2QuadPoints[0].x;
        aPDTVertex.position.y = c_pv2QuadPoints[0].y;
        aPDTVertex.position.z = -(float)(ucY) / (float)(m_ucVirticalGradientLevelLower);
        aPDTVertex.texCoord.x = 0.0f;
        aPDTVertex.texCoord.y = 0.5f + (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
        rSkyObjectQuad.SetVertex(1, aPDTVertex);
        aPDTVertex.position.x = c_pv2QuadPoints[1].x;
        aPDTVertex.position.y = c_pv2QuadPoints[1].y;
        aPDTVertex.position.z = -(float)(ucY + 1) / (float)(m_ucVirticalGradientLevelLower);
        aPDTVertex.texCoord.x = 1.0f;
        aPDTVertex.texCoord.y = 0.5f + (float)(ucY + 1) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
        rSkyObjectQuad.SetVertex(2, aPDTVertex);
        aPDTVertex.position.x = c_pv2QuadPoints[1].x;
        aPDTVertex.position.y = c_pv2QuadPoints[1].y;
        aPDTVertex.position.z = -(float)(ucY) / (float)(m_ucVirticalGradientLevelLower);
        aPDTVertex.texCoord.x = 1.0f;
        aPDTVertex.texCoord.y = 0.5f + (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
        rSkyObjectQuad.SetVertex(3, aPDTVertex);
    }
}

void CSkyBox::SetSkyObjectQuadHorizon(TSkyObjectQuadVector *pSkyObjectQuadVector, const Vector3 *c_pv3QuadPoints)
{
    pSkyObjectQuadVector->clear();
    pSkyObjectQuadVector->resize(1);
    CSkyObjectQuad &rSkyObjectQuad = pSkyObjectQuadVector->at(0);

    TPDTVertex aPDTVertex;
    aPDTVertex.position = c_pv3QuadPoints[0];
    aPDTVertex.texCoord.x = 0.0f;
    aPDTVertex.texCoord.y = 1.0f;
    rSkyObjectQuad.SetVertex(0, aPDTVertex);

    aPDTVertex.position = c_pv3QuadPoints[1];
    aPDTVertex.texCoord.x = 0.0f;
    aPDTVertex.texCoord.y = 0.0f;
    rSkyObjectQuad.SetVertex(1, aPDTVertex);

    aPDTVertex.position = c_pv3QuadPoints[2];
    aPDTVertex.texCoord.x = 1.0f;
    aPDTVertex.texCoord.y = 1.0f;
    rSkyObjectQuad.SetVertex(2, aPDTVertex);

    aPDTVertex.position = c_pv3QuadPoints[3];
    aPDTVertex.texCoord.x = 1.0f;
    aPDTVertex.texCoord.y = 0.0f;
    rSkyObjectQuad.SetVertex(3, aPDTVertex);
}

void CSkyBox::Refresh()
{
    Vector3 v3QuadPoints[4];

    if (m_ucRenderMode == CSkyObject::SKY_RENDER_MODE_DEFAULT || m_ucRenderMode == CSkyObject::SKY_RENDER_MODE_DIFFUSE)
    {
        if (m_ucVirticalGradientLevelUpper + m_ucVirticalGradientLevelLower <= 0)
            return;

        Vector2 v2QuadPoints[2];

        //// Face 0: FRONT
        v2QuadPoints[0] = Vector2(1.0f, -1.0f);
        v2QuadPoints[1] = Vector2(-1.0f, -1.0f);
        SetSkyObjectQuadVertical(&m_Faces[0].m_SkyObjectQuadVector, v2QuadPoints);
        m_Faces[0].name = "front";

        //// Face 1: BACK
        v2QuadPoints[0] = Vector2(-1.0f, 1.0f);
        v2QuadPoints[1] = Vector2(1.0f, 1.0f);
        SetSkyObjectQuadVertical(&m_Faces[1].m_SkyObjectQuadVector, v2QuadPoints);
        m_Faces[1].name = "back";

        //// Face 2: LEFT
        v2QuadPoints[0] = Vector2(-1.0f, -1.0f);
        v2QuadPoints[1] = Vector2(-1.0f, 1.0f);
        SetSkyObjectQuadVertical(&m_Faces[2].m_SkyObjectQuadVector, v2QuadPoints);
        m_Faces[2].name = "left";

        //// Face 3: RIGHT
        v2QuadPoints[0] = Vector2(1.0f, 1.0f);
        v2QuadPoints[1] = Vector2(1.0f, -1.0f);
        SetSkyObjectQuadVertical(&m_Faces[3].m_SkyObjectQuadVector, v2QuadPoints);
        m_Faces[3].name = "right";

        //// Face 4: TOP
        v3QuadPoints[0] = Vector3(1.0f, 1.0f, 1.0f);
        v3QuadPoints[1] = Vector3(-1.0f, 1.0f, 1.0f);
        v3QuadPoints[2] = Vector3(1.0f, -1.0f, 1.0f);
        v3QuadPoints[3] = Vector3(-1.0f, -1.0f, 1.0f);
        SetSkyObjectQuadHorizon(&m_Faces[4].m_SkyObjectQuadVector, v3QuadPoints);
        m_Faces[4].name = "top";

        //// Face 5: BOTTOM
        v3QuadPoints[0] = Vector3(-1.0f, 1.0f, -1.0f);
        v3QuadPoints[1] = Vector3(1.0f, 1.0f, -1.0f);
        v3QuadPoints[2] = Vector3(-1.0f, -1.0f, -1.0f);
        v3QuadPoints[3] = Vector3(1.0f, -1.0f, -1.0f);
        SetSkyObjectQuadHorizon(&m_Faces[5].m_SkyObjectQuadVector, v3QuadPoints);
        m_Faces[5].name = "bottom";
    }
    else if (m_ucRenderMode == CSkyObject::SKY_RENDER_MODE_TEXTURE)
    {
        // Face 0: FRONT
        v3QuadPoints[0] = Vector3(1.0f, -1.0f, -1.0f);
        v3QuadPoints[1] = Vector3(1.0f, -1.0f, 1.0f);
        v3QuadPoints[2] = Vector3(-1.0f, -1.0f, -1.0f);
        v3QuadPoints[3] = Vector3(-1.0f, -1.0f, 1.0f);

        //UpdateSkyFaceQuadTransform(v3QuadPoints);

        SetSkyObjectQuadHorizon(&m_Faces[0].m_SkyObjectQuadVector, v3QuadPoints);
        m_Faces[0].name = "front";

        //// Face 1: BACK
        v3QuadPoints[0] = Vector3(-1.0f, 1.0f, -1.0f);
        v3QuadPoints[1] = Vector3(-1.0f, 1.0f, 1.0f);
        v3QuadPoints[2] = Vector3(1.0f, 1.0f, -1.0f);
        v3QuadPoints[3] = Vector3(1.0f, 1.0f, 1.0f);

        //UpdateSkyFaceQuadTransform(v3QuadPoints);

        SetSkyObjectQuadHorizon(&m_Faces[1].m_SkyObjectQuadVector, v3QuadPoints);
        m_Faces[1].name = "back";

        // Face 2: LEFT
        v3QuadPoints[0] = Vector3(1.0f, 1.0f, -1.0f);
        v3QuadPoints[1] = Vector3(1.0f, 1.0f, 1.0f);
        v3QuadPoints[2] = Vector3(1.0f, -1.0f, -1.0f);
        v3QuadPoints[3] = Vector3(1.0f, -1.0f, 1.0f);

        //UpdateSkyFaceQuadTransform(v3QuadPoints);

        SetSkyObjectQuadHorizon(&m_Faces[2].m_SkyObjectQuadVector, v3QuadPoints);
        m_Faces[2].name = "left";

        // Face 3: RIGHT
        v3QuadPoints[0] = Vector3(-1.0f, -1.0f, -1.0f);
        v3QuadPoints[1] = Vector3(-1.0f, -1.0f, 1.0f);
        v3QuadPoints[2] = Vector3(-1.0f, 1.0f, -1.0f);
        v3QuadPoints[3] = Vector3(-1.0f, 1.0f, 1.0f);

        //UpdateSkyFaceQuadTransform(v3QuadPoints);

        SetSkyObjectQuadHorizon(&m_Faces[3].m_SkyObjectQuadVector, v3QuadPoints);
        m_Faces[3].name = "right";

        // Face 4: TOP
        v3QuadPoints[0] = Vector3(1.0f, -1.0f, 1.0f);
        v3QuadPoints[1] = Vector3(1.0f, 1.0f, 1.0f);
        v3QuadPoints[2] = Vector3(-1.0f, -1.0f, 1.0f);
        v3QuadPoints[3] = Vector3(-1.0f, 1.0f, 1.0f);

        //UpdateSkyFaceQuadTransform(v3QuadPoints);

        SetSkyObjectQuadHorizon(&m_Faces[4].m_SkyObjectQuadVector, v3QuadPoints);
        m_Faces[4].name = "top";

        ////// Face 5: BOTTOM
        v3QuadPoints[0] = Vector3(1.0f, 1.0f, -1.0f);
        v3QuadPoints[1] = Vector3(1.0f, -1.0f, -1.0f);
        v3QuadPoints[2] = Vector3(-1.0f, 1.0f, -1.0f);
        v3QuadPoints[3] = Vector3(-1.0f, -1.0f, -1.0f);
        //UpdateSkyFaceQuadTransform(v3QuadPoints);

        SetSkyObjectQuadHorizon(&m_Faces[5].m_SkyObjectQuadVector, v3QuadPoints);
        m_Faces[5].name = "bottom";
    }

    //// Clouds..
    v3QuadPoints[0] = Vector3(1.0f, 1.0f, 0.0f);
    v3QuadPoints[1] = Vector3(-1.0f, 1.0f, 0.0f);
    v3QuadPoints[2] = Vector3(1.0f, -1.0f, 0.0f);
    v3QuadPoints[3] = Vector3(-1.0f, -1.0f, 0.0f);
    SetSkyObjectQuadHorizon(&m_FaceCloud.m_SkyObjectQuadVector, v3QuadPoints);
}

void CSkyBox::SetCloudColor(const DirectX::SimpleMath::Color &color, const DirectX::SimpleMath::Color &nextColor, uint32_t transitionTime)
{
    for (CSkyObjectQuad &quad : m_FaceCloud.m_SkyObjectQuadVector)
    {
        quad.SetSrcColor(0, color);
        quad.SetTransition(0, nextColor, transitionTime);
        quad.SetSrcColor(1, color);
        quad.SetTransition(1, nextColor, transitionTime);
        quad.SetSrcColor(2, color);
        quad.SetTransition(2, nextColor, transitionTime);
        quad.SetSrcColor(3, color);
        quad.SetTransition(3, nextColor, transitionTime);
    }
}

void CSkyBox::SetSkyColor(const TVectorGradientColor &colorVector,
                          const TVectorGradientColor &nextColorVector, long lTransitionTime)
{
    for (TSkyObjectFace &aFace : m_Faces)
    {
        uint32_t color = 0;
        for (CSkyObjectQuad &quad : aFace.m_SkyObjectQuadVector)
        {
            quad.SetSrcColor(0, colorVector[color].second);
            quad.SetTransition(0, nextColorVector[color].second, lTransitionTime);

            quad.SetSrcColor(1, colorVector[color].first);
            quad.SetTransition(1, nextColorVector[color].first, lTransitionTime);

            quad.SetSrcColor(2, colorVector[color].second);
            quad.SetTransition(2, nextColorVector[color].second, lTransitionTime);

            quad.SetSrcColor(3, colorVector[color].first);
            quad.SetTransition(3, nextColorVector[color].first, lTransitionTime);

            color++;
        }
    }

    TSkyObjectFace &aFaceTop = m_Faces[4];
    uint32_t color = 0;

    for (CSkyObjectQuad &quad : aFaceTop.m_SkyObjectQuadVector)
    {
        quad.SetSrcColor(0, colorVector[color].first);
        quad.SetTransition(0, nextColorVector[color].first, lTransitionTime);

        quad.SetSrcColor(1, colorVector[color].first);
        quad.SetTransition(1, nextColorVector[color].first, lTransitionTime);

        quad.SetSrcColor(2, colorVector[color].first);
        quad.SetTransition(2, nextColorVector[color].first, lTransitionTime);

        quad.SetSrcColor(3, colorVector[color].first);
        quad.SetTransition(3, nextColorVector[color].first, lTransitionTime);
    }

    TSkyObjectFace &aFaceBottom = m_Faces[5];
    color = colorVector.size() - 1;

    for (CSkyObjectQuad &quad : aFaceBottom.m_SkyObjectQuadVector)
    {
        quad.SetSrcColor(0, colorVector[color].second);
        quad.SetTransition(0, nextColorVector[color].second, lTransitionTime);

        quad.SetSrcColor(1, colorVector[color].second);
        quad.SetTransition(1, nextColorVector[color].second, lTransitionTime);

        quad.SetSrcColor(2, colorVector[color].second);
        quad.SetTransition(2, nextColorVector[color].second, lTransitionTime);

        quad.SetSrcColor(3, colorVector[color].second);
        quad.SetTransition(3, nextColorVector[color].second, lTransitionTime);
    }
}

void CSkyBox::StartTransition()
{
    m_bTransitionStarted = true;

    for (auto &face : m_Faces)
        face.StartTransition();

    m_FaceCloud.StartTransition();
}

void CSkyBox::Update()
{
    CSkyObject::Update();

    if (!m_bTransitionStarted)
        return;

    bool bResult = false;
    for (auto &face : m_Faces)
        bResult = face.Update() || bResult;

    bResult = m_FaceCloud.Update() || bResult;
    m_bTransitionStarted = bResult;
}

void CSkyBox::Render()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(43, 140, 140, 0), L"** CSkyBox::Render **");

    // 2004.01.25 myevan 처리를 렌더링 후반으로 옮기고, DepthTest 처리
    Engine::GetDevice().SetDepthEnable(true, false);

    STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, false);
    STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, false);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, false);

    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    STATEMANAGER.SetTexture(1, NULL);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorld);

    //Render Face
    if (m_ucRenderMode == CSkyObject::SKY_RENDER_MODE_TEXTURE)
    {
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

        for (unsigned int i = 0; i < 6; ++i)
        {
            CGraphicImageInstance *pFaceImageInstance = m_Faces[i].texture.get();
            if (!pFaceImageInstance)
                break;

            STATEMANAGER.SetTexture(0, pFaceImageInstance->GetTextureReference().GetD3DTexture());
            m_Faces[i].Render();
        }

        //STATEMANAGER.SetTexture(0, NULL);
        STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSU);
        STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSV);
    }
    else
    {
        for (auto &face : m_Faces)
            face.Render();
    }

	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
    Engine::GetDevice().SetDepthEnable(true, true);
    STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);

    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);

    D3DPERF_EndEvent();
}

void CSkyBox::RenderCloud()
{
    CGraphicImageInstance *pCloudGraphicImageInstance = m_FaceCloud.texture.get();
    if (!pCloudGraphicImageInstance)
        return;

    // 2004.01.25 myevan 처리를 렌더링 후반으로 옮기고, DepthTest 처리

    Engine::GetDevice().SetDepthEnable(true, false);


    STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, false);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, true);
    STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);

    STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

    m_matTextureCloud._31 = m_fCloudPositionU;
    m_matTextureCloud._32 = m_fCloudPositionV;

    uint64_t dwCurTime = DX::StepTimer::instance().GetTotalMillieSeconds();

    m_fCloudPositionU += m_fCloudScrollSpeedU * (float)(dwCurTime - m_dwlastTime) * 0.001f;
    if (m_fCloudPositionU >= 1.0f)
        m_fCloudPositionU = 0.0f;

    m_fCloudPositionV += m_fCloudScrollSpeedV * (float)(dwCurTime - m_dwlastTime) * 0.001f;
    if (m_fCloudPositionV >= 1.0f)
        m_fCloudPositionV = 0.0f;

    m_dwlastTime = dwCurTime;

    STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &m_matTextureCloud);

    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATEINVALPHA_ADDCOLOR);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	//D3DXMATRIX matProjCloud;
	//D3DXMatrixPerspectiveFovRH(&matProjCloud, D3DX_PI * 0.25f, 1.33333f, 50.0f, 999999.0f);
    Matrix matProjCloud = Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PI * 0.25f, 1.33333f, 50.0f, 999999.0f);

    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldCloud);
    STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matProjCloud);
    STATEMANAGER.SetTexture(0, pCloudGraphicImageInstance->GetTexturePointer()->GetD3DTexture());
    m_FaceCloud.Render();
    STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);

    STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);

	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
    Engine::GetDevice().SetDepthEnable(true, true);

    STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
}

#ifndef METIN2_CLIENT_ETERLIB_SKYBOX_HPP
#define METIN2_CLIENT_ETERLIB_SKYBOX_HPP

#pragma once

#include "GrpBase.h"
#include "GrpScreen.h"
#include "GrpImageInstance.h"
#include "ColorTransitionHelper.h"

#include <unordered_map>
#include <string>

typedef std::vector<std::pair<DirectX::SimpleMath::Color, DirectX::SimpleMath::Color>> TVectorGradientColor;

class CSkyObjectQuad
{
public:
    CSkyObjectQuad();

    void Clear(uint8_t vertexIdx, const DirectX::SimpleMath::Color &color);
    void SetSrcColor(uint8_t vertexIdx, const DirectX::SimpleMath::Color &color);
    void SetTransition(uint8_t vertexIdx, const DirectX::SimpleMath::Color &color, uint32_t duration);

    void SetVertex(uint8_t vertexIdx, const TPDTVertex &vertex);

    void StartTransition();
    bool Update();
    void Render();

private:
    TPDTVertex m_Vertex[4];
    TIndex m_Indices[4]; // 인덱스 버퍼...
    CColorTransitionHelper m_Helper[4];
};

class CSkyObject : public CScreen
{
public:
    enum
    {
        SKY_RENDER_MODE_DEFAULT,
        // = SKY_RENDER_MODE_TEXTURE
        SKY_RENDER_MODE_DIFFUSE,
        SKY_RENDER_MODE_TEXTURE,
        SKY_RENDER_MODE_MODULATE,
        SKY_RENDER_MODE_MODULATE2X,
        SKY_RENDER_MODE_MODULATE4X,
    };

    CSkyObject();
    virtual ~CSkyObject();

    virtual void Render() = 0;
    virtual void Update() = 0;
    virtual void StartTransition();

    void SetRenderMode(unsigned char ucRenderMode)
    {
        m_ucRenderMode = ucRenderMode;
    }

    bool isTransitionStarted() const
    {
        return m_bTransitionStarted;
    }

protected:
    std::unique_ptr<CGraphicImageInstance> GenerateTexture(std::string_view filename);

protected:
    //////////////////////////////////////////////////////////////////////////
    // 타입 정의

    typedef std::vector<CSkyObjectQuad> TSkyObjectQuadVector;
    typedef TSkyObjectQuadVector::iterator TSkyObjectQuadIterator;

    struct TSkyObjectFace
    {
        TSkyObjectFace();
        ~TSkyObjectFace();

        void StartTransition();
        bool Update();
        void Render();

        std::string name;
        std::string texturePath;
        std::unique_ptr<CGraphicImageInstance> texture;
        TSkyObjectQuadVector m_SkyObjectQuadVector;
    };

    // 구름...
    TSkyObjectFace m_FaceCloud; // 구름 일단 한장...
    Matrix m_matWorldCloud, m_matTranslationCloud, m_matTextureCloud;
    float m_fCloudScaleX, m_fCloudScaleY, m_fCloudHeight;
    float m_fCloudTextureScaleX, m_fCloudTextureScaleY;
    float m_fCloudScrollSpeedU, m_fCloudScrollSpeedV;
    float m_fCloudPositionU, m_fCloudPositionV;

    uint64_t m_dwlastTime;

    // Transform...
    Matrix m_matWorld, m_matTranslation;
    Vector3 m_v3Position;
    float m_fScaleX, m_fScaleY, m_fScaleZ;

    // 랜더링 관련... 임시 변수..
    unsigned char m_ucRenderMode;

    std::string m_strCurTime;
    bool m_bTransitionStarted;
    bool m_bSkyMatrixUpdated;

    CGraphicImageInstance m_CloudAlphaImageInstance;
};

class CSkyBox : public CSkyObject
{
public:
    CSkyBox();
    virtual ~CSkyBox();

    void Update();
    void Render();
    void RenderCloud();

    void Destroy();

    void SetSkyBoxScale(const Vector3 &c_rv3Scale);
    void SetGradientLevel(uint8_t byUpper, uint8_t byLower);
    void SetFaceTexture(std::string_view filename, int iFaceIndex);
    void SetCloudTexture(std::string_view filename);
    void SetCloudScale(const Vector2 &c_rv2CloudScale);
    void SetCloudHeight(float fHeight);
    void SetCloudTextureScale(const Vector2 &c_rv2CloudTextureScale);
    void SetCloudScrollSpeed(const Vector2 &c_rv2CloudScrollSpeed);
    void SetCloudColor(const DirectX::SimpleMath::Color &color, const DirectX::SimpleMath::Color &nextColor, uint32_t transitionTime);
    void Refresh();
    void SetSkyColor(const TVectorGradientColor &colorVector,
                     const TVectorGradientColor &nextColorVector,
                     long lTransitionTime);
    void StartTransition();

protected:
    void SetSkyObjectQuadVertical(TSkyObjectQuadVector *pSkyObjectQuadVector,
                                  const Vector2 *c_pv2QuadPoints);
    void SetSkyObjectQuadHorizon(TSkyObjectQuadVector *pSkyObjectQuadVector,
                                 const Vector3 *c_pv3QuadPoints);

protected:
    unsigned char m_ucVirticalGradientLevelUpper;
    unsigned char m_ucVirticalGradientLevelLower;
    TSkyObjectFace m_Faces[6];
};

#endif

#include "StdAfx.h"
#include "../eterLib/ResourceManager.h"
#include "../eterLib/StateManager.h"

#include "WeaponTrace.h"

#include "../EterBase/StepTimer.h"
#include "../eterBase/Timer.h"
#include "../EterLib/Engine.hpp"
#include "../EterLib/GrpDevice.h"

CDynamicPool<CWeaponTrace> CWeaponTrace::ms_kPool;

void CWeaponTrace::DestroySystem()
{
    ms_kPool.Destroy();
}

void CWeaponTrace::Delete(CWeaponTrace *pkWTDel)
{
    assert(pkWTDel!=NULL && "CWeaponTrace::Delete");

    pkWTDel->Clear();
    ms_kPool.Free(pkWTDel);
}

CWeaponTrace *CWeaponTrace::New()
{
    return ms_kPool.Alloc();
}

void CWeaponTrace::Update(float fReachScale)
{
    const float now = DX::StepTimer::Instance().GetTotalSeconds();
    float fElapsedTime = now - m_fLastUpdate;
    m_fLastUpdate = now;

    if (!m_pInstance)
        return;
    {
        // ÀÜ»óÀ» ³²±â´Â ½Ã°£ ¹üÀ§ ³»ÀÇ Á¡µé¸¸ À¯ÁöÇÕ´Ï´Ù.
        TTimePointList::iterator it;
        for (it = m_ShortTimePointList.begin(); it != m_ShortTimePointList.end(); ++it)
        {
            it->first += fElapsedTime;
            if (it->first > m_fLifeTime)
            {
                ++it;
                break;
            }
        }
        if (it != m_ShortTimePointList.end())
            m_ShortTimePointList.erase(it, m_ShortTimePointList.end());
        for (it = m_LongTimePointList.begin(); it != m_LongTimePointList.end(); ++it)
        {
            it->first += fElapsedTime;
            if (it->first > m_fLifeTime)
            {
                ++it;
                break;
            }
        }
        if (it != m_LongTimePointList.end())
            m_LongTimePointList.erase(it, m_LongTimePointList.end());
    }

    if (m_isPlaying && m_fz >= 0.0001f)
    {
        Matrix *pMatrix;
        if (m_pInstance->GetCompositeBoneMatrix(m_dwModelInstanceIndex, m_iBoneIndex, &pMatrix))
        {
            Matrix *pBoneMat;
            m_pInstance->GetBoneMatrix(m_dwModelInstanceIndex, m_iBoneIndex, &pBoneMat);
            Matrix mat = *pMatrix;
            auto fScale = m_pInstance->GetScale().x;

            mat._41 = pBoneMat->_41 * fScale;
            mat._42 = pBoneMat->_42 * fScale;
            mat._43 = pBoneMat->_43 * fScale;
            // 현재 위치를 추가합니다.
            Matrix matPoint;
            Matrix matTranslation = Matrix::CreateTranslation(0.0f, 0.0f, m_fLength * fReachScale * fScale);
            Matrix matRotation = Matrix::CreateRotationZ(DirectX::XMConvertToRadians(m_fRotation));

            //DirectX::SimpleMath::MatrixTranslation(&matTranslation, 0.0f, m_fLength, 0.0f);

            matPoint = /**pMatrix*/mat * matRotation;
            /*TPDTVertex PDTVertex;
            PDTVertex.position.x = m_fx + matPoint._41;
            PDTVertex.position.y = m_fy + matPoint._42;
            PDTVertex.position.z = m_fz + matPoint._43;
            PDTVertex.diffuse = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 0.1f);
            m_PDTVertexVector.push_back(PDTVertex);*/
            m_ShortTimePointList.push_front(
                TTimePoint(
                    0.0f,
                    Vector3(
                        m_fx + matPoint._41,
                        m_fy + matPoint._42,
                        m_fz + matPoint._43
                        )
                    )
                );

            matPoint = matTranslation * matPoint;
            /*PDTVertex.position.x = m_fx + matPoint._41;
            PDTVertex.position.y = m_fy + matPoint._42;
            PDTVertex.position.z = m_fz + matPoint._43;
            PDTVertex.diffuse = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 0.1f);
            m_PDTVertexVector.push_back(PDTVertex);*/
            m_LongTimePointList.push_front(
                TTimePoint(
                    0.0f,
                    Vector3(
                        m_fx + matPoint._41,
                        m_fy + matPoint._42,
                        m_fz + matPoint._43
                        )
                    )
                );
        }
    }
    //if (!BuildVertex())
    //	return;
}

bool CWeaponTrace::BuildVertex()
{
    const int max_size = 300;
    // calculate speed
    float h[max_size];
    float stk[max_size];
    int sp = 0;
    Vector3 r[max_size];

    if (m_LongTimePointList.size() <= 1)
        return false;

    //SPDLOG_DEBUG("## %f %f %f\n", m_LongTimePointList[0].second.x, m_LongTimePointList[0].second.y, m_LongTimePointList[0].second.z);

    /*m_LongTimePointList.clear();
    m_LongTimePointList.push_back(TTimePoint(0.00,Vector3(0,0,0)));
    m_LongTimePointList.push_back(TTimePoint(0.01,Vector3(0,1,0)));
    m_LongTimePointList.push_back(TTimePoint(0.04,Vector3(0,1,0)));
    m_LongTimePointList.push_back(TTimePoint(0.05,Vector3(0,0,0)));
    m_ShortTimePointList = m_LongTimePointList;

  */
    std::vector<TPDTVertex> m_ShortVertexVector, m_LongVertexVector;

    float length = std::min(m_fLifeTime, m_LongTimePointList.back().first);

    int n = m_LongTimePointList.size() - 1;
    assert(n<max_size-1);

    // cubic spline

    for (int loop = 0; loop <= 1; ++loop)
    {
        auto &Input = (loop) ? m_LongTimePointList : m_ShortTimePointList;
        std::vector<TPDTVertex> &Output = (loop) ? m_LongVertexVector : m_ShortVertexVector;
        int i;

        for (i = 0; i < n; ++i)
        {
            h[i] = Input[i + 1].first - Input[i].first;
            r[i] = (Input[i + 1].second - Input[i].second) * (3 / h[i]);
        }
        r[n] = Vector3(0.0f, 0.0f, 0.0f);
        for (i = n; i > 0; i--)
        {
            r[i] += r[i - 1];
        }

        float rate = 0.5f;
        r[0] *= 0.5f;
        stk[sp++] = rate;
        for (i = 1; i < n; i++)
        {
            r[i] -= r[i - 1];
            rate = 1 / (4 - rate);
            r[i] *= rate;
            stk[sp++] = rate;
        }
        r[n] -= r[n - 1];
        rate = 1 / (2 - rate);
        r[n] *= rate;

        for (i = n - 1; i >= 0; i--)
        {
            r[i] -= r[i + 1] * stk[--sp];
        }

        int base = 0;
        Vector3 a, b, c, d;
        Vector3 v3Tmp = Input[base + 1].second - Input[base].second;
        float timebase = 0, timenext = h[base], dt = m_fSamplingTime;
        a = Input[base].second;
        b = r[base];
        c = (v3Tmp * 3 - r[base + 1] * h[base] - r[base] * (2 * h[base]))
            * (1 / (h[base] * h[base]));
        d = (v3Tmp * -2 + (r[base + 1] + r[base]) * h[base])
            * (1 / (h[base] * h[base] * h[base]));

        for (float t = 0; t <= length; t += dt)
        {
            while (t > timenext)
            {
                timebase = timenext;
                base++;
                if (base >= n)
                    break;
                Vector3 v3Tmp = Input[base + 1].second - Input[base].second;
                a = Input[base].second;
                b = r[base];
                c = (v3Tmp * 3 - r[base + 1] * h[base] - r[base] * (2 * h[base]))
                    * (1 / (h[base] * h[base]));
                d = (v3Tmp * -2 + (r[base + 1] + r[base]) * h[base])
                    * (1 / (h[base] * h[base] * h[base]));

                timenext += h[base];
                if (loop)
                {
                    //SPDLOG_DEBUG("%f:%f %f %f\n",Input[base].first,Input[base].second.x,Input[base].second.y,Input[base].second.z);
                }
            }
            if (base > n)
                break;
            float cc = t - timebase;

            TPDTVertex v;
            //v.diffuse = DirectX::SimpleMath::Color(0.3f,0.8f,1.0f, (loop)?max(1.0f-(t/m_fLifeTime),0.0f)/2:0.0f );
            float ttt = std::min(std::max((t + Input[0].first) / m_fLifeTime, 0.0f), 1.0f);
            v.diffuse = Color(0.3f, 0.8f, 1.0f,
                                  (loop)
                                      ? std::min<float>(std::max<float>((1.0f - ttt) * (1.0f - ttt) / 2.5 - 0.1f, 0.0f),
                                                        1.0f)
                                      : 0.0f).BGRA();
            //v.diffuse = DirectX::SimpleMath::Color(0.0f,0.0f,0.0f, (loop)?min(max((1.0f-ttt)*(1.0f-ttt)-0.1f,0.0f),1.0f):0.0f );
            //v.diffuse =	0xffffffff;
            v.position = a + (b + (c + d * cc) * cc) * cc; // next position 
            v.texCoord.x = t / m_fLifeTime;
            v.texCoord.y = loop ? 0 : 1;
            Output.push_back(v);
            if (loop)
            {
                //	SPDLOG_DEBUG("%f %f %f\n", timebase,t,timenext);
                //SPDLOG_DEBUG("a:%f %f %f\nb:%f %f %f \nc:%f %f %f \nd:%f %f %f, \n",,a.x,a.y,a.z,b.x,b.y,b.z,c.x,c.y,c.z,d.x,d.y,d.z);

                //SPDLOG_DEBUG("%f %f %f\n",v.position.x,v.position.y,v.position.z);
                /*Matrix * pBoneMat;
                m_pInstance->GetBoneMatrix(m_dwModelInstanceIndex, 55, &pBoneMat);
                Vector3 vbone(m_fx+pBoneMat->_41,m_fy+pBoneMat->_42,m_fz+pBoneMat->_43);
                float len = D3DXVec3Length(&(v.position-vbone));*/
            }
        }
    }

    // build vertex

    m_PDTVertexVector.clear();

    /*
    TTimePointList::iterator lit1,lit2, sit1,sit2;
    
    lit2 = lit1 = m_LongTimePointList.begin();
    ++lit2;
    
    sit2 = sit1 = m_ShortTimePointList.begin();
    ++sit2;
    */
    std::vector<TPDTVertex>::iterator lit, sit;
    for (lit = m_LongVertexVector.begin(), sit = m_ShortVertexVector.begin();
         lit != m_LongVertexVector.end();
         ++lit, ++sit)
    {
        m_PDTVertexVector.push_back(*lit);
        m_PDTVertexVector.push_back(*sit);
        /*float len = D3DXVec3Length(&(lit->position - sit->position));
        if (len>160)
            SPDLOG_DEBUG("dist:{0}",len);*/
    }

    return true;
}

void CWeaponTrace::Render()
{
    //if (!m_isPlaying)
    //	return;
    //if (m_CurvingTraceVector.size() < 4)
    //	return;

    if (!BuildVertex())
        return;

    if (m_PDTVertexVector.size() < 4)
        return;

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CWeaponTrace::Render **");

    LPDIRECT3DTEXTURE9 lpTexture = NULL;

    STATEMANAGER.SaveTransform(D3DTS_WORLD, &Matrix::Identity);
    Engine::GetDevice().SetFvF(TPDTVertex::kFVF);
    STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000011);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

    Engine::GetDevice().SetDepthEnable(true, false);

    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, (m_bUseTexture) ? D3DTOP_SELECTARG2 : D3DTOP_SELECTARG1);
    //STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    //STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
    //STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, (m_bUseTexture) ? D3DTOP_SELECTARG2 : D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
    STATEMANAGER.SetTexture(0, lpTexture);
    STATEMANAGER.SetTexture(1, NULL);
    Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,
                                 int(m_PDTVertexVector.size() - 2),
                                 &m_PDTVertexVector[0],
                                 sizeof(TPDTVertex));

    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

    Engine::GetDevice().SetDepthEnable(true, true);


    STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);

    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);

    STATEMANAGER.RestoreTransform(D3DTS_WORLD);
    STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);

    D3DPERF_EndEvent();
}

void CWeaponTrace::UseAlpha()
{
    m_bUseTexture = false;
}

void CWeaponTrace::UseTexture()
{
    m_bUseTexture = true;
}

void CWeaponTrace::SetTexture(const char *c_szFileName)
{
    CGraphicImage::Ptr pImage = CResourceManager::Instance().LoadResource<CGraphicImage>("lot_ade10-2.tga");
    m_ImageInstance.SetImagePointer(pImage);

    //CGraphicTexture * pTexture = m_ImageInstance.GetTexturePointer();
    //m_lpTexture = pTexture->GetD3DTexture();
}

bool CWeaponTrace::SetWeaponInstance(CGraphicThingInstance *pInstance, uint32_t dwModelIndex, const char *c_szBoneName)
{
    pInstance->Update();
    pInstance->DeformNoSkin();

    Vector3 v3Min;
    Vector3 v3Max;
    if (!pInstance->GetBoundBox(dwModelIndex, &v3Min, &v3Max))
        return false;

    m_iBoneIndex = 0;
    m_dwModelInstanceIndex = dwModelIndex;

    m_pInstance = pInstance;
    Matrix *pmat;
    pInstance->GetBoneMatrix(dwModelIndex, 0, &pmat);
    Vector3 v3Bone(pmat->_41, pmat->_42, pmat->_43);

    m_fLength =
        sqrtf(
            fMAX(
                ((v3Bone - v3Min).LengthSquared()),
                ((v3Bone - v3Max).LengthSquared())
                )
            );

    return true;
}

void CWeaponTrace::SetPosition(float fx, float fy, float fz)
{
    m_fx = fx;
    m_fy = fy;
    m_fz = fz;
}

void CWeaponTrace::SetRotation(float fRotation)
{
    m_fRotation = fRotation;
}

void CWeaponTrace::SetLifeTime(float fLifeTime)
{
    m_fLifeTime = fLifeTime;
}

void CWeaponTrace::SetSamplingTime(float fSamplingTime)
{
    m_fSamplingTime = fSamplingTime;
}

void CWeaponTrace::TurnOn()
{
    m_isPlaying = TRUE;
}

void CWeaponTrace::TurnOff()
{
    m_isPlaying = FALSE;
    //Clear();
}

void CWeaponTrace::Clear()
{
    //m_PDTVertexVector.clear();
    //m_CurvingTraceVector.clear();

    m_ShortTimePointList.clear();
    m_LongTimePointList.clear();
    Initialize();
}

void CWeaponTrace::Initialize()
{
    m_pInstance = NULL;
    m_dwModelInstanceIndex = 0;

    m_fx = 0.0f;
    m_fy = 0.0f;
    m_fz = 0.0f;
    m_fRotation = 0.0f;

    m_fLifeTime = 0.18f;
    //m_fLifeTime = 3.0f;
    m_fSamplingTime = 0.003f;
    //m_fLifeTime = 3.0f;
    //m_fSamplingTime = 0.003f;

    m_isPlaying = FALSE;

    m_bUseTexture = false;

    m_iBoneIndex = 0;

    m_fLastUpdate = DX::StepTimer::Instance().GetTotalSeconds();
    ///////////////////////////////////////////////////////////////////////

    //const int c_iSplineCount = 8;
    //m_SplineValueVector.clear();
    //m_SplineValueVector.resize(c_iSplineCount);

    //for (int i = 0; i < c_iSplineCount; ++i)
    //{
    //	float fValue = float(i) / float(c_iSplineCount);
    //	m_SplineValueVector[i].fValue1 = fValue;
    //	m_SplineValueVector[i].fValue2 = fValue * fValue;
    //	m_SplineValueVector[i].fValue3 = fValue * fValue * fValue;
    //}
}

CWeaponTrace::CWeaponTrace()
{
    Initialize();
}

CWeaponTrace::~CWeaponTrace()
{
}

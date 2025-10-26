#include "StdAfx.h"
#include "../eterBase/Utils.h"
#include "AttributeInstance.h"
#include "GrpMath.h"

const float c_fStepSize = 50.0f;

bool CAttributeInstance::Picking(const Vector3 &v, const Vector3 &dir, float &out_x, float &out_y)
{
    if (IsEmpty())
        return false;
    //fy *= -1.0f;

    bool bPicked = false;
    float nx = 0;
    float ny = 0;

    for (uint32_t i = 0; i < m_v3HeightDataVector.size(); ++i)
        for (uint32_t j = 0; j < m_v3HeightDataVector[i].size(); j += 3)
        {
            const Vector3 &cv0 = m_v3HeightDataVector[i][j];
            const Vector3 &cv2 = m_v3HeightDataVector[i][j + 1];
            const Vector3 &cv1 = m_v3HeightDataVector[i][j + 2];

            Vector3 n = (cv1 - cv0).Cross((cv2 - cv0));
            Vector3 x;
            float t = - (v - cv0).Dot(n) / dir.Dot(n);

            x = v + t * dir;

            Vector3 temp = (cv1 - cv0).Cross(x - cv0);
            if (temp.Dot(n) < 0)
                continue;
            temp = (cv2 - cv1).Cross(x - cv1);
            if (temp.Dot(n) < 0)
                continue;
            temp = (cv0 - cv2).Cross(x - cv2);
            if (temp.Dot(n) < 0)
                continue;

            if (bPicked)
            {
                if ((v.x - x.x) * (v.x - x.x) + (v.y - x.y) * (v.y - x.y) < (v.x - nx) * (v.x - nx) + (v.y - ny) * (
                        v.y - ny))
                {
                    nx = x.x;
                    ny = x.y;
                }
            }
            else
            {
                nx = x.x;
                ny = x.y;
            }
            bPicked = true;
        }
    if (bPicked)
    {
        out_x = nx;
        out_y = ny;
    }
    return bPicked;
}

bool CAttributeInstance::GetHeight(float fx, float fy, float *pfHeight)
{
    if (IsEmpty())
        return false;

    fy *= -1.0f;

    if (!IsInHeight(fx, fy))
        return false;

    bool bFlag = false;

    for (uint32_t i = 0; i < m_v3HeightDataVector.size(); ++i)
        for (uint32_t j = 0; j < m_v3HeightDataVector[i].size(); j += 3)
        {
            const Vector3 &c_rv3Vertex0 = m_v3HeightDataVector[i][j];
            const Vector3 &c_rv3Vertex1 = m_v3HeightDataVector[i][j + 1];
            const Vector3 &c_rv3Vertex2 = m_v3HeightDataVector[i][j + 2];

            if (
                fx < c_rv3Vertex0.x && fx < c_rv3Vertex1.x && fx < c_rv3Vertex2.x ||
                fx > c_rv3Vertex0.x && fx > c_rv3Vertex1.x && fx > c_rv3Vertex2.x ||
                fy < c_rv3Vertex0.y && fy < c_rv3Vertex1.y && fy < c_rv3Vertex2.y ||
                fy > c_rv3Vertex0.y && fy > c_rv3Vertex1.y && fy > c_rv3Vertex2.y
            )
                continue;

            if (IsInTriangle2D(c_rv3Vertex0.x, c_rv3Vertex0.y,
                               c_rv3Vertex1.x, c_rv3Vertex1.y,
                               c_rv3Vertex2.x, c_rv3Vertex2.y, fx, fy))
            {
                Vector3 v3Line1 = c_rv3Vertex1 - c_rv3Vertex0;
                Vector3 v3Line2 = c_rv3Vertex2 - c_rv3Vertex0;
                Vector3 v3Cross;

                v3Cross = v3Line1.Cross(v3Line2);
                v3Cross.Normalize();

                if (0.0f != v3Cross.z)
                {
                    float fd = (v3Cross.x * c_rv3Vertex0.x + v3Cross.y * c_rv3Vertex0.y + v3Cross.z * c_rv3Vertex0.z);
                    float fm = (v3Cross.x * fx + v3Cross.y * fy);
                    *pfHeight = fMAX((fd - fm) / v3Cross.z, *pfHeight);

                    bFlag = true;
                }
            }
        }

    return bFlag;
}

CAttributeData::Ptr CAttributeInstance::GetObjectPointer() const
{
    return m_roAttributeData;
}

bool CAttributeInstance::IsInHeight(float fx, float fy)
{
    float fdx = m_matGlobal._41 - fx;
    float fdy = m_matGlobal._42 - fy;
    if (sqrtf(fdx * fdx + fdy * fdy) > m_fHeightRadius)
        return false;

    return true;
}

void CAttributeInstance::SetObjectPointer(CAttributeData::Ptr pAttributeData)
{
    Clear();
    m_roAttributeData = pAttributeData;
}

void CAttributeInstance::RefreshObject(const Matrix &c_rmatGlobal)
{
    assert(m_roAttributeData);

    m_matGlobal = c_rmatGlobal;

    // Height
    m_fHeightRadius = m_roAttributeData->GetMaximizeRadius();

    uint32_t dwHeightDataCount = m_roAttributeData->GetHeightDataCount();
    m_v3HeightDataVector.clear();
    m_v3HeightDataVector.resize(dwHeightDataCount);
    for (uint32_t i = 0; i < dwHeightDataCount; ++i)
    {
        const THeightData *c_pHeightData;
        if (!m_roAttributeData->GetHeightDataPointer(i, &c_pHeightData))
            continue;

        uint32_t dwVertexCount = c_pHeightData->v3VertexVector.size();
        m_v3HeightDataVector[i].clear();
        m_v3HeightDataVector[i].resize(dwVertexCount);
        for (uint32_t j = 0; j < dwVertexCount; ++j)
        {
            m_v3HeightDataVector[i][j] = Vector3::Transform(c_pHeightData->v3VertexVector[j], m_matGlobal);
        }
    }
}

const char *CAttributeInstance::GetDataFileName() const
{
    return m_roAttributeData->GetFileName();
}

bool CAttributeInstance::IsEmpty() const
{
    if (!m_v3HeightDataVector.empty())
        return false;

    return true;
}

void CAttributeInstance::Clear()
{
    m_fHeightRadius = 0.0f;
    m_fCollisionRadius = 0.0f;
    m_matGlobal = Matrix::Identity;

    m_v3HeightDataVector.clear();
    m_roAttributeData = nullptr;
}

CAttributeInstance::CAttributeInstance()
    : m_fCollisionRadius(0.0f)
      , m_fHeightRadius(0.0f)
{
    m_matGlobal = Matrix::Identity;
}

CAttributeInstance::~CAttributeInstance()
{
}

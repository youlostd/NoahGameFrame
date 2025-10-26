﻿#include "StdAfx.h"
#include "AttributeData.h"

const char c_szAttributeDataFileHeader[] = "AttributeData";
const int c_iAttributeDataFileHeaderLength = 13;

/*uint32_t CAttributeData::GetCollisionDataCount() const
{
	return m_CollisionDataVector.size();
}

bool CAttributeData::GetCollisionDataPointer(uint32_t dwIndex, const TCollisionData ** c_ppCollisionData) const
{
	if (dwIndex >= GetCollisionDataCount())
		return false;

	*c_ppCollisionData = &m_CollisionDataVector[dwIndex];

	return true;
}
*/

const CStaticCollisionDataVector &CAttributeData::GetCollisionDataVector() const
{
    return m_StaticCollisionDataVector;
}

const THeightDataVector &CAttributeData::GetHeightDataVector() const
{
    return m_HeightDataVector;
}

uint32_t CAttributeData::GetHeightDataCount() const
{
    return m_HeightDataVector.size();
}

bool CAttributeData::GetHeightDataPointer(uint32_t dwIndex, const THeightData **c_ppHeightData) const
{
    if (dwIndex >= GetHeightDataCount())
        return false;

    *c_ppHeightData = &m_HeightDataVector[dwIndex];

    return true;
}

float CAttributeData::GetMaximizeRadius()
{
    return m_fMaximizeRadius;
}

size_t CAttributeData::AddCollisionData(const CStaticCollisionData &data)
{
    m_StaticCollisionDataVector.push_back(data);
    return m_StaticCollisionDataVector.size();
}

bool CAttributeData::Load(int /*iSize*/, const void *c_pvBuf)
{
    const uint8_t *c_pbBuf = static_cast<const uint8_t *>(c_pvBuf);

    char szHeader[c_iAttributeDataFileHeaderLength + 1];
    memcpy(szHeader, c_pbBuf, c_iAttributeDataFileHeaderLength + 1);
    c_pbBuf += c_iAttributeDataFileHeaderLength + 1;
    if (strcmp(szHeader, c_szAttributeDataFileHeader))
        return false;

    uint32_t dwCollisionDataCount;
    uint32_t dwHeightDataCount;
    memcpy(&dwCollisionDataCount, c_pbBuf, sizeof(uint32_t));
    c_pbBuf += sizeof(uint32_t);
    memcpy(&dwHeightDataCount, c_pbBuf, sizeof(uint32_t));
    c_pbBuf += sizeof(uint32_t);

    m_StaticCollisionDataVector.clear();
    m_StaticCollisionDataVector.resize(dwCollisionDataCount);
    m_HeightDataVector.clear();
    m_HeightDataVector.resize(dwHeightDataCount);

    for (uint32_t i = 0; i < dwCollisionDataCount; ++i)
    {
        CStaticCollisionData &rCollisionData = m_StaticCollisionDataVector[i];
        memcpy(&rCollisionData.dwType, c_pbBuf, sizeof(uint32_t));
        c_pbBuf += sizeof(uint32_t);
        memcpy(rCollisionData.szName, c_pbBuf, 32);
        c_pbBuf += 32;
        memcpy(&rCollisionData.v3Position, c_pbBuf, sizeof(Vector3));
        c_pbBuf += sizeof(Vector3);

        switch (rCollisionData.dwType)
        {
        case COLLISION_TYPE_PLANE:
            memcpy(rCollisionData.fDimensions, c_pbBuf, 2 * sizeof(float));
            c_pbBuf += 2 * sizeof(float);
            break;
        case COLLISION_TYPE_BOX:
            memcpy(rCollisionData.fDimensions, c_pbBuf, 3 * sizeof(float));
            c_pbBuf += 3 * sizeof(float);
            break;
        case COLLISION_TYPE_SPHERE:
            memcpy(rCollisionData.fDimensions, c_pbBuf, sizeof(float));
            c_pbBuf += sizeof(float);
            break;
        case COLLISION_TYPE_CYLINDER:
            memcpy(rCollisionData.fDimensions, c_pbBuf, 2 * sizeof(float));
            c_pbBuf += 2 * sizeof(float);
            break;
        case COLLISION_TYPE_AABB:
            memcpy(rCollisionData.fDimensions, c_pbBuf, 3 * sizeof(float));
            c_pbBuf += 3 * sizeof(float);
            break;
        case COLLISION_TYPE_OBB:
            memcpy(rCollisionData.fDimensions, c_pbBuf, 3 * sizeof(float));
            c_pbBuf += 3 * sizeof(float);
            break;
        }

        memcpy(&rCollisionData.quatRotation, c_pbBuf, sizeof(Quaternion));
        c_pbBuf += sizeof(Quaternion);
    }

    for (uint32_t j = 0; j < dwHeightDataCount; ++j)
    {
        THeightData &rHeightData = m_HeightDataVector[j];
        memcpy(rHeightData.szName, c_pbBuf, 32);
        c_pbBuf += 32;

        uint32_t dwPrimitiveCount;
        memcpy(&dwPrimitiveCount, c_pbBuf, sizeof(uint32_t));
        c_pbBuf += sizeof(uint32_t);

        rHeightData.v3VertexVector.clear();
        rHeightData.v3VertexVector.resize(dwPrimitiveCount);
        memcpy(&rHeightData.v3VertexVector[0], c_pbBuf, dwPrimitiveCount * sizeof(Vector3));
        c_pbBuf += dwPrimitiveCount * sizeof(Vector3);

        // Getting Maximize Radius
        for (uint32_t k = 0; k < rHeightData.v3VertexVector.size(); ++k)
        {
            m_fMaximizeRadius = std::max(m_fMaximizeRadius, fabs(rHeightData.v3VertexVector[k].x) + 50.0f);
            m_fMaximizeRadius = std::max(m_fMaximizeRadius, fabs(rHeightData.v3VertexVector[k].y) + 50.0f);
            m_fMaximizeRadius = std::max(m_fMaximizeRadius, fabs(rHeightData.v3VertexVector[k].z) + 50.0f);
        }
        // Getting Maximize Radius
    }

    return true;
}

void CAttributeData::Clear()
{
    m_StaticCollisionDataVector.clear();
    m_HeightDataVector.clear();
}

CAttributeData::CAttributeData()
{
    m_fMaximizeRadius = 0.0f;
}

CAttributeData::~CAttributeData()
{
}

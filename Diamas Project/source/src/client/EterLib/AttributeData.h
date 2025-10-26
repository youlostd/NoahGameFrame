#pragma once

#include "Resource.h"
#include "CollisionData.h"

typedef struct SHeightData
{
    char szName[32 + 1];
    std::vector<Vector3> v3VertexVector;
} THeightData;

using THeightDataVector = std::vector<THeightData>;

class CAttributeData : public ConcreteResource<CAttributeData>
{
    /*
    enum ECollisionType
    {
    COLLISION_TYPE_PLANE,
    COLLISION_TYPE_BOX,
    COLLISION_TYPE_SPHERE,
    COLLISION_TYPE_CYLINDER,
    };

    typedef struct SCollisionData
    {
    uint32_t dwType;
    char szName[32+1];

    Vector3 v3Position;
    float fDimensions[3];
    D3DXD3DXQUATERNION quatRotation;
    } TCollisionData;*/

public:
    CAttributeData();
    ~CAttributeData() override;

    void Clear() override;

    bool Load(int iSize, const void *c_pvBuf) override;

    //uint32_t GetCollisionDataCount() const;
    //bool GetCollisionDataPointer(uint32_t dwIndex, const TCollisionData ** c_ppCollisionData) const;
    const CStaticCollisionDataVector &GetCollisionDataVector() const;
    const THeightDataVector &GetHeightDataVector() const;

    size_t AddCollisionData(const CStaticCollisionData &collisionData); // return m_StaticCollisionDataVector.size();

    uint32_t GetHeightDataCount() const;
    bool GetHeightDataPointer(uint32_t dwIndex, const THeightData **c_ppHeightData) const;

    float GetMaximizeRadius();

protected:
    float m_fMaximizeRadius;

    //std::vector<TCollisionData> m_CollisionDataVector;
    CStaticCollisionDataVector m_StaticCollisionDataVector;
    THeightDataVector m_HeightDataVector;
};

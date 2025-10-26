#ifndef METIN2_CLIENT_ETERLIB_ATTRIBUTEINSTANCE_HPP
#define METIN2_CLIENT_ETERLIB_ATTRIBUTEINSTANCE_HPP

#pragma once

#include "AttributeData.h"
#include "Pool.h"

#include <vector>

class CAttributeInstance
{
public:
    CAttributeInstance();
    virtual ~CAttributeInstance();

    void Clear();
    bool IsEmpty() const;

    const char *GetDataFileName() const;

    // NOTE : Object 전용
    void SetObjectPointer(CAttributeData::Ptr pAttributeData);
    void RefreshObject(const Matrix &c_rmatGlobal);
    CAttributeData::Ptr GetObjectPointer() const;

    bool Picking(const Vector3 &v, const Vector3 &dir, float &out_x, float &out_y);

    bool IsInHeight(float fx, float fy);
    bool GetHeight(float fx, float fy, float *pfHeight);

    bool IsHeightData() const;

protected:
    void SetGlobalMatrix(const Matrix &c_rmatGlobal);
    void SetGlobalPosition(const Vector3 &c_rv3Position);

protected:
    float m_fCollisionRadius;
    float m_fHeightRadius;

    Matrix m_matGlobal;

    std::vector<std::vector<Vector3>> m_v3HeightDataVector;

    CAttributeData::Ptr m_roAttributeData;
};

#endif

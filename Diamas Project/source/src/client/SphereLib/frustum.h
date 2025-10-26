/* Copyright (C) John W. Ratcliff, 2001.
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) John W. Ratcliff, 2001"
 */
#ifndef METIN2_CLIENT_SPHERELIB_FRUSTUM_HPP
#define METIN2_CLIENT_SPHERELIB_FRUSTUM_HPP

#pragma once

/***********************************************************************/
/** FRUSTUM.H   : Represents a clipping frustum.                       */
/**               You should replace this with your own more robust    */
/**               view frustum clipper.                                */
/**                                                                    */
/**               Written by John W. Ratcliff jratcliff@att.net        */
/***********************************************************************/

#include "vector.h"


enum ViewState
{
    VS_INSIDE,
    // completely inside the frustum.
    VS_PARTIAL,
    // partially inside and partially outside the frustum.
    VS_OUTSIDE // completely outside the frustum
};

struct Frustum
{
    void BuildViewFrustum(Matrix &mat);
    void BuildViewFrustum2(Matrix &mat, float fNear, float fFar, float fFov, float fAspect,
                           const Vector3 &vCamera, const Vector3 &vLook);
    [[nodiscard]] ViewState ViewVolumeTest(const Vector3 &c_v3Center, const float c_fRadius) const;

private:
    bool m_bUsingSphere = false;
    Vector3 m_v3Center;
    float m_fRadius = 0.0f;
    Plane m_plane[6];
};

#endif

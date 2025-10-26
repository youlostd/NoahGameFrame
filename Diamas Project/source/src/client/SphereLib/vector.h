/* Copyright (C) John W. Ratcliff, 2001.
* All rights reserved worldwide.
*
* This software is provided "as is" without express or implied
* warranties. You may freely copy and compile this source into
* applications you distribute provided that the copyright text
* below is included in the resulting source code, for example:
* "Portions Copyright (C) John W. Ratcliff, 2001"
*/
#ifndef METIN2_CLIENT_SPHERELIB_VECTOR_HPP
#define METIN2_CLIENT_SPHERELIB_VECTOR_HPP

#pragma once

#include <math.h>
#include <vector>

#include <Direct3D.hpp>
//#include "stl.h"

/***********************************************************************/
/** VECTOR.H    : Template class to represent a 2d and 3d vector type. */
/**                                                                    */
/**               Written by John W. Ratcliff jratcliff@att.net        */
/***********************************************************************/

using Vector3d = DirectX::SimpleMath::Vector3;

typedef std::vector<Vector3d> Vector3dVector;

inline Vector3d operator *(float s, const Vector3d &v)
{
    Vector3d Scaled(v.x * s, v.y * s, v.z * s);
    return (Scaled);
};

#endif

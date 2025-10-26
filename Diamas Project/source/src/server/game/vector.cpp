#include "vector.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <boost/math/constants/constants.hpp>
#include <storm/math/AngleUtil.hpp>

float GetDegreeFromPosition(float x, float y)
{
    return storm::SignedDegreeToUnsignedDegree(storm::DegreesFromRadians(std::atan2(x, y)));
}

float GetDegreeFromPositionXY(int32_t sx, int32_t sy, int32_t ex, int32_t ey)
{
    return GetDegreeFromPosition(ex - sx, ey - sy);
}

void GetDeltaByDegree(float fDegree, float fDistance, float *x, float *y)
{
    float rad = storm::RadiansFromDegrees(fDegree);

    *x = fDistance * std::sin(rad);
    *y = fDistance * std::cos(rad);
}

float GetDegreeDelta(float iDegree, float iDegree2)
{
    if (iDegree > 180.0f)
        iDegree = iDegree - 360.0f;

    if (iDegree2 > 180.0f)
        iDegree2 = iDegree2 - 360.0f;

    return fabs(iDegree - iDegree2);
}

// This actually gets us a bounding rectangle, which isn't too bad,
// but not optimal either.
void RotateRegion(int32_t &sx, int32_t &sy,
                  int32_t &ex, int32_t &ey,
                  float xRot, float yRot, float zRot)
{
    xRot = storm::RadiansFromDegrees(xRot);
    yRot = storm::RadiansFromDegrees(yRot);
    zRot = storm::RadiansFromDegrees(zRot);

    Eigen::Vector3f center(sx + (ex - sx) / 2,
                           sy + (ey - sy) / 2,
                           0.0f);

    Eigen::Matrix3f m;
    m = Eigen::AngleAxisf(xRot, Eigen::Vector3f::UnitX()) *
        Eigen::AngleAxisf(yRot, Eigen::Vector3f::UnitY()) *
        Eigen::AngleAxisf(zRot, Eigen::Vector3f::UnitZ());

    Eigen::Vector3f start = center + m * (Eigen::Vector3f(sx, sy, 0.0f) - center);
    Eigen::Vector3f end = center + m * (Eigen::Vector3f(ex, ey, 0.0f) - center);

    sx = static_cast<int32_t>(start.x());
    sy = static_cast<int32_t>(start.y());
    ex = static_cast<int32_t>(end.x());
    ey = static_cast<int32_t>(end.y());

    if (sx > ex)
        std::swap(sx, ex);

    if (sy > ey)
        std::swap(sy, ey);
}

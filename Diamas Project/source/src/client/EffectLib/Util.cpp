#include "Util.hpp"

Color BlendColor(const Color &a, const Color &b)
{
    if (b.A() == 0.0f)
        return a;

    return DirectX::SimpleMath::Color(
        0.125f * a.R() + 0.875f * b.R(),
        0.125f * a.G() + 0.875f * b.G(),
        0.125f * a.B() + 0.875f * b.B(),
        a.A()
        );
}

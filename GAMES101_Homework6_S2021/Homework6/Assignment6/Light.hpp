//
// 由 Göksu Güvendiren 于 2019/5/14 创建。
//

#pragma once

#include "Vector.hpp"

class Light
{
public:
    Light(const Vector3f &p, const Vector3f &i) : position(p), intensity(i) {}
    virtual ~Light() = default;
    Vector3f position;
    Vector3f intensity;
};

#pragma once

namespace Game
{
    struct Vector2
    {
        float x, y;
        Vector2() : x(0), y(0) {}
        Vector2(float _x, float _y) : x(_x), y(_y) {}
    };

    struct Vector3
    {
        float x, y, z;
        Vector3() : x(0), y(0), z(0) {}
        Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

        Vector3 operator-(const Vector3& o) const { return { x - o.x, y - o.y, z - o.z }; }
        Vector3 operator+(const Vector3& o) const { return { x + o.x, y + o.y, z + o.z }; }
        bool IsZero() const { return x == 0.f && y == 0.f && z == 0.f; }
        float Length() const { return sqrtf(x * x + y * y + z * z); }
        float Length2D() const { return sqrtf(x * x + y * y); }
    };

    struct QAngle
    {
        float pitch, yaw, roll;
        QAngle() : pitch(0), yaw(0), roll(0) {}
        QAngle(float p, float y, float r) : pitch(p), yaw(y), roll(r) {}
    };

    struct ViewMatrix
    {
        float m[4][4];
    };
}

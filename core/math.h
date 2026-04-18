#pragma once

// ---------------------------------------------------------------
// Math primitives used across the cheat — vectors, angles, matrix.
// ---------------------------------------------------------------

#include <cmath>

namespace Math
{
    constexpr float kPi     = 3.14159265358979323846f;
    constexpr float kRad2Deg = 180.0f / kPi;
    constexpr float kDeg2Rad = kPi / 180.0f;

    struct Vec3
    {
        float x{}, y{}, z{};

        Vec3() = default;
        Vec3(float a, float b, float c) : x(a), y(b), z(c) {}

        Vec3 operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
        Vec3 operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
        Vec3 operator*(float s)       const { return { x * s, y * s, z * s }; }

        bool  IsZero()   const { return x == 0.f && y == 0.f && z == 0.f; }
        float Length()   const { return sqrtf(x * x + y * y + z * z); }
        float Length2D() const { return sqrtf(x * x + y * y); }
        float Dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    };

    struct QAngle
    {
        float pitch{}, yaw{}, roll{};

        QAngle() = default;
        QAngle(float p, float y, float r = 0.f) : pitch(p), yaw(y), roll(r) {}
    };

    struct ViewMatrix
    {
        float m[4][4];
    };

    inline void ClampAngles(QAngle& a)
    {
        if (a.pitch >  89.f)  a.pitch =  89.f;
        if (a.pitch < -89.f)  a.pitch = -89.f;
        while (a.yaw >  180.f) a.yaw -= 360.f;
        while (a.yaw < -180.f) a.yaw += 360.f;
        a.roll = 0.f;
    }

    inline QAngle CalcAngle(const Vec3& src, const Vec3& dst)
    {
        Vec3 d = dst - src;
        float hyp = d.Length2D();
        QAngle out;
        out.pitch = -atan2f(d.z, hyp) * kRad2Deg;
        out.yaw   =  atan2f(d.y, d.x) * kRad2Deg;
        out.roll  = 0.f;
        ClampAngles(out);
        return out;
    }

    inline float AngleFov(const QAngle& view, const QAngle& aim)
    {
        float dp = aim.pitch - view.pitch;
        float dy = aim.yaw   - view.yaw;
        while (dy >  180.f) dy -= 360.f;
        while (dy < -180.f) dy += 360.f;
        return sqrtf(dp * dp + dy * dy);
    }

    inline void AngleToDirection(float pitch, float yaw, float out[3])
    {
        float cp = cosf(pitch * kDeg2Rad);
        float sp = sinf(pitch * kDeg2Rad);
        float cy = cosf(yaw   * kDeg2Rad);
        float sy = sinf(yaw   * kDeg2Rad);
        out[0] = cp * cy;
        out[1] = cp * sy;
        out[2] = -sp;
    }

    inline bool WorldToScreen(const ViewMatrix& vm, const float* world,
                              float& sx, float& sy, float scrW, float scrH)
    {
        float w = vm.m[3][0] * world[0] + vm.m[3][1] * world[1]
                + vm.m[3][2] * world[2] + vm.m[3][3];
        if (w < 0.001f) return false;
        float inv = 1.0f / w;
        float x = vm.m[0][0] * world[0] + vm.m[0][1] * world[1]
                + vm.m[0][2] * world[2] + vm.m[0][3];
        float y = vm.m[1][0] * world[0] + vm.m[1][1] * world[1]
                + vm.m[1][2] * world[2] + vm.m[1][3];
        sx = scrW * 0.5f + x * inv * scrW * 0.5f;
        sy = scrH * 0.5f - y * inv * scrH * 0.5f;
        return true;
    }
}

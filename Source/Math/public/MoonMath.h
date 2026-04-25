#pragma once

// 将所有 GLM 宏集中在数学库内部，不泄漏到项目其他文件
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <cmath>
#include <cstddef>
#include <string>

// -----------------------------
// MoonVector2
// -----------------------------
struct MoonVector2
{
    float x = 0.0f;
    float y = 0.0f;

    MoonVector2() = default;
    constexpr MoonVector2(float inX, float inY) : x(inX), y(inY) {}
    explicit MoonVector2(float inF) : x(inF), y(inF) {}

    float& operator[](std::size_t i) { return (&x)[i]; }
    const float& operator[](std::size_t i) const { return (&x)[i]; }

    MoonVector2 operator+(const MoonVector2& other) const { return MoonVector2(x + other.x, y + other.y); }
    MoonVector2 operator-(const MoonVector2& other) const { return MoonVector2(x - other.x, y - other.y); }
    MoonVector2 operator*(float s) const { return MoonVector2(x * s, y * s); }
    MoonVector2 operator/(float s) const { return MoonVector2(x / s, y / s); }
    MoonVector2 operator-() const { return MoonVector2(-x, -y); }

    MoonVector2& operator+=(const MoonVector2& other) { x += other.x; y += other.y; return *this; }
    MoonVector2& operator-=(const MoonVector2& other) { x -= other.x; y -= other.y; return *this; }
    MoonVector2& operator*=(float s) { x *= s; y *= s; return *this; }
    MoonVector2& operator/=(float s) { x /= s; y /= s; return *this; }

    std::string ToString() const;
};

inline MoonVector2 operator*(float s, const MoonVector2& v) { return v * s; }

// -----------------------------
// MoonVector3
// -----------------------------
struct MoonVector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    MoonVector3() = default;
    constexpr MoonVector3(float inX, float inY, float inZ) : x(inX), y(inY), z(inZ) {}
    explicit MoonVector3(float inF) : x(inF), y(inF), z(inF) {}

    float& operator[](std::size_t i) { return (&x)[i]; }
    const float& operator[](std::size_t i) const { return (&x)[i]; }

    MoonVector3 operator+(const MoonVector3& other) const { return MoonVector3(x + other.x, y + other.y, z + other.z); }
    MoonVector3 operator-(const MoonVector3& other) const { return MoonVector3(x - other.x, y - other.y, z - other.z); }
    MoonVector3 operator*(float s) const { return MoonVector3(x * s, y * s, z * s); }
    MoonVector3 operator/(float s) const { return MoonVector3(x / s, y / s, z / s); }
    MoonVector3 operator-() const { return MoonVector3(-x, -y, -z); }

    MoonVector3& operator+=(const MoonVector3& other) { x += other.x; y += other.y; z += other.z; return *this; }
    MoonVector3& operator-=(const MoonVector3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
    MoonVector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    MoonVector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

    std::string ToString() const;
};

inline MoonVector3 operator*(float s, const MoonVector3& v) { return v * s; }

// -----------------------------
// MoonVector4
// -----------------------------
struct MoonVector4
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    MoonVector4() = default;
    constexpr MoonVector4(float inX, float inY, float inZ, float inW) : x(inX), y(inY), z(inZ), w(inW) {}
    explicit MoonVector4(float inF) : x(inF), y(inF), z(inF), w(inF) {}
    MoonVector4(const MoonVector3& v, float inW) : x(v.x), y(v.y), z(v.z), w(inW) {}

    float& operator[](std::size_t i) { return (&x)[i]; }
    const float& operator[](std::size_t i) const { return (&x)[i]; }

    MoonVector4 operator+(const MoonVector4& other) const { return MoonVector4(x + other.x, y + other.y, z + other.z, w + other.w); }
    MoonVector4 operator-(const MoonVector4& other) const { return MoonVector4(x - other.x, y - other.y, z - other.z, w - other.w); }
    MoonVector4 operator*(float s) const { return MoonVector4(x * s, y * s, z * s, w * s); }
    MoonVector4 operator/(float s) const { return MoonVector4(x / s, y / s, z / s, w / s); }
    MoonVector4 operator-() const { return MoonVector4(-x, -y, -z, -w); }

    MoonVector4& operator+=(const MoonVector4& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
    MoonVector4& operator-=(const MoonVector4& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
    MoonVector4& operator*=(float s) { x *= s; y *= s; z *= s; w *= s; return *this; }
    MoonVector4& operator/=(float s) { x /= s; y /= s; z /= s; w /= s; return *this; }

    MoonVector3 ToVector3() const { return MoonVector3(x, y, z); }
    std::string ToString() const;
};

inline MoonVector4 operator*(float s, const MoonVector4& v) { return v * s; }

// -----------------------------
// MoonMatrix4x4
// -----------------------------
struct MoonMatrix4x4
{
    float m[4][4];

    MoonMatrix4x4();
    MoonMatrix4x4(const MoonMatrix4x4& other);
    MoonMatrix4x4& operator=(const MoonMatrix4x4& other);

    float* Data() { return &m[0][0]; }
    const float* Data() const { return &m[0][0]; }

    static MoonMatrix4x4 Identity();

    MoonMatrix4x4 operator*(const MoonMatrix4x4& other) const;
    MoonVector4 operator*(const MoonVector4& v) const;
    MoonMatrix4x4 operator*(float s) const;

    MoonMatrix4x4 Transpose() const;
    MoonMatrix4x4 Inverse() const;

    MoonVector3 TransformPosition(const MoonVector3& pos) const;
    MoonVector3 TransformDirection(const MoonVector3& dir) const;

    std::string ToString() const;
};

// -----------------------------
// Global math functions
// -----------------------------

inline float MoonSqrt(float v) { return std::sqrt(v); }
inline float MoonAbs(float v) { return std::abs(v); }
inline float MoonRadians(float degrees) { return degrees * 0.01745329252f; }
inline float MoonDegrees(float radians) { return radians * 57.295779513f; }
inline float MoonClamp(float v, float minV, float maxV) { return v < minV ? minV : (v > maxV ? maxV : v); }

// Vector2
inline float MoonDot(const MoonVector2& a, const MoonVector2& b) { return a.x * b.x + a.y * b.y; }
inline float MoonLengthSq(const MoonVector2& v) { return MoonDot(v, v); }
inline float MoonLength(const MoonVector2& v) { return MoonSqrt(MoonLengthSq(v)); }
inline MoonVector2 MoonNormalize(const MoonVector2& v) { float len = MoonLength(v); return len > 0.0f ? v / len : MoonVector2(0.0f); }

// Vector3
inline float MoonDot(const MoonVector3& a, const MoonVector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float MoonLengthSq(const MoonVector3& v) { return MoonDot(v, v); }
inline float MoonLength(const MoonVector3& v) { return MoonSqrt(MoonLengthSq(v)); }
inline MoonVector3 MoonNormalize(const MoonVector3& v) { float len = MoonLength(v); return len > 0.0f ? v / len : MoonVector3(0.0f); }
inline MoonVector3 MoonCross(const MoonVector3& a, const MoonVector3& b)
{
    return MoonVector3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
}

// Vector4
inline float MoonDot(const MoonVector4& a, const MoonVector4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

// Matrix
MoonMatrix4x4 MoonLookAt(const MoonVector3& eye, const MoonVector3& target, const MoonVector3& up);
MoonMatrix4x4 MoonPerspective(float fovRadians, float aspect, float nearPlane, float farPlane);
MoonMatrix4x4 MoonTranslate(const MoonVector3& translation);
MoonMatrix4x4 MoonScale(const MoonVector3& scale);
MoonMatrix4x4 MoonRotate(float angleRadians, const MoonVector3& axis);
MoonMatrix4x4 MoonInverseTranspose(const MoonMatrix4x4& m);

// -----------------------------
// Internal glm conversion helpers (only used by MoonMath.cpp and backends)
// -----------------------------
namespace MoonMathInternal
{
    inline glm::vec2 ToGLM(const MoonVector2& v) { return glm::vec2(v.x, v.y); }
    inline glm::vec3 ToGLM(const MoonVector3& v) { return glm::vec3(v.x, v.y, v.z); }
    inline glm::vec4 ToGLM(const MoonVector4& v) { return glm::vec4(v.x, v.y, v.z, v.w); }
    inline glm::mat4 ToGLM(const MoonMatrix4x4& m) { return glm::make_mat4(m.Data()); }

    inline MoonVector2 FromGLM(const glm::vec2& v) { return MoonVector2(v.x, v.y); }
    inline MoonVector3 FromGLM(const glm::vec3& v) { return MoonVector3(v.x, v.y, v.z); }
    inline MoonVector4 FromGLM(const glm::vec4& v) { return MoonVector4(v.x, v.y, v.z, v.w); }
    inline MoonMatrix4x4 FromGLM(const glm::mat4& m)
    {
        MoonMatrix4x4 result;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                result.m[i][j] = m[i][j];
        return result;
    }
}

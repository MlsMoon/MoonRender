#include "Source/Math/public/MoonMath.h"

#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <iomanip>

// -----------------------------
// MoonMatrix4x4
// -----------------------------

MoonMatrix4x4::MoonMatrix4x4()
{
    std::memset(m, 0, sizeof(m));
}

MoonMatrix4x4::MoonMatrix4x4(const MoonMatrix4x4& other)
{
    std::memcpy(m, other.m, sizeof(m));
}

MoonMatrix4x4& MoonMatrix4x4::operator=(const MoonMatrix4x4& other)
{
    if (this != &other)
    {
        std::memcpy(m, other.m, sizeof(m));
    }
    return *this;
}

MoonMatrix4x4 MoonMatrix4x4::Identity()
{
    MoonMatrix4x4 result;
    result.m[0][0] = 1.0f; result.m[0][1] = 0.0f; result.m[0][2] = 0.0f; result.m[0][3] = 0.0f;
    result.m[1][0] = 0.0f; result.m[1][1] = 1.0f; result.m[1][2] = 0.0f; result.m[1][3] = 0.0f;
    result.m[2][0] = 0.0f; result.m[2][1] = 0.0f; result.m[2][2] = 1.0f; result.m[2][3] = 0.0f;
    result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;
    return result;
}

MoonMatrix4x4 MoonMatrix4x4::operator*(const MoonMatrix4x4& other) const
{
    const glm::mat4 a = glm::make_mat4(&m[0][0]);
    const glm::mat4 b = glm::make_mat4(&other.m[0][0]);
    const glm::mat4 r = a * b;
    MoonMatrix4x4 result;
    std::memcpy(result.m, glm::value_ptr(r), sizeof(result.m));
    return result;
}

MoonVector4 MoonMatrix4x4::operator*(const MoonVector4& v) const
{
    const glm::mat4 mat = glm::make_mat4(&m[0][0]);
    const glm::vec4 r = mat * glm::vec4(v.x, v.y, v.z, v.w);
    return MoonVector4(r.x, r.y, r.z, r.w);
}

MoonMatrix4x4 MoonMatrix4x4::operator*(float s) const
{
    MoonMatrix4x4 result;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result.m[i][j] = m[i][j] * s;
    return result;
}

MoonMatrix4x4 MoonMatrix4x4::Transpose() const
{
    const glm::mat4 mat = glm::make_mat4(&m[0][0]);
    const glm::mat4 r = glm::transpose(mat);
    MoonMatrix4x4 result;
    std::memcpy(result.m, glm::value_ptr(r), sizeof(result.m));
    return result;
}

MoonMatrix4x4 MoonMatrix4x4::Inverse() const
{
    const glm::mat4 mat = glm::make_mat4(&m[0][0]);
    const glm::mat4 r = glm::inverse(mat);
    MoonMatrix4x4 result;
    std::memcpy(result.m, glm::value_ptr(r), sizeof(result.m));
    return result;
}

MoonVector3 MoonMatrix4x4::TransformPosition(const MoonVector3& pos) const
{
    const glm::mat4 mat = glm::make_mat4(&m[0][0]);
    const glm::vec4 r = mat * glm::vec4(pos.x, pos.y, pos.z, 1.0f);
    return MoonVector3(r.x, r.y, r.z);
}

MoonVector3 MoonMatrix4x4::TransformDirection(const MoonVector3& dir) const
{
    const glm::mat4 mat = glm::make_mat4(&m[0][0]);
    const glm::vec4 r = mat * glm::vec4(dir.x, dir.y, dir.z, 0.0f);
    return MoonVector3(r.x, r.y, r.z);
}

std::string MoonMatrix4x4::ToString() const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    for (int i = 0; i < 4; ++i)
    {
        oss << "[";
        for (int j = 0; j < 4; ++j)
        {
            oss << m[j][i] << (j < 3 ? ", " : "");
        }
        oss << "]" << (i < 3 ? "\n" : "");
    }
    return oss.str();
}

// -----------------------------
// Vector ToString
// -----------------------------

std::string MoonVector2::ToString() const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "(" << x << ", " << y << ")";
    return oss.str();
}

std::string MoonVector3::ToString() const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "(" << x << ", " << y << ", " << z << ")";
    return oss.str();
}

std::string MoonVector4::ToString() const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "(" << x << ", " << y << ", " << z << ", " << w << ")";
    return oss.str();
}

// -----------------------------
// Global matrix functions
// -----------------------------

MoonMatrix4x4 MoonLookAt(const MoonVector3& eye, const MoonVector3& target, const MoonVector3& up)
{
    const glm::mat4 r = glm::lookAt(
        glm::vec3(eye.x, eye.y, eye.z),
        glm::vec3(target.x, target.y, target.z),
        glm::vec3(up.x, up.y, up.z));
    MoonMatrix4x4 result;
    std::memcpy(result.m, glm::value_ptr(r), sizeof(result.m));
    return result;
}

MoonMatrix4x4 MoonPerspective(float fovRadians, float aspect, float nearPlane, float farPlane)
{
    const glm::mat4 r = glm::perspective(fovRadians, aspect, nearPlane, farPlane);
    MoonMatrix4x4 result;
    std::memcpy(result.m, glm::value_ptr(r), sizeof(result.m));
    return result;
}

MoonMatrix4x4 MoonTranslate(const MoonVector3& translation)
{
    const glm::mat4 r = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));
    MoonMatrix4x4 result;
    std::memcpy(result.m, glm::value_ptr(r), sizeof(result.m));
    return result;
}

MoonMatrix4x4 MoonScale(const MoonVector3& scale)
{
    const glm::mat4 r = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));
    MoonMatrix4x4 result;
    std::memcpy(result.m, glm::value_ptr(r), sizeof(result.m));
    return result;
}

MoonMatrix4x4 MoonRotate(float angleRadians, const MoonVector3& axis)
{
    const glm::mat4 r = glm::rotate(glm::mat4(1.0f), angleRadians, glm::vec3(axis.x, axis.y, axis.z));
    MoonMatrix4x4 result;
    std::memcpy(result.m, glm::value_ptr(r), sizeof(result.m));
    return result;
}

MoonMatrix4x4 MoonInverseTranspose(const MoonMatrix4x4& m)
{
    glm::mat4 mat = glm::make_mat4(m.Data());
    mat[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    const glm::mat4 r = glm::transpose(glm::inverse(mat));
    MoonMatrix4x4 result;
    std::memcpy(result.m, glm::value_ptr(r), sizeof(result.m));
    return result;
}


﻿#pragma once
#include <directxmath.h>

namespace Render
{
    // 方向光
    struct DirectionalLight
    {
        DirectionalLight() = default;

        DirectionalLight(const DirectionalLight&) = default;
        DirectionalLight& operator=(const DirectionalLight&) = default;

        DirectionalLight(DirectionalLight&&) = default;
        DirectionalLight& operator=(DirectionalLight&&) = default;

        DirectionalLight(const DirectX::XMFLOAT4& _direction_intensity) : direction_intensity(_direction_intensity) {}
        
        DirectX::XMFLOAT4 direction_intensity; // dir : x,y,z intensity: w

    };

    
    // 点光
    struct PointLight
    {
        PointLight() = default;

        PointLight(const PointLight&) = default;
        PointLight& operator=(const PointLight&) = default;

        PointLight(PointLight&&) = default;
        PointLight& operator=(PointLight&&) = default;

        PointLight(const DirectX::XMFLOAT4& _ambient, const DirectX::XMFLOAT4& _diffuse, const DirectX::XMFLOAT4& _specular,
            const DirectX::XMFLOAT3& _position, float _range, const DirectX::XMFLOAT3& _att) :
            ambient(_ambient), diffuse(_diffuse), specular(_specular), position(_position), range(_range), att(_att), pad() {}

        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT4 specular;

        // 打包成4D向量: (position, range)
        DirectX::XMFLOAT3 position;
        float range;

        // 打包成4D向量: (A0, A1, A2, pad)
        DirectX::XMFLOAT3 att;
        float pad; // 最后用一个浮点数填充使得该结构体大小满足16的倍数，便于我们以后在HLSL设置数组
    };
    

    // 聚光灯
    struct SpotLight
    {
        SpotLight() = default;

        SpotLight(const SpotLight&) = default;
        SpotLight& operator=(const SpotLight&) = default;

        SpotLight(SpotLight&&) = default;
        SpotLight& operator=(SpotLight&&) = default;

        SpotLight(const DirectX::XMFLOAT4& _ambient, const DirectX::XMFLOAT4& _diffuse, const DirectX::XMFLOAT4& _specular,
            const DirectX::XMFLOAT3& _position, float _range, const DirectX::XMFLOAT3& _direction,
            float _spot, const DirectX::XMFLOAT3& _att) :
            ambient(_ambient), diffuse(_diffuse), specular(_specular), 
            position(_position), range(_range), direction(_direction), spot(_spot), att(_att), pad() {}

        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT4 specular;

        // 打包成4D向量: (position, range)
        DirectX::XMFLOAT3 position;
        float range;

        // 打包成4D向量: (direction, spot)
        DirectX::XMFLOAT3 direction;
        float spot;

        // 打包成4D向量: (att, pad)
        DirectX::XMFLOAT3 att;
        float pad; // 最后用一个浮点数填充使得该结构体大小满足16的倍数，便于我们以后在HLSL设置数组
    };
}


#pragma once

#include <directxmath.h>
#include <functional>
#include <string>
#include <vector>

namespace Object
{
    class MoonObject;

    enum class ComponentType
    {
        Transform,
        Mesh,
        Light,
        Camera,
        AutoRotate
    };

    enum class ComponentPropertyType
    {
        Text,
        Float,
        Float3,
        Float4
    };

    enum class ComponentConflictGroup
    {
        None,
        Transform,
        Renderable,
        Light,
        Camera
    };

    struct ComponentProperty
    {
        const char* name = "";
        ComponentPropertyType type = ComponentPropertyType::Text;
        bool readOnly = false;
        bool clamp = false;
        float speed = 0.1f;
        float minValue = 0.0f;
        float maxValue = 0.0f;
        const char* format = "%.3f";

        std::function<std::string()> getText;
        std::function<float()> getFloat;
        std::function<void(float)> setFloat;
        std::function<DirectX::XMFLOAT3()> getFloat3;
        std::function<void(const DirectX::XMFLOAT3&)> setFloat3;
        std::function<DirectX::XMFLOAT4()> getFloat4;
        std::function<void(const DirectX::XMFLOAT4&)> setFloat4;
    };

    class MoonComponent
    {
    public:
        virtual ~MoonComponent() = default;

        virtual ComponentType GetType() const = 0;
        virtual const char* GetDisplayName() const = 0;
        virtual std::vector<ComponentProperty> GetProperties() { return {}; }
        virtual void Update(MoonObject& owner, float dt) {}
        virtual void Normalize() {}
        virtual ComponentConflictGroup GetConflictGroup() const { return ComponentConflictGroup::None; }
        virtual bool AllowMultiple() const { return false; }
    };
}

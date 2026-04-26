#pragma once

#include "Source/Math/public/MoonMath.h"

#include <functional>
#include <string>
#include <utility>
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

#define MOON_COMPONENT(TypeName, DisplayName, ConflictGroup)                        \
    ComponentType GetType() const override { return ComponentType::TypeName; }      \
    const char* GetDisplayName() const override { return DisplayName; }             \
    ComponentConflictGroup GetConflictGroup() const override                         \
    {                                                                               \
        return ComponentConflictGroup::ConflictGroup;                               \
    }

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
        std::function<MoonVector3()> getFloat3;
        std::function<void(const MoonVector3&)> setFloat3;
        std::function<MoonVector4()> getFloat4;
        std::function<void(const MoonVector4&)> setFloat4;
    };

    class MoonComponent
    {
    public:
        virtual ~MoonComponent() = default;

        virtual ComponentType GetType() const = 0;
        virtual const char* GetDisplayName() const = 0;
        std::vector<ComponentProperty> GetProperties() const { return m_properties; }
        virtual void Update(MoonObject& owner, float dt) {}
        virtual void Normalize() {}
        virtual ComponentConflictGroup GetConflictGroup() const { return ComponentConflictGroup::None; }
        virtual bool AllowMultiple() const { return false; }

        void SetOwner(MoonObject* owner) { m_owner = owner; }
        MoonObject* GetOwner() const { return m_owner; }

    protected:
        void RegisterProperty(ComponentProperty property)
        {
            m_properties.push_back(std::move(property));
        }

    private:
        std::vector<ComponentProperty> m_properties;
        MoonObject* m_owner = nullptr;
    };

    // Base class for components that can be enabled/disabled at runtime.
    // Components inheriting from this base are skipped by Scene::Update and
    // by the scene-wide Find* lookups when disabled, and the Inspector
    // renders an enable/disable checkbox in front of their header.
    class MoonToggleableComponent : public MoonComponent
    {
    public:
        bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool enabled) { m_enabled = enabled; }

    private:
        bool m_enabled = true;
    };

    namespace MoonProp
    {
        // -- Float3 factories --

        inline ComponentProperty Float3(
            const char* name,
            MoonVector3& member,
            const char* format = "%.3f",
            float speed = 0.05f)
        {
            ComponentProperty prop;
            prop.name = name;
            prop.type = ComponentPropertyType::Float3;
            prop.format = format;
            prop.speed = speed;
            prop.getFloat3 = [&member]() { return member; };
            prop.setFloat3 = [&member](const MoonVector3& v) { member = v; };
            return prop;
        }

        inline ComponentProperty Float3(
            const char* name,
            MoonVector3& member,
            const char* format,
            float speed,
            float minValue,
            float maxValue)
        {
            ComponentProperty prop;
            prop.name = name;
            prop.type = ComponentPropertyType::Float3;
            prop.clamp = true;
            prop.format = format;
            prop.speed = speed;
            prop.minValue = minValue;
            prop.maxValue = maxValue;
            prop.getFloat3 = [&member]() { return member; };
            prop.setFloat3 = [&member](const MoonVector3& v) { member = v; };
            return prop;
        }

        inline ComponentProperty Float3(
            const char* name,
            std::function<MoonVector3()> getter,
            std::function<void(const MoonVector3&)> setter,
            const char* format = "%.3f",
            float speed = 0.05f)
        {
            ComponentProperty prop;
            prop.name = name;
            prop.type = ComponentPropertyType::Float3;
            prop.format = format;
            prop.speed = speed;
            prop.getFloat3 = std::move(getter);
            prop.setFloat3 = std::move(setter);
            return prop;
        }

        // -- Float factories --

        inline ComponentProperty Float(
            const char* name,
            float& member,
            const char* format = "%.3f",
            float speed = 0.1f)
        {
            ComponentProperty prop;
            prop.name = name;
            prop.type = ComponentPropertyType::Float;
            prop.format = format;
            prop.speed = speed;
            prop.getFloat = [&member]() { return member; };
            prop.setFloat = [&member](float v) { member = v; };
            return prop;
        }

        inline ComponentProperty Float(
            const char* name,
            float& member,
            const char* format,
            float speed,
            float minValue,
            float maxValue)
        {
            ComponentProperty prop;
            prop.name = name;
            prop.type = ComponentPropertyType::Float;
            prop.clamp = true;
            prop.format = format;
            prop.speed = speed;
            prop.minValue = minValue;
            prop.maxValue = maxValue;
            prop.getFloat = [&member]() { return member; };
            prop.setFloat = [&member](float v) { member = v; };
            return prop;
        }

        inline ComponentProperty Float(
            const char* name,
            std::function<float()> getter,
            std::function<void(float)> setter,
            const char* format = "%.3f",
            float speed = 0.1f)
        {
            ComponentProperty prop;
            prop.name = name;
            prop.type = ComponentPropertyType::Float;
            prop.format = format;
            prop.speed = speed;
            prop.getFloat = std::move(getter);
            prop.setFloat = std::move(setter);
            return prop;
        }

        inline ComponentProperty Float(
            const char* name,
            std::function<float()> getter,
            std::function<void(float)> setter,
            const char* format,
            float speed,
            float minValue,
            float maxValue)
        {
            ComponentProperty prop;
            prop.name = name;
            prop.type = ComponentPropertyType::Float;
            prop.clamp = true;
            prop.format = format;
            prop.speed = speed;
            prop.minValue = minValue;
            prop.maxValue = maxValue;
            prop.getFloat = std::move(getter);
            prop.setFloat = std::move(setter);
            return prop;
        }

        // -- Float4 factories --

        inline ComponentProperty Float4(
            const char* name,
            MoonVector4& member,
            const char* format = "%.3f",
            float speed = 0.01f)
        {
            ComponentProperty prop;
            prop.name = name;
            prop.type = ComponentPropertyType::Float4;
            prop.format = format;
            prop.speed = speed;
            prop.getFloat4 = [&member]() { return member; };
            prop.setFloat4 = [&member](const MoonVector4& v) { member = v; };
            return prop;
        }

        // -- Text factories --

        inline ComponentProperty Text(
            const char* name,
            std::function<std::string()> getter)
        {
            ComponentProperty prop;
            prop.name = name;
            prop.type = ComponentPropertyType::Text;
            prop.readOnly = true;
            prop.getText = std::move(getter);
            return prop;
        }

        inline ComponentProperty Text(
            const char* name,
            const std::string& member)
        {
            ComponentProperty prop;
            prop.name = name;
            prop.type = ComponentPropertyType::Text;
            prop.readOnly = true;
            prop.getText = [&member]() { return member; };
            return prop;
        }
    }
}

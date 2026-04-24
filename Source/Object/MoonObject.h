#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "Source/Object/MoonComponent.h"

namespace Object
{
    class MoonObject
    {
    public:
        explicit MoonObject(std::string name) : m_name(std::move(name)) {}

        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }

        template<typename T, typename... Args>
        T* AddComponent(Args&&... args)
        {
            static_assert(std::is_base_of<MoonComponent, T>::value, "T must derive from MoonComponent");

            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            if (!component->AllowMultiple() && HasComponent<T>())
            {
                return nullptr;
            }

            const ComponentConflictGroup conflictGroup = component->GetConflictGroup();
            if (conflictGroup != ComponentConflictGroup::None)
            {
                for (const auto& existingComponent : m_components)
                {
                    if (existingComponent->GetConflictGroup() == conflictGroup)
                    {
                        return nullptr;
                    }
                }
            }

            T* result = component.get();
            m_components.push_back(std::move(component));
            return result;
        }

        template<typename T>
        T* GetComponent()
        {
            static_assert(std::is_base_of<MoonComponent, T>::value, "T must derive from MoonComponent");

            for (const auto& component : m_components)
            {
                if (auto* typedComponent = dynamic_cast<T*>(component.get()))
                {
                    return typedComponent;
                }
            }
            return nullptr;
        }

        template<typename T>
        const T* GetComponent() const
        {
            static_assert(std::is_base_of<MoonComponent, T>::value, "T must derive from MoonComponent");

            for (const auto& component : m_components)
            {
                if (const auto* typedComponent = dynamic_cast<const T*>(component.get()))
                {
                    return typedComponent;
                }
            }
            return nullptr;
        }

        template<typename T>
        bool HasComponent() const
        {
            return GetComponent<T>() != nullptr;
        }

        const std::vector<std::unique_ptr<MoonComponent>>& GetComponents() const { return m_components; }

    private:
        std::string m_name;
        std::vector<std::unique_ptr<MoonComponent>> m_components;
    };
}

#include "Source/Object/Scene.h"

#include "Source/Object/CameraComponent.h"
#include "Source/Object/LightComponent.h"
#include "Source/Object/MeshComponent.h"
#include "Source/Object/MoonObject.h"
#include "Source/Object/TransformComponent.h"

namespace Object
{
    Scene::Scene() = default;
    Scene::~Scene() = default;

    MoonObject* Scene::SpawnObject(const std::string& name)
    {
        auto newObject = std::make_unique<MoonObject>(name);
        MoonObject* ptr = newObject.get();
        m_objects.push_back(std::move(newObject));
        return ptr;
    }

    void Scene::DestroyObject(MoonObject* object)
    {
        if (object == nullptr)
        {
            return;
        }

        // Detach from parent first
        if (MoonObject* parent = object->GetParent())
        {
            parent->RemoveChild(object);
        }

        // Recursively destroy children (copy list since RemoveChild mutates)
        const std::vector<MoonObject*> children = object->GetChildren();
        for (MoonObject* child : children)
        {
            DestroyObject(child);
        }

        m_objects.erase(
            std::remove_if(m_objects.begin(), m_objects.end(),
                [object](const auto& ptr) { return ptr.get() == object; }),
            m_objects.end());
    }

    void Scene::Clear()
    {
        m_objects.clear();
    }

    std::vector<MoonObject*> Scene::GetRootObjects() const
    {
        std::vector<MoonObject*> roots;
        for (const auto& object : m_objects)
        {
            if (object->GetParent() == nullptr)
            {
                roots.push_back(object.get());
            }
        }
        return roots;
    }

    MoonObject* Scene::FindMainCamera() const
    {
        for (const auto& object : m_objects)
        {
            if (object->HasComponent<TransformComponent>() && object->HasComponent<CameraComponent>())
            {
                return object.get();
            }
        }
        return nullptr;
    }

    MoonObject* Scene::FindDirectionalLight() const
    {
        for (const auto& object : m_objects)
        {
            const auto* lightComponent = object->GetComponent<LightComponent>();
            if (object->HasComponent<TransformComponent>() &&
                lightComponent != nullptr &&
                lightComponent->GetLightKind() == LightKind::Directional)
            {
                return object.get();
            }
        }
        return nullptr;
    }

    MoonObject* Scene::FindFirstRenderable() const
    {
        for (const auto& object : m_objects)
        {
            if (object->HasComponent<TransformComponent>() && object->HasComponent<MeshComponent>())
            {
                return object.get();
            }
        }
        return nullptr;
    }

    void Scene::Update(float dt)
    {
        for (const auto& object : m_objects)
        {
            for (const auto& component : object->GetComponents())
            {
                component->Update(*object, dt);
            }
        }
    }
}

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Object
{
    class MoonObject;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        MoonObject* SpawnObject(const std::string& name);
        void DestroyObject(MoonObject* object);
        void Clear();

        const std::vector<std::unique_ptr<MoonObject>>& GetObjects() const { return m_objects; }
        std::vector<MoonObject*> GetRootObjects() const;

        MoonObject* FindMainCamera() const;
        MoonObject* FindDirectionalLight() const;
        MoonObject* FindFirstRenderable() const;

        void Update(float dt);

    private:
        std::vector<std::unique_ptr<MoonObject>> m_objects;
    };
}

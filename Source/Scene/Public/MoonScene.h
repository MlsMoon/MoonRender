#pragma once
#include <vector>

#include "Source/GameObject/public/MoonObject.h"

class MoonScene
{
public:
    MoonScene();
    ~MoonScene();

private:
    std::vector<MoonObject*> moon_objects;
    MoonObject* selected_obj;

public:
    std::vector<MoonObject*> GetAllObjects();

    bool AddObject(MoonObject* object);
};

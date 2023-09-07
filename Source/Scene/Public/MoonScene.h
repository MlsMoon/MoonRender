#pragma once
#include <vector>

#include "Source/GameObject/public/MoonObject.h"

class MoonScene
{
public:
    MoonScene();
    ~MoonScene();

private:
    std::vector<MoonObject> moon_objects;

public:
    std::vector<MoonObject> GetAllObjects();
};

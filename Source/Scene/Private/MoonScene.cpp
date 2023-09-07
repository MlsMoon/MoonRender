#include "../Public/MoonScene.h"

MoonScene::MoonScene()
{
    moon_objects = std::vector<MoonObject>();
}

MoonScene::~MoonScene()
{

}


std::vector<MoonObject> MoonScene::GetAllObjects()
{
    return moon_objects;
}

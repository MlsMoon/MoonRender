#include "../Public/MoonScene.h"

#include "Source/GameObject/public/Camera.h"

MoonScene::MoonScene()
{
    moon_objects = std::vector<MoonObject*>();
    //添加默认摄像机
    auto* view_port_camera = new ViewPortCamera();
    AddObject(view_port_camera);
}

MoonScene::~MoonScene()
{
    for (MoonObject* item : moon_objects) {
        delete item;
    }
    moon_objects.clear();
}


std::vector<MoonObject*> MoonScene::GetAllObjects()
{
    return moon_objects;
}

bool MoonScene::AddObject(MoonObject* object)
{
    this->moon_objects.push_back(object);
    return true;
}

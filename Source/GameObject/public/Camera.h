#pragma once
#include "MoonObject.h"
#include "Source/EventSystem/public/EventCenter.h"

class Camera:public MoonObject
{
public:
    Camera();
    ~Camera();
    
    float fov;
    
};

class ViewPortCamera:public Camera
{
public:
    
};

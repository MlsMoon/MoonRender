#pragma once
#include <functional>
#include <memory>
#include "Source/Core/public/MoonRenderClass.h"
#include <unordered_map>
#include "string"
#include "any"

template<typename R>
using MoonFunctionPtr = std::function<void(R)>;

class EventCenter:public MoonRenderClass::Singleton<EventCenter>
{
public:
    static bool EventTrigger(const std::string& event_name,float param);
    static bool AddListener(const std::string& event_name,MoonFunctionPtr<float> register_event);

    EventCenter();
    ~EventCenter();
    
private:
    static std::unordered_map<std::string , MoonFunctionPtr<float> > register_events_map_float;
    
};


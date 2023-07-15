#pragma once
#include "Source/Core/public/MoonRenderClass.h"
#include <unordered_map>
#include "string"

template<typename R, typename... Args>
using FunctionPtr = R (*)(Args...);



class EventCenter:public MoonRenderClass::Singleton<EventCenter>
{
public:
    bool EventTrigger(std::string event_name);
    template<typename C, typename... Args>
    void AddListener(std::string event_name, FunctionPtr<C> register_event);
    EventCenter();
    ~EventCenter();
    
private:
    
    
};

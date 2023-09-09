#include "../public/EventCenter.h"

#include <utility>

// 定义静态成员变量
//instance
EventCenter* MoonRenderBaseClass::Singleton<EventCenter>::instance = nullptr;
MemberAccessor* MoonRenderBaseClass::Singleton<MemberAccessor>::instance = nullptr;
std::unordered_map<std::string, MoonFunctionPtr<float>> EventCenter::register_events_map_float;


bool EventCenter::EventTrigger(const std::string& event_name, float param)
{
    if (instance==nullptr)
        if(!EventCenter::CreateInstance<EventCenter>())
            return false;
    const auto func_ptr = EventCenter::register_events_map_float[event_name];
    func_ptr(param);
    return true;
}

bool EventCenter::AddListener(const std::string& event_name, MoonFunctionPtr<float> register_event)
{
    if (instance==nullptr)
        if(!CreateInstance<EventCenter>())
            return false;
    EventCenter::register_events_map_float[event_name] = std::move(register_event);
    return true;
}

EventCenter::EventCenter()
{
    
}

EventCenter::~EventCenter()
{
    
}

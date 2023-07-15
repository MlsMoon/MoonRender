#include "EventCenter.h"

// 定义一个函数类型别名
typedef void(*FunctionPointer)();

// bool EventCenter::EventTrigger(std::string event_name)
// {
//     auto it = registered_events.find(event_name);
//     if (it != registered_events.end()) {
//         void* void_ptr = it->second;
//         FunctionPointer func = reinterpret_cast<FunctionPointer>(void_ptr);
//         func();
//     }
// }
//
// void EventCenter::AddListener(std::string event_name, void* event)
// {
//     registered_events.insert(std::make_pair(event_name,(void*)event));
// }


EventCenter::EventCenter()
{
    
}

EventCenter::~EventCenter()
{
    
}

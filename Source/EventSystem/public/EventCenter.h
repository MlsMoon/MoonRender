#pragma once
#include <functional>
#include <memory>
#include "Source/AppWin/public/MoonRenderClass.h"
#include <unordered_map>
#include "string"
#include "any"
#include "Source/GameObject/public/MoonObject.h"

template<typename R>
using MoonFunctionPtr = std::function<void(R)>;


class MemberAccessor:public MoonRenderBaseClass::Singleton<MemberAccessor>
{
public:
    void registerMemberFloat(const std::string& className, const std::string& memberName, std::function<void(MoonObject*, float)> setter) {
        memberRegistry_[className + "::" + memberName] = setter;
    }
    
    void setMemberFloat(const std::string& className, const std::string& memberName, MoonObject* obj, float value) {
        auto key = className + "::" + memberName;
        auto it = memberRegistry_.find(key);
        if (it != memberRegistry_.end()) {
            it->second(obj, value);
        }
    }

private:
    std::unordered_map<std::string, std::function<void(MoonObject*, float)>> memberRegistry_;
};

#define REGISTER_MEMBER_FLOAT(className, memberName) \
MemberAccessor::GetInstance()->registerMemberFloat(#className, #memberName, [](MoonObject* obj, float value) { static_cast<className*>(obj)->memberName = value; })


class EventCenter:public MoonRenderBaseClass::Singleton<EventCenter>
{
public:
    static bool EventTrigger(const std::string& event_name,float param);
    static bool AddListener(const std::string& event_name,MoonFunctionPtr<float> register_event);

    EventCenter();
    ~EventCenter();
    
private:
    static std::unordered_map<std::string , MoonFunctionPtr<float> > register_events_map_float;
    
};


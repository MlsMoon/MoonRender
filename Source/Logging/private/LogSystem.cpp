#include "../public/LogSystem.h"

namespace Logging
{
    LogSystem::LogSystem()
    {
        if (LogSystem::CurrentLogSys!=nullptr)
            return;
        CurrentLogSys = this;
        log_content="";
    }

    LogSystem::~LogSystem()
    {

    }

    void LogSystem::Print(const std::string print_content)
    {
        if (LogSystem::CurrentLogSys==nullptr)
            return;
        CurrentLogSys->log_content = CurrentLogSys->log_content + print_content +"\n";
    }

    void LogSystem::Print(const MoonVector3 print_content)
    {
        if (LogSystem::CurrentLogSys==nullptr)
            return;
        LogSystem::Print("vec3:("+std::to_string(print_content.x)+","+std::to_string(print_content.y)+","+std::to_string(print_content.z)+")");
    }

    std::string LogSystem::GetLogContent()
    {
        return log_content;
    }
}



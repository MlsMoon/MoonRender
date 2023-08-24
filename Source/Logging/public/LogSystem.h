#pragma once
#include <string>
#include <directxmath.h>

#define MOON_LOG(content) Logging::LogSystem::Print(content)

namespace Logging
{
    class LogSystem
    {
    public:
        LogSystem();
        ~LogSystem();

        inline static LogSystem* CurrentLogSys = nullptr;
        static void Print(const std::string print_content);
        static void Print(const DirectX::XMFLOAT3 print_content);
        std::string GetLogContent();
        
    private:
        std::string log_content;
    };
}


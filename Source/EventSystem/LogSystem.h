#pragma once
#include <string>
#include <directxmath.h>

namespace EventSystem
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


#pragma once
#include <vector>


namespace MoonRenderClass
{
    template<typename T>
    class Singleton {
    public:
        static T& GetInstance()
        {
            if (instance == nullptr)
            {
                instance = new T();
            }
            return instance;
        }

        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

    protected:
        Singleton() = default;
        ~Singleton() = default;
        static T instance;
    };

    std::vector<int> a ;
}



#pragma once


namespace MoonRenderBaseClass
{
    template<typename T>
    class Singleton {
    public:
        static T* GetInstance()
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
        template<typename C>
        [[nodiscard]] static bool CreateInstance()
        {
            if (instance == nullptr)
            {
                instance = new C();
            }
            else
            {
                return false;
            }
            return true;
        }

        Singleton() = default;
        ~Singleton() = default;
        static T* instance;
    };
}





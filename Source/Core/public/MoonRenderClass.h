#pragma once

namespace MoonRenderClass
{
    class Singleton
    {
    public:
        static Singleton* GetInstance()
        {
            if (m_pInstance == nullptr)
            {
                m_pInstance = new Singleton();
            }
            return m_pInstance;
        }

    protected:
        Singleton()
        {
            //Singleton created.
        }
        ~Singleton()
        {
            //"Singleton destroyed."
            delete  m_pInstance;
        }
    private:
        static Singleton *m_pInstance;
    };


}



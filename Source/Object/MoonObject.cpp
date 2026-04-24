#include "Source/Object/MoonObject.h"

#include <algorithm>

namespace Object
{
    void MoonObject::SetParent(MoonObject* parent)
    {
        if (m_parent == parent)
        {
            return;
        }

        if (m_parent != nullptr)
        {
            m_parent->RemoveChild(this);
        }

        m_parent = parent;

        if (m_parent != nullptr)
        {
            m_parent->AddChild(this);
        }
    }

    void MoonObject::AddChild(MoonObject* child)
    {
        if (child == nullptr || child == this)
        {
            return;
        }

        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it == m_children.end())
        {
            m_children.push_back(child);
            if (child->GetParent() != this)
            {
                child->SetParent(this);
            }
        }
    }

    void MoonObject::RemoveChild(MoonObject* child)
    {
        if (child == nullptr)
        {
            return;
        }

        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it != m_children.end())
        {
            m_children.erase(it);
            if (child->GetParent() == this)
            {
                child->SetParent(nullptr);
            }
        }
    }
}

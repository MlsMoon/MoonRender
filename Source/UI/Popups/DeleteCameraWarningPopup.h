#pragma once

#include "Source/UI/Popups/PopupBase.h"

namespace MoonUI
{
    class DeleteCameraWarningPopup : public PopupBase
    {
    protected:
        const char* GetPopupId() const override;
        void DrawContent() override;
    };
}

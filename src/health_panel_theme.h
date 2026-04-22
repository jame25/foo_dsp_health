#pragma once
#include <windows.h>

class IHealthPanelTheme {
public:
    virtual ~IHealthPanelTheme() = default;
    virtual COLORREF get_bg_color() const = 0;
    virtual COLORREF get_text_color() const = 0;
    virtual bool is_dark_mode() const = 0;
};

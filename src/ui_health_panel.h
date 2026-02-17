#pragma once
#include "stdafx.h"
#include "guids.h"
#include "dsp_monitor.h"
#include <helpers/BumpableElem.h>
#include <libPPUI/win32_op.h>

class CHealthPanelWindow : public ui_element_instance, public CWindowImpl<CHealthPanelWindow> {
public:
    DECLARE_WND_CLASS_EX(TEXT("{A1B2C3D4-HEALTH-PANEL-WINDOW}"), CS_VREDRAW | CS_HREDRAW, (-1));

    CHealthPanelWindow(ui_element_config::ptr config, ui_element_instance_callback_ptr p_callback);
    ~CHealthPanelWindow();

    void initialize_window(HWND parent) { WIN32_OP(Create(parent) != NULL); }

    BEGIN_MSG_MAP_EX(CHealthPanelWindow)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_SIZE(OnSize)
    END_MSG_MAP()

    // ui_element_instance
    HWND get_wnd() { return *this; }
    void set_configuration(ui_element_config::ptr config) { m_config = config; }
    ui_element_config::ptr get_configuration() { return m_config; }
    void notify(const GUID& p_what, t_size p_param1, const void* p_param2, t_size p_param2size);

    // Static ui_element registration
    static GUID g_get_guid() { return guid_dsp_health_element; }
    static GUID g_get_subclass() { return ui_element_subclass_dsp; }
    static void g_get_name(pfc::string_base& out) { out = "DSP Health Monitor"; }
    static const char* g_get_description() { return "Real-time CPU monitoring per DSP node with enable/disable toggles."; }
    static ui_element_config::ptr g_get_default_configuration() {
        return ui_element_config::g_create_empty(g_get_guid());
    }

private:
    static constexpr UINT_PTR TIMER_REFRESH = 1;
    static constexpr UINT TIMER_INTERVAL_MS = 100;
    static constexpr int ROW_HEIGHT = 28;
    static constexpr int HEADER_HEIGHT = 8;
    static constexpr int FOOTER_HEIGHT = 50;
    static constexpr int TOGGLE_RADIUS = 6;
    static constexpr int TOGGLE_LEFT = 14;
    static constexpr int BAR_RIGHT_MARGIN = 59;
    static constexpr int DOTS_LEFT = 252;
    static constexpr int BAR_LEFT = 275;
    static constexpr int TEXT_LEFT = 32;

    int OnCreate(LPCREATESTRUCT);
    void OnDestroy();
    void OnPaint(CDCHandle);
    void OnTimer(UINT_PTR id);
    void OnLButtonDown(UINT flags, CPoint pt);
    BOOL OnEraseBkgnd(CDCHandle dc);
    void OnSize(UINT type, CSize size);

    void PaintContent(Gdiplus::Graphics& g, const CRect& rc);
    void DrawNodeRow(Gdiplus::Graphics& g, int y, int width, const DspNodeInfo& info);
    void DrawFooter(Gdiplus::Graphics& g, int y, int width);
    Gdiplus::Color CpuBarColor(double percent);
    bool IsDarkMode() const;
    COLORREF GetBgColor() const;
    COLORREF GetTextColor() const;

    ui_element_config::ptr m_config;
    ULONG_PTR m_gdiplus_token = 0;
    float m_scale = 1.0f;

protected:
    const ui_element_instance_callback_ptr m_callback;
};

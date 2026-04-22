#pragma once
#include "stdafx.h"
#include "guids.h"
#include "dsp_monitor.h"
#include "health_panel_theme.h"
#include <helpers/BumpableElem.h>
#include <libPPUI/win32_op.h>
#include <memory>

// Default UI theme: wraps ui_element_instance_callback_ptr.
// Declared here because it needs the foobar2000 SDK (via stdafx.h) to see
// ui_element_instance_callback_ptr and ui_color_* constants.
class DefaultUiTheme : public IHealthPanelTheme {
public:
    explicit DefaultUiTheme(ui_element_instance_callback_ptr cb) : m_callback(cb) {}
    COLORREF get_bg_color() const override { return m_callback->query_std_color(ui_color_background); }
    COLORREF get_text_color() const override { return m_callback->query_std_color(ui_color_text); }
    bool is_dark_mode() const override { return m_callback->is_dark_mode(); }
private:
    ui_element_instance_callback_ptr m_callback;
};

// Pure WTL painting window. Hosts no foobar2000 UI callback; reads colours and
// dark-mode state through an IHealthPanelTheme* set by its owner before the
// window is created.
class CHealthPanelView : public CWindowImpl<CHealthPanelView> {
public:
    DECLARE_WND_CLASS_EX(TEXT("{A1B2C3D4-HEALTH-PANEL-VIEW}"), CS_VREDRAW | CS_HREDRAW, (-1));

    CHealthPanelView() = default;
    explicit CHealthPanelView(IHealthPanelTheme* theme) : m_theme(theme) {}

    void set_theme(IHealthPanelTheme* theme) { m_theme = theme; }

    void initialize_window(HWND parent) { WIN32_OP(Create(parent) != NULL); }

    BEGIN_MSG_MAP_EX(CHealthPanelView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_SIZE(OnSize)
    END_MSG_MAP()

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

    IHealthPanelTheme* m_theme = nullptr;   // non-owning
    ULONG_PTR m_gdiplus_token = 0;
    float m_scale = 1.0f;
};

// Default UI adapter. IS a CHealthPanelView (inherits its painting + message
// map) and also implements ui_element_instance. Owns a DefaultUiTheme whose
// pointer it hands to the view base before the HWND is created.
//
// Inheritance (rather than composition) is required here: the SDK wraps
// CHealthPanelElement in ImplementBumpableElem<> + ui_element_instance_impl_helper<>,
// both of which reach for m_hWnd and operator HWND() on TImpl directly.
class CHealthPanelElement : public CHealthPanelView, public ui_element_instance {
public:
    CHealthPanelElement(ui_element_config::ptr config, ui_element_instance_callback_ptr callback);

    BEGIN_MSG_MAP_EX(CHealthPanelElement)
        CHAIN_MSG_MAP(CHealthPanelView)
    END_MSG_MAP()

    // ui_element_instance
    HWND get_wnd() override { return m_hWnd; }
    void set_configuration(ui_element_config::ptr config) override { m_config = config; }
    ui_element_config::ptr get_configuration() override { return m_config; }
    void notify(const GUID& what, t_size p1, const void* p2, t_size p2size) override;

    // Static ui_element registration
    static GUID g_get_guid() { return guid_dsp_health_element; }
    static GUID g_get_subclass() { return ui_element_subclass_dsp; }
    static void g_get_name(pfc::string_base& out) { out = "DSP Health Monitor"; }
    static const char* g_get_description() { return "Real-time CPU monitoring per DSP node with enable/disable toggles."; }
    static ui_element_config::ptr g_get_default_configuration() {
        return ui_element_config::g_create_empty(g_get_guid());
    }

protected:
    ui_element_instance_callback_ptr m_callback;

private:
    ui_element_config::ptr           m_config;
    std::unique_ptr<DefaultUiTheme>  m_theme_impl;
};

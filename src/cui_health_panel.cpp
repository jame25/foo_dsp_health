// This file intentionally does NOT include stdafx.h. Include-path rewiring
// (the CUI SDK vendoring prep) ensures CUI SDK headers and main SDK headers
// resolve to the same pfc / foobar2000 SDK. PCH is disabled for this file
// via the vcxproj so the CUI SDK's own include order is honoured.

#include "columns_ui-sdk/ui_extension.h"
#include "guids.h"
#include "health_panel_theme.h"
#include "ui_health_panel.h"
#include <functional>
#include <memory>

namespace {

// ---------- CuiTheme ----------

class CuiTheme : public IHealthPanelTheme {
public:
    COLORREF get_bg_color() const override { return m_helper.get_colour(cui::colours::colour_background); }
    COLORREF get_text_color() const override { return m_helper.get_colour(cui::colours::colour_text); }
    bool is_dark_mode() const override { return m_helper.is_dark_mode_active(); }
private:
    cui::colours::helper m_helper;
};

// ---------- CuiColourNotifier ----------

class CuiColourNotifier : public cui::colours::common_callback {
public:
    explicit CuiColourNotifier(std::function<void()> on_change)
        : m_on_change(std::move(on_change)) {
        cui::colours::manager::ptr api;
        if (fb2k::std_api_try_get(api)) {
            api->register_common_callback(this);
            m_api = api;
        }
    }

    ~CuiColourNotifier() {
        if (m_api.is_valid()) m_api->deregister_common_callback(this);
    }

    void on_colour_changed(uint32_t) const override { m_on_change(); }
    void on_bool_changed(uint32_t) const override { m_on_change(); }

private:
    std::function<void()>       m_on_change;
    cui::colours::manager::ptr  m_api;
};

// ---------- cui_health_panel ----------

class cui_health_panel : public uie::window {
public:
    // uie::extension_base
    const GUID& get_extension_guid() const override { return guid_dsp_health_cui_panel; }
    void get_name(pfc::string_base& out) const override { out = "DSP Health Monitor"; }
    void set_config(stream_reader*, t_size, abort_callback&) override {}
    void get_config(stream_writer*, abort_callback&) const override {}

    // uie::window
    void get_category(pfc::string_base& out) const override { out = "Panels"; }
    unsigned get_type() const override { return uie::type_panel; }
    bool get_description(pfc::string_base& out) const override {
        out = "Real-time CPU monitoring per DSP node with enable/disable toggles.";
        return true;
    }
    bool is_available(const uie::window_host_ptr&) const override { return true; }

    HWND create_or_transfer_window(HWND parent, const uie::window_host_ptr& host,
                                   const ui_helpers::window_position_t& position) override {
        m_host = host;
        if (!m_theme)    m_theme    = std::make_unique<CuiTheme>();
        if (!m_view)     m_view     = std::make_unique<CHealthPanelView>(m_theme.get());
        if (!m_notifier) m_notifier = std::make_unique<CuiColourNotifier>(
            [this]() { if (m_view && m_view->m_hWnd) m_view->Invalidate(FALSE); });

        m_view->initialize_window(parent);
        SetWindowPos(m_view->m_hWnd, nullptr,
                     position.x, position.y, position.cx, position.cy, SWP_NOZORDER);
        return m_view->m_hWnd;
    }

    void destroy_window() override {
        if (m_view && m_view->m_hWnd) m_view->DestroyWindow();
        m_view.reset();
        m_notifier.reset();
        m_theme.reset();
        m_host = uie::window_host_ptr{};
    }

    HWND get_wnd() const override { return m_view ? m_view->m_hWnd : nullptr; }

private:
    std::unique_ptr<CHealthPanelView>   m_view;
    std::unique_ptr<CuiTheme>           m_theme;
    std::unique_ptr<CuiColourNotifier>  m_notifier;
    uie::window_host_ptr                m_host;
};

static uie::window_factory<cui_health_panel> g_cui_health_panel_factory;

} // namespace

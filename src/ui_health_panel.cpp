#include "stdafx.h"
#include "ui_health_panel.h"
#include "dsp_chain_model.h"
#include <algorithm>
#include <cstring>

CHealthPanelWindow::CHealthPanelWindow(ui_element_config::ptr config, ui_element_instance_callback_ptr p_callback)
    : m_callback(p_callback), m_config(config) {
}

CHealthPanelWindow::~CHealthPanelWindow() {
}

int CHealthPanelWindow::OnCreate(LPCREATESTRUCT) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_gdiplus_token, &gdiplusStartupInput, NULL);
    SetTimer(TIMER_REFRESH, TIMER_INTERVAL_MS);

    UINT dpi = 96;
    HDC hdc = GetDC();
    if (hdc) {
        dpi = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(hdc);
    }
    m_scale = (float)dpi / 96.0f;
    if (m_scale < 1.0f) m_scale = 1.0f;

    return 0;
}

void CHealthPanelWindow::OnDestroy() {
    KillTimer(TIMER_REFRESH);
    if (m_gdiplus_token) {
        Gdiplus::GdiplusShutdown(m_gdiplus_token);
        m_gdiplus_token = 0;
    }
}

void CHealthPanelWindow::OnTimer(UINT_PTR id) {
    if (id == TIMER_REFRESH) {
        Invalidate(FALSE);
    }
}

BOOL CHealthPanelWindow::OnEraseBkgnd(CDCHandle dc) {
    return TRUE; // GDI+ handles all drawing
}

void CHealthPanelWindow::OnSize(UINT type, CSize size) {
    Invalidate(FALSE);
}

void CHealthPanelWindow::notify(const GUID& p_what, t_size p_param1, const void* p_param2, t_size p_param2size) {
    if (p_what == ui_element_notify_colors_changed || p_what == ui_element_notify_font_changed) {
        Invalidate();
    }
}

bool CHealthPanelWindow::IsDarkMode() const {
    return m_callback->is_dark_mode();
}

COLORREF CHealthPanelWindow::GetBgColor() const {
    return m_callback->query_std_color(ui_color_background);
}

COLORREF CHealthPanelWindow::GetTextColor() const {
    return m_callback->query_std_color(ui_color_text);
}

Gdiplus::Color CHealthPanelWindow::CpuBarColor(double percent) {
    if (percent > 80.0) return Gdiplus::Color(220, 60, 60);     // red
    if (percent > 50.0) return Gdiplus::Color(220, 180, 40);    // yellow
    return Gdiplus::Color(60, 180, 80);                          // green
}

void CHealthPanelWindow::OnPaint(CDCHandle) {
    CPaintDC paintDC(*this);
    CRect rc;
    GetClientRect(&rc);

    // Double-buffer via GDI+ Bitmap
    Gdiplus::Bitmap buffer(rc.Width(), rc.Height(), PixelFormat32bppARGB);
    Gdiplus::Graphics g(&buffer);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    PaintContent(g, rc);

    // Blit to screen
    Gdiplus::Graphics screenGraphics(paintDC.m_hDC);
    screenGraphics.DrawImage(&buffer, 0, 0);
}

void CHealthPanelWindow::PaintContent(Gdiplus::Graphics& g, const CRect& rc) {
    auto& monitor = DspMonitorService::get();

    // Background
    COLORREF bgCol = GetBgColor();
    Gdiplus::Color bg(GetRValue(bgCol), GetGValue(bgCol), GetBValue(bgCol));
    Gdiplus::SolidBrush bgBrush(bg);
    g.FillRectangle(&bgBrush, 0, 0, rc.Width(), rc.Height());

    COLORREF txtCol = GetTextColor();
    Gdiplus::Color textColor(GetRValue(txtCol), GetGValue(txtCol), GetBValue(txtCol));
    Gdiplus::SolidBrush textBrush(textColor);

    // DSP node rows
    int y = (int)(HEADER_HEIGHT * m_scale);
    size_t count = monitor.get_node_count();

    for (size_t idx = 0; idx < count; idx++) {
        DspNodeInfo info = monitor.get_node_info(idx);
        DrawNodeRow(g, y, rc.Width(), info);
        y += (int)(ROW_HEIGHT * m_scale);
    }

    if (count == 0) {
        Gdiplus::Font emptyFont(L"Segoe UI", 12.0f * m_scale, Gdiplus::FontStyleItalic, Gdiplus::UnitPixel);
        Gdiplus::Color dimColor(128, GetRValue(txtCol), GetGValue(txtCol), GetBValue(txtCol));
        Gdiplus::SolidBrush dimBrush(dimColor);
        g.DrawString(L"No active DSPs", -1, &emptyFont,
            Gdiplus::PointF((float)TEXT_LEFT * m_scale, (float)y + 4.0f * m_scale), &dimBrush);
        y += (int)(ROW_HEIGHT * m_scale);
    }

    // Extra spacing before footer
    y += (int)(8.0f * m_scale);

    // Footer
    DrawFooter(g, y, rc.Width());
}

void CHealthPanelWindow::DrawNodeRow(Gdiplus::Graphics& g, int y, int width, const DspNodeInfo& info) {
    COLORREF txtCol = GetTextColor();
    Gdiplus::Color textColor(GetRValue(txtCol), GetGValue(txtCol), GetBValue(txtCol));

    // Toggle circle
    float cx = (float)TOGGLE_LEFT * m_scale;
    float cy = (float)y + (float)ROW_HEIGHT * m_scale / 2.0f;
    float toggleRadius = (float)TOGGLE_RADIUS * m_scale;
    Gdiplus::Color toggleColor = info.enabled
        ? Gdiplus::Color(60, 180, 80)
        : Gdiplus::Color(128, GetRValue(txtCol), GetGValue(txtCol), GetBValue(txtCol));

    if (info.enabled) {
        Gdiplus::SolidBrush fillBrush(toggleColor);
        g.FillEllipse(&fillBrush, cx - toggleRadius, cy - toggleRadius,
            toggleRadius * 2.0f, toggleRadius * 2.0f);
    } else {
        Gdiplus::Pen pen(toggleColor, 1.5f * m_scale);
        g.DrawEllipse(&pen, cx - toggleRadius, cy - toggleRadius,
            toggleRadius * 2.0f, toggleRadius * 2.0f);
    }

    // DSP name
    Gdiplus::Font nameFont(L"Segoe UI", 12.0f * m_scale, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush textBrush(textColor);
    pfc::stringcvt::string_wide_from_utf8 wideName(info.name);
    g.DrawString(wideName, -1, &nameFont,
        Gdiplus::PointF((float)TEXT_LEFT * m_scale, (float)y + 5.0f * m_scale), &textBrush);

    // Three-dot config button
    if (info.enabled) {
        float dotsCx = (float)DOTS_LEFT * m_scale;
        float dotsCy = cy;
        float dotR = 1.5f * m_scale;
        float dotGap = 4.0f * m_scale;
        Gdiplus::Color dotsColor(160, GetRValue(txtCol), GetGValue(txtCol), GetBValue(txtCol));
        Gdiplus::SolidBrush dotsBrush(dotsColor);
        for (int d = -1; d <= 1; d++) {
            float dx = dotsCx + d * dotGap;
            g.FillEllipse(&dotsBrush, dx - dotR, dotsCy - dotR, dotR * 2.0f, dotR * 2.0f);
        }
    }

    // CPU bar (always visible for enabled DSPs)
    if (info.enabled) {
        int barMaxWidth = width - (int)(BAR_LEFT * m_scale) - BAR_RIGHT_MARGIN;
        if (barMaxWidth < 10) return;

        float barWidth = (float)(info.cpu_percent / 100.0 * barMaxWidth);
        if (barWidth > barMaxWidth) barWidth = (float)barMaxWidth;
        if (barWidth < 2.0f && info.cpu_percent > 0.0) barWidth = 2.0f;

        // Bar background
        Gdiplus::Color barBgColor(40, GetRValue(txtCol), GetGValue(txtCol), GetBValue(txtCol));
        Gdiplus::SolidBrush barBgBrush(barBgColor);
        g.FillRectangle(&barBgBrush, (int)(BAR_LEFT * m_scale), y + (int)(8.0f * m_scale), barMaxWidth, (int)(12.0f * m_scale));

        // Bar fill
        if (info.cpu_percent > 0.0) {
            Gdiplus::SolidBrush barBrush(CpuBarColor(info.cpu_percent));
            g.FillRectangle(&barBrush, (int)(BAR_LEFT * m_scale), y + (int)(8.0f * m_scale), (int)barWidth, (int)(12.0f * m_scale));
        }

        // Percentage text
        wchar_t pctText[16];
        swprintf_s(pctText, L"%.0f%%", info.cpu_percent);
        Gdiplus::Font pctFont(L"Segoe UI", 11.0f * m_scale, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::StringFormat rightAlign;
        rightAlign.SetAlignment(Gdiplus::StringAlignmentFar);
        Gdiplus::RectF pctRect((float)(width - BAR_RIGHT_MARGIN + 4), (float)y + 5.0f * m_scale, (float)BAR_RIGHT_MARGIN - 8, (float)ROW_HEIGHT * m_scale);
        g.DrawString(pctText, -1, &pctFont, pctRect, &rightAlign, &textBrush);
    } else {
        Gdiplus::Font dashFont(L"Segoe UI", 11.0f * m_scale, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::Color dimColor(128, GetRValue(txtCol), GetGValue(txtCol), GetBValue(txtCol));
        Gdiplus::SolidBrush dimBrush(dimColor);
        Gdiplus::StringFormat rightAlign;
        rightAlign.SetAlignment(Gdiplus::StringAlignmentFar);
        Gdiplus::RectF pctRect((float)(width - BAR_RIGHT_MARGIN + 4), (float)y + 5.0f * m_scale, (float)BAR_RIGHT_MARGIN - 8, (float)ROW_HEIGHT * m_scale);
        g.DrawString(L"--", -1, &dashFont, pctRect, &rightAlign, &dimBrush);
    }
}

void CHealthPanelWindow::DrawFooter(Gdiplus::Graphics& g, int y, int width) {
    auto& monitor = DspMonitorService::get();
    COLORREF txtCol = GetTextColor();
    Gdiplus::Color textColor(GetRValue(txtCol), GetGValue(txtCol), GetBValue(txtCol));
    Gdiplus::SolidBrush textBrush(textColor);

    // Separator line
    Gdiplus::Color sepColor(60, GetRValue(txtCol), GetGValue(txtCol), GetBValue(txtCol));
    Gdiplus::Pen sepPen(sepColor, 1.0f * m_scale);
    g.DrawLine(&sepPen, (int)(8.0f * m_scale), y, width - (int)(8.0f * m_scale), y);
    y += (int)(4.0f * m_scale);

    Gdiplus::Font labelFont(L"Segoe UI", 11.0f * m_scale, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

    // Total DSP load
    double totalCpu = monitor.get_total_cpu();
    wchar_t totalText[64];
    swprintf_s(totalText, L"Total DSP load: %.0f%%", totalCpu);
    g.DrawString(totalText, -1, &labelFont,
        Gdiplus::PointF(8.0f * m_scale, (float)y + 2.0f * m_scale), &textBrush);
}

void CHealthPanelWindow::OnLButtonDown(UINT flags, CPoint pt) {
    auto& monitor = DspMonitorService::get();
    size_t count = monitor.get_node_count();

    int y = (int)(HEADER_HEIGHT * m_scale);
    int scaledRowHalf = (int)(ROW_HEIGHT * m_scale / 2.0f);
    int scaledToggleLeft = (int)(TOGGLE_LEFT * m_scale);
    int scaledToggleRadius = (int)(TOGGLE_RADIUS * m_scale);
    int scaledDotsLeft = (int)(DOTS_LEFT * m_scale);
    int dotsHitHalf = (int)(8.0f * m_scale);
    int pad = (int)(4.0f * m_scale);
    for (size_t i = 0; i < count; i++) {
        int rowCenter = y + scaledRowHalf;

        // Toggle circle hit-test
        CRect toggleRect(
            scaledToggleLeft - scaledToggleRadius - pad,
            rowCenter - scaledToggleRadius - pad,
            scaledToggleLeft + scaledToggleRadius + pad,
            rowCenter + scaledToggleRadius + pad
        );
        if (toggleRect.PtInRect(pt)) {
            monitor.toggle_dsp_enabled(i);
            return;
        }

        // Three-dot config button hit-test (enabled DSPs only)
        DspNodeInfo info = monitor.get_node_info(i);
        if (info.enabled) {
            CRect dotsRect(
                scaledDotsLeft - dotsHitHalf,
                rowCenter - dotsHitHalf,
                scaledDotsLeft + dotsHitHalf,
                rowCenter + dotsHitHalf
            );
            if (dotsRect.PtInRect(pt)) {
                DspChainModel::show_dsp_config_by_guid(info.guid, m_hWnd);
                return;
            }
        }

        y += (int)(ROW_HEIGHT * m_scale);
    }
}

// ui_element factory
namespace {
    class ui_element_health_panel : public ui_element_impl_withpopup<CHealthPanelWindow> {};
    static service_factory_single_t<ui_element_health_panel> g_ui_element_health_panel_factory;
}

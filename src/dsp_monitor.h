#pragma once
#include "stdafx.h"
#include <SDK/dsp_manager.h>
#include <SDK/playback_stream_capture.h>
#include <map>
#include <cstring>

struct GUIDLess {
    bool operator()(const GUID& a, const GUID& b) const {
        return memcmp(&a, &b, sizeof(GUID)) < 0;
    }
};

struct DspNodeInfo {
    GUID guid;
    pfc::string8 name;
    double cpu_percent;
    double peak_percent;
    bool enabled;
};

class DspMonitor :
    public playback_stream_capture_callback
{
public:
    DspMonitor();
    ~DspMonitor();

    // dsp_config_callback - handled by wrapper class
    void on_core_settings_change(const dsp_chain_config& p_newdata);

    // Public API for the UI
    void on_chunk(const audio_chunk& chunk) override;

    // Public API for the UI
    size_t get_node_count() const;
    DspNodeInfo get_node_info(size_t index) const;
    double get_total_cpu() const;

    void refresh_chain();
    void toggle_dsp_enabled(size_t index);

private:
    dsp_manager m_shadow;
    dsp_chain_config_impl m_current_config;
    std::vector<DspNodeInfo> m_node_info;
    std::map<GUID, dsp_preset_impl, GUIDLess> m_disabled_presets;

    void update_node_info();
    void save_disabled_dsp(const GUID& guid);
    void restore_disabled_dsp(const GUID& guid);
};

class DspMonitorService {
public:
    static DspMonitor& get();
private:
    static DspMonitor s_instance;
};

#include "stdafx.h"
#include "dsp_monitor.h"

DspMonitor DspMonitorService::s_instance;

DspMonitor& DspMonitorService::get() {
    return s_instance;
}

DspMonitor::DspMonitor() {
}

DspMonitor::~DspMonitor() {
}

void DspMonitor::on_core_settings_change(const dsp_chain_config& p_newdata) {
    m_current_config.copy(p_newdata);
    m_shadow.set_config(p_newdata);
    update_node_info();
}

void DspMonitor::on_chunk(const audio_chunk& chunk) {
    if (!m_shadow.is_active()) return;

    dsp_chunk_list_impl chunk_list;
    audio_chunk* c = chunk_list.insert_item(0, chunk.get_used_size());
    c->copy(chunk);

    abort_callback_dummy noAbort;
    dsp_track_t track;
    m_shadow.run(&chunk_list, track, 0, noAbort);

    // Update node info from shadow timings
    for (size_t i = 0; i < m_shadow.get_timing_count() && i < m_node_info.size(); i++) {
        const auto& timing = m_shadow.get_timing(i);
        m_node_info[i].cpu_percent = timing.cpu_percent();
        double peak_pct = timing.buffer_duration_us > 0.0
            ? (timing.peak_duration_us / timing.buffer_duration_us) * 100.0
            : 0.0;
        m_node_info[i].peak_percent = peak_pct;
    }
}

void DspMonitor::refresh_chain() {
    dsp_chain_config_impl chain;
    dsp_config_manager::get()->get_core_settings(chain);
    m_current_config.copy(chain);
    m_shadow.set_config(chain);
    update_node_info();
}

void DspMonitor::update_node_info() {
    m_node_info.clear();
    
    // First add all currently enabled DSPs from the chain
    for (size_t i = 0; i < m_current_config.get_count(); i++) {
        DspNodeInfo info;
        info.guid = m_current_config.get_item(i).get_owner();
        dsp_entry::g_name_from_guid(info.name, info.guid);
        info.cpu_percent = 0.0;
        info.peak_percent = 0.0;
        info.enabled = true;

        m_node_info.push_back(info);
        // Remove from disabled map if present (in case it was added back externally)
        m_disabled_presets.erase(info.guid);
    }

    // Then add all previously disabled DSPs
    for (const auto& [guid, preset] : m_disabled_presets) {
        DspNodeInfo info;
        info.guid = guid;
        dsp_entry::g_name_from_guid(info.name, guid);
        info.cpu_percent = 0.0;
        info.peak_percent = 0.0;
        info.enabled = false;

        m_node_info.push_back(info);
    }
    
    // Sort alphabetically by name so indices match UI
    std::sort(m_node_info.begin(), m_node_info.end(), [](const DspNodeInfo& a, const DspNodeInfo& b) {
        return strcmp(a.name, b.name) < 0;
    });
}

void DspMonitor::toggle_dsp_enabled(size_t index) {
    if (index >= m_node_info.size()) return;
    
    const GUID& guid = m_node_info[index].guid;
    bool currently_enabled = m_node_info[index].enabled;
    
    if (currently_enabled) {
        save_disabled_dsp(guid);
    } else {
        restore_disabled_dsp(guid);
    }
}

void DspMonitor::save_disabled_dsp(const GUID& guid) {
    dsp_chain_config_impl chain;
    dsp_config_manager::get()->get_core_settings(chain);

    // Save the preset before removing, so we can restore it later
    size_t count = chain.get_count();
    pfc::bit_array_bittable mask(count);
    for (size_t i = 0; i < count; i++) {
        if (chain.get_item(i).get_owner() == guid) {
            m_disabled_presets[guid] = chain.get_item(i);
            mask.set(i, true);
        }
    }

    chain.remove_mask(mask);
    // set_core_settings triggers on_core_settings_change -> update_node_info,
    // so m_disabled_presets must already contain the guid at this point
    dsp_config_manager::get()->set_core_settings(chain);
}

void DspMonitor::restore_disabled_dsp(const GUID& guid) {
    auto it = m_disabled_presets.find(guid);
    if (it == m_disabled_presets.end()) return;

    dsp_preset_impl saved_preset = it->second;
    m_disabled_presets.erase(it);

    dsp_chain_config_impl chain;
    dsp_config_manager::get()->get_core_settings(chain);

    if (!chain.contains_dsp(guid)) {
        chain.insert_item(saved_preset, chain.get_count());
        dsp_config_manager::get()->set_core_settings(chain);
    }
}

size_t DspMonitor::get_node_count() const {
    return m_node_info.size();
}

DspNodeInfo DspMonitor::get_node_info(size_t index) const {
    if (index < m_node_info.size()) return m_node_info[index];
    return {};
}

double DspMonitor::get_total_cpu() const {
    double total = 0.0;
    for (const auto& n : m_node_info) total += n.cpu_percent;
    return total;
}

// Service factory for dsp_config_callback
namespace {
    class DspMonitorConfigCallback : public dsp_config_callback {
    public:
        void on_core_settings_change(const dsp_chain_config& p_newdata) override {
            DspMonitorService::get().on_core_settings_change(p_newdata);
        }
    };
    static service_factory_single_t<DspMonitorConfigCallback> g_dsp_monitor_callback_factory;
}

// initquit to register/unregister stream capture and load initial chain
namespace {
    class DspMonitorInit : public initquit {
    public:
        void on_init() override {
            playback_stream_capture::get()->add_callback(&DspMonitorService::get());
            DspMonitorService::get().refresh_chain();
        }
        void on_quit() override {
            playback_stream_capture::get()->remove_callback(&DspMonitorService::get());
        }
    };
    static initquit_factory_t<DspMonitorInit> g_dsp_monitor_init_factory;
}

#include "stdafx.h"
#include "dsp_chain_model.h"
#include <SDK/dsp_manager.h>

void DspChainModel::toggle_dsp(size_t chain_index) {
    dsp_chain_config_impl chain;
    dsp_config_manager::get()->get_core_settings(chain);

    if (chain_index >= chain.get_count()) return;

    const GUID guid = chain.get_item(chain_index).get_owner();
    chain.remove_item(chain_index);
    dsp_config_manager::get()->set_core_settings(chain);
}

void DspChainModel::remove_dsp(size_t chain_index) {
    dsp_chain_config_impl chain;
    dsp_config_manager::get()->get_core_settings(chain);

    if (chain_index >= chain.get_count()) return;

    chain.remove_item(chain_index);
    dsp_config_manager::get()->set_core_settings(chain);
}

void DspChainModel::insert_dsp(const dsp_preset& preset, size_t at_index) {
    dsp_chain_config_impl chain;
    dsp_config_manager::get()->get_core_settings(chain);

    size_t idx = (at_index <= chain.get_count()) ? at_index : chain.get_count();
    chain.insert_item(preset, idx);
    dsp_config_manager::get()->set_core_settings(chain);
}

bool DspChainModel::show_dsp_config(size_t chain_index, HWND parent) {
    dsp_chain_config_impl chain;
    dsp_config_manager::get()->get_core_settings(chain);

    if (chain_index >= chain.get_count()) return false;

    dsp_preset_impl preset = chain.get_item(chain_index);
    if (dsp_entry::g_show_config_popup(preset, parent)) {
        chain.replace_item(preset, chain_index);
        dsp_config_manager::get()->set_core_settings(chain);
        return true;
    }
    return false;
}

bool DspChainModel::show_dsp_config_by_guid(const GUID& guid, HWND parent) {
    dsp_chain_config_impl chain;
    dsp_config_manager::get()->get_core_settings(chain);

    for (size_t i = 0; i < chain.get_count(); i++) {
        if (chain.get_item(i).get_owner() == guid) {
            dsp_preset_impl preset = chain.get_item(i);
            if (dsp_entry::g_show_config_popup(preset, parent)) {
                chain.replace_item(preset, i);
                dsp_config_manager::get()->set_core_settings(chain);
                return true;
            }
            return false;
        }
    }
    return false;
}

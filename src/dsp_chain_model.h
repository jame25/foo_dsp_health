#pragma once
#include "stdafx.h"

class dsp_preset;

class DspChainModel {
public:
    static void toggle_dsp(size_t chain_index);
    static void remove_dsp(size_t chain_index);
    static void insert_dsp(const dsp_preset& preset, size_t at_index);
    static bool show_dsp_config(size_t chain_index, HWND parent);
    static bool show_dsp_config_by_guid(const GUID& guid, HWND parent);
};

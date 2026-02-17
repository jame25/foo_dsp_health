#include "stdafx.h"

DECLARE_COMPONENT_VERSION(
    "DSP Health Monitor",
    "0.1.0",
    "Real-time DSP chain CPU monitoring dashboard.\n"
    "Visualizes per-DSP-node CPU usage,\n"
    "and provides one-click enable/disable toggles.\n\n"
    "https://github.com/jame25/foo_dsp_health"
);

VALIDATE_COMPONENT_FILENAME("foo_dsp_health.dll");
FOOBAR2000_IMPLEMENT_CFG_VAR_DOWNGRADE;

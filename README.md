# foo_dsp_health

A foobar2000 component that provides a real-time DSP monitoring dashboard. Visualizes CPU usage per DSP node and offers one-click enable/disable toggles.

<img width="620" height="191" alt="foo_dsp_health" src="https://github.com/user-attachments/assets/23adf50c-b3fd-4d8f-90f8-78d805249676" />


## Features

- **Per-DSP CPU monitoring** - See how much processing time each DSP in your chain consumes, expressed as a percentage of real-time (100% = can't keep up)
- **Color-coded CPU bars** - Green (<50%), yellow (50-80%), red (>80%)
- **One-click toggles** - Enable/disable individual DSPs directly from the panel
- **DSP configuration** - Click the three-dot button next to a DSP name to open its configuration dialog
- **Dark mode support** - Automatically matches foobar2000's theme
- **Dockable panel** - Standard ui_element that can be placed anywhere in your layout

## How It Works

The component runs a **shadow DSP chain** - a second instrumented copy of your active DSP configuration. Audio captured via `playback_stream_capture` is fed through this shadow chain, which uses `QueryPerformanceCounter` to measure the processing time of each individual DSP node.

CPU% is calculated as the ratio of processing time to audio buffer duration. If a 10ms buffer takes 2ms to process, that DSP is at 20% CPU.

## Building

### Requirements

- Visual Studio 2022 (v143 toolset)
- Windows 10 SDK
- [foobar2000 SDK](https://www.foobar2000.org/SDK) (included in `lib/foobar2000_SDK/`)
- [WTL](https://sourceforge.net/projects/wtl/) (place in `lib/WTL/`)

### Steps

1. Clone this repository
2. Place the WTL headers in `lib/WTL/Include/` if not already present
3. Open `foo_dsp_health.sln` in Visual Studio 2022
4. Select `Release | x64` (or `Release | Win32` for 32-bit foobar2000)
5. Build the solution (Ctrl+Shift+B)

The output DLL will be in `x64\Release\foo_dsp_health.dll`.

## Installation

Copy `foo_dsp_health.dll` into your foobar2000 components folder:

- **Standard install:** `%PROGRAMFILES%\foobar2000\components\`
- **Portable install:** `<foobar2000 folder>\components\`
- **User components (v2.0+):** `%APPDATA%\foobar2000\user-components\foo_dsp_health\`

Restart foobar2000 after copying.

## Usage

1. Access the panel either via **View -> DSP -> DSP Health Monitor** or by right-clicking the UI layout area and adding the **DSP Health Monitor** panel
2. Play audio with DSPs active (configure in Preferences > Playback > DSP Manager)
3. The panel displays per-node CPU bars and toggle circles
4. Click the green circle to disable a DSP, click the hollow circle to re-enable
5. Click the three-dot button next to a DSP name to open its configuration dialog

## Project Structure

```
foo_dsp_health/
  src/
    main.cpp              Component registration
    dsp_monitor.h/.cpp    Shadow chain engine + DSP enable/disable
    dsp_chain_model.h/.cpp  DSP chain read/write operations
    ui_health_panel.h/.cpp  GDI+ dockable panel (ui_element)
    guids.h               Component GUIDs
    stdafx.h/.cpp         Precompiled header
  lib/
    foobar2000_SDK/       foobar2000 SDK (patched dsp_manager for timing)
    WTL/                  Windows Template Library headers
  foo_dsp_health.sln      Visual Studio solution
  foo_dsp_health.vcxproj  Visual Studio project
```

## SDK Modification

This component patches `lib/foobar2000_SDK/foobar2000/SDK/dsp_manager.h/.cpp` to add per-node `QueryPerformanceCounter` timing around each DSP's `run()` call. The patch adds a `node_timing_t` struct and public accessors while preserving the existing API.

## License
MIT

## Support Development

If you find these components useful, consider supporting development:

| Platform | Payment Methods |
|----------|----------------|
| **[Ko-fi](https://ko-fi.com/Jame25)** | Cards, PayPal |
| **[Stripe](https://buy.stripe.com/3cIdR874Bg1NfRdaJf1sQ02)** | Alipay, Cards, Apple Pay, Google Pay |

Your support helps cover development time and enables new features. Thank you! 🙏

---

## 支持开发

如果您觉得这些组件有用，请考虑支持开发：

| 平台 | 支付方式 |
|------|----------|
| **[Ko-fi](https://ko-fi.com/Jame25)** | 银行卡、PayPal |
| **[Stripe](https://buy.stripe.com/dRmcN474B8zlfRd2cJ1sQ01)** | 支付宝、银行卡、Apple Pay、Google Pay |

您的支持有助于支付开发时间并实现新功能。谢谢！🙏

---

**Feature Requests:** Paid feature requests are available for supporters. [Contact me on Telegram](https://t.me/jame25) to discuss.

**功能请求：** 为支持者提供付费功能请求。[请在 Telegram 上联系我](https://t.me/jame25) 进行讨论。


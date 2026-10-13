# MPC PD LAB — Technical plan

Target: original Akai MPC One, Hakai 3.9.1, ARMv7 Linux. Independent from MPC CV LAB.

## Objective
Create standalone native VST2 plugins from Pure Data / Organelle patches, starting with a stereo delay. Compare two backends:
- **libpd**: embed Pure Data runtime.
- **hvcc**: compile compatible Pure Data patches into C/C++.

VST2 infrastructure reference: https://github.com/sd88me/mpc-vst-plugins
Backends: https://github.com/libpd/libpd and https://github.com/Wasted-Audio/hvcc

## First prototype
Stereo delay with time, feedback, wet/dry, lowpass cutoff, modulation depth and modulation rate. Use vanilla Pd only initially, without Organelle-specific externals.

## Performance protocol
Target 44.1 kHz / 128 frames per audio block (~2.90 ms). This is a block deadline, not the plugin's CPU usage.

Compare the same patch, audio input, buffer size, parameters, and host conditions. Record processing time per block (median, p95, maximum), RAM, glitches/xruns and stability with:
1. One plugin instance.
2. Four plugin instances.
3. Realistic MPC session with other tracks/plugins.

Test continuous operation and rapid parameter changes. Avoid blocking or synchronous logging on the audio thread. Verify preset/project reload.

## Milestones
1. Confirm build toolchain and wrapper compatibility.
2. Create a minimal vanilla Pd stereo delay patch and desktop reference.
3. Build and offline-test libpd VST2 ARMv7.
4. Build and offline-test hvcc VST2 ARMv7.
5. Add MPC touchscreen/Q-Link skin.
6. Deploy to MPC One after user approval and benchmark.
7. Select an Organelle patch and audit external dependencies.

## Current status
Desktop libpd adapter integration is verified with engine and hosted audio smoke tests
at 44.1 kHz / 128 frames; see LIBPD_ADAPTER.md for results. ARMv7 packaging
and hardware benchmarking are next. No plugin has been deployed or tested on the MPC.

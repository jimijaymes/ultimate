# Ultimate Channel Strip — User Manual (Development Edition 0.1)

**Status:** Initial implementation; Windows build and audio behaviour require verification. Hardware-inspired models are approximations, not measured reproductions.

## Signal flow
Input trim → Preamp (transformer + circuit) → EQ → Dynamics (optical/FET) → Tape + output transformer → Master Mix → Output trim.

Each of the four sections has its own On/Off switch. Master Mix blends the processed signal with an input-trimmed, latency-aligned dry path.

## 1. Preamp
**Input Transformer:** Off, Neve 10468 Input, Neve LO1166 Output, API 2622 + 2520, API 2520 + 2503, 1176 UTC Input, 1176 Class A Output, 1176 Rev F Output.
**Circuit:** Off, Discrete Class A, Class AB, Op-Amp, Triode Tube, Pentode Tube.
**Transformer Drive / Circuit Drive:** −24 to +24 dB each; both receive the Master Drive offset. **Character:** 0–100%. **Bias:** −100 to +100%.

## 2. Equaliser
**EQ Model:** Neve 1073, SSL G Series, API 560.
Neve and SSL currently use approximate four-band digital EQs: Low shelf (30–350 Hz, ±15 dB), Low Mid bell (100–2,000 Hz, ±15 dB), High Mid bell (800–9,000 Hz, ±15 dB), High shelf (3–16 kHz, ±15 dB). HPF: 20–300 Hz (20 Hz is neutral).
API 560 uses ten independent ±12 dB bands at 31, 63, 125, 250, 500 Hz, 1, 2, 4, 8 and 16 kHz. It is currently a fixed-Q approximation, not a measured proportional-Q hardware model.

## 3. Dynamics
**DYNAMICS ON:** bypasses both compressors. **OPTO ON** and **FET ON:** independently enable each compressor. **FET FIRST:** reverses compressor order.
Optical controls: Peak Reduction 0–100, Makeup −12 to +24 dB, Compress 3:1 / Limit 20:1, Enhanced / Classic T4 response, LA-2A Slow / LA-3A Fast release.
FET controls: Drive 0–100, Attack 0–100, Release 0–100, ratios 4:1 / 8:1 / 12:1 / 20:1 / All Buttons, 1176 Classic / Extended timing. The initial All Buttons model is a simplified high-ratio/wider-knee approximation.
**Shared Sidechain HPF:** 20–400 Hz, detector only; it does not directly filter audible bass.
**Important:** The initial Ultimate dynamics are simplified and do not yet contain every component of standalone OptoFET's nonlinear processing.

## 4. Tape and Output Transformer
**Tape:** Off, Studer A80 15 IPS, Studer A80 30 IPS, Nakamichi Dragon Metal, Studer J37 Abbey Road 15 IPS, Budget Cassette Ferric, Budget Cassette Metal.
**Tape Drive:** −24 to +24 dB.
**Output Transformer:** same eight model choices as the preamp transformer; **Output Transformer Drive:** −24 to +24 dB. Both drives receive the Master Drive offset.
**Not yet included:** Saturator's separate wow/flutter transport-delay feature.

## 5. Master
**Input:** −24 to +24 dB. **Output:** −24 to +24 dB.
**Global Saturation Drive Offset:** −24 to +24 dB, added to the preamp transformer, circuit, tape and output transformer drives; it does not change FET Drive.
**Mix:** 0–100% (dry to wet).
**Oversampling:** 1×, 2×, 4×, 8×. Fixed reported latency with separate wet/dry delay alignment. Higher oversampling costs more CPU.

## Suggested starting points
- **Vocals:** Neve input transformer + Class A, gentle Neve EQ, optical compression, subtle tape.
- **Electric guitar:** API input transformer + Class AB, SSL EQ, moderate FET compression and tape.
- **Drum bus:** API 560, FET at 4:1, restrained tape and 60–80% Mix.
- **Mix bus:** minimal drive, gentle EQ/optical compression, 4× oversampling, level-match to bypass.

## Development status and limitations
- Verify compilation, parameter automation, session recall, mono/stereo operation, CPU use, latency and Mix alignment.
- Neve/SSL/API EQ modes are voicings, not circuit-accurate hardware replicas.
- Standalone OptoFET improvements must be explicitly ported and tested.
- Saturator's wow/flutter is not yet implemented in Ultimate.

## Algorithm update policy
Ultimate is the integration target for relevant improvements in standalone Saturator and OptoFET. For each change: identify the source commit, review the affected Ultimate algorithm, port compatible DSP, test builds and audio, update this manual, and record the Ultimate commit. Synchronisation is **not automatic**.

See [ALGORITHM_SYNC.md](ALGORITHM_SYNC.md) for the update checklist.

# Ultimate Algorithm Synchronisation Policy

Ultimate is the integration target for compatible improvements from the standalone plugins. GitHub does **not** automatically copy changes between repositories.

## Sources and mappings
- `jimijaymes/saturator`: preamp input transformer, circuit, tape and output transformer, oversampling and fixed-latency wet/dry blend.
- `jimijaymes/optofet`: optical compressor, FET compressor, sidechain HPF, FET timing, makeup and order/bypass logic.
- EQ models (Neve 1073 / SSL G Series / API 560): maintained in Ultimate until separate implementations exist.
- Other plugins (FET, OptoValve, Guitar Polisher, Compander): review for useful improvements, but do not silently replace the intended Ultimate models.

## Every relevant algorithm change
1. Identify source repository, commit SHA and affected DSP stage.
2. Compare source algorithm against Ultimate implementation; do not assume identical state or sample-rate handling.
3. Port the compatible algorithm, preserving independent stage state and section bypass.
4. Check gain staging, sidechain detector behaviour, latency, oversampling and host automation.
5. Run Windows VST3 build and verify audio in a DAW before calling the change complete.
6. Update `USER_MANUAL.md` and the change log below.

## Current differences / backlog
- Ultimate dynamics use simplified optical and FET algorithms, not a complete standalone OptoFET port.
- Ultimate tape stage omits Saturator's wow/flutter transport-delay processing.
- Neve 1073, SSL G and API 560 EQs are approximate digital voicings, not measured hardware responses.
- Test mode-switch smoothing and wet/dry phase alignment.

## Change log
| Date | Source | Ultimate change | Build / test |
|---|---|---|---|
| 2026-11-21 | Initial standalone designs | Four-section Ultimate 0.1 implementation | Build/audio verification pending |
| 2026-11-21 | Synchronisation policy | Added manual and source-to-Ultimate update checklist | Documentation only |

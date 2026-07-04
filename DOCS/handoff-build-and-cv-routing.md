# Handoff: Build Toolchain And CV Routing

## Current Handoff: July 4, 2026

Stop point: the firmware is now based more closely on Tom Whitwell official pitch behavior while
keeping our chord-bank features. The current chord CV calibration has been hardware-tested as
**bang on**, with velocity `75` as chord 1 and all 12 chords reachable.

Latest hex:

```text
\\wsl$\nymphscore_lite\home\nymph\Chord-Organ-starmandeluxe-ChordBanks\Chord-Organ\build\teensy.avr.teensy31\Chord-Organ.ino.hex
```

Latest SHA256:

```text
a394afb6c8649be0db186c187231ad80bb1662881cf405a7205ae2e34ae9b7a1
```

Latest verified constants:

```cpp
#define CHORD_CV_PIN 6
#define ROOT_CV_PIN 8
#define CHORD_CV_VALUE_RANGE 88
#define CHORD_CV_INPUT_OFFSET -7120
#define CHORD_CV_VALUE_BASE 1
#define ROOT_CV_BASE_OFFSET 24
#define ROOT_CV_VALUE_RANGE 40
```

Current intended behavior:

```text
Button        -> waveform select
Chord knob    -> bank select from actual CHORD*.TXT files on SD
Chord CV      -> chord select, calibrated for Oxi velocity lane as 1-based values
Root CV       -> pitch/root, direct and snappy
Root knob     -> fine-tune offset around the pitch CV
Bank LEDs     -> show selected bank until waveform is changed
```

Current code notes:

- `readAnalogSettled(pin)` now does a single `analogRead(pin)`, matching official behavior more
  closely.
- Chord CV calibration that worked before velocity-lane shifting: `CHORD_CV_VALUE_RANGE 88`,
  `CHORD_CV_INPUT_OFFSET 32`.
- Chord CV is now velocity-lane friendly with `CHORD_CV_VALUE_BASE 1`: value `1` selects chord
  `0`, value `13` selects chord `12`.
- Oxi One User Manual v5.0 says default velocity is `75`.
- The `CHORD_CV_INPUT_OFFSET -4704` test made velocity `50` the first chord in practice.
- The `CHORD_CV_INPUT_OFFSET -6304` test made velocity `66` the first chord in practice.
- The `CHORD_CV_VALUE_RANGE 88` / `CHORD_CV_INPUT_OFFSET -7204` test shifted cleanly starting at
  velocity `77`, but only produced 11 distinct chords.
- The `CHORD_CV_VALUE_RANGE 96` / `CHORD_CV_INPUT_OFFSET -7204` test still started at `77`, made
  chord 5 shaky, and still only produced 11 distinct chords. Do not keep `96` as the baseline.
- The `CHORD_CV_VALUE_RANGE 88` / `CHORD_CV_INPUT_OFFSET -7040` test had a shaky chord boundary
  around velocity `74-75`.
- Current keeper build keeps `CHORD_CV_VALUE_RANGE 88` and shifts slightly upward with
  `CHORD_CV_INPUT_OFFSET -7120`. Hardware test result: bang on, velocity `75` starts chord 1,
  and all 12 chords are reachable.
- Root CV smoothing/filter block was removed from `checkInterface()`.
- Root pot smoothing/filter block was also removed from `checkInterface()`.
- `ROOT_CV_VALUE_RANGE` was changed from `39` to `40` for the current test build.
- Pitch generation now uses `MIDI_TO_FREQ[noteNumber] * rootFineTuneMultiplier`, so root CV changes
  use the official-style lookup table instead of calling `pow()` for every voice on every pitch
  change.
- Root knob fine tuning still uses `pow()`, but only to update `rootFineTuneMultiplier` when the knob
  changes.
- The user wants the module **as snappy as possible**. Avoid adding lag, smoothing, hysteresis,
  latches, or protection unless explicitly asked.

Remaining issue:

```text
Flash and test whether the official-style lookup-table pitch engine removes the delay.
```

Likely next places to inspect:

- `loop()`: changes call `updateAmpAndFreq()`, then `updateFrequencies()` inside the `changed` block.
- `glide`: if `settings.glide` is true from SD/defaults, pitch will intentionally smear.
- `updateFrequencies()`: confirm it jumps immediately when `gliding` is false.
- Bank/chord/root changes all share the `changed` update path; if delay persists after smoothing
  removal, suspect glide/config or oscillator update timing, not CV mapping.

Important collaboration note:

- The user is testing hardware live. When they report behavior, do **not** edit/build unless they
  explicitly ask to try/build/fix. Discuss first.
- Keep changes tiny and named by one parameter at a time.
- Always verify generated build source and hex hash before reporting a hex is ready.

This repo has been built from WSL using the Arduino IDE installed under the Windows `babyj`
profile. Do not look under `/mnt/c/Users/nymph` for the Arduino toolchain.

## Actual Arduino Toolchain

Arduino IDE:

```text
C:\Users\babyj\AppData\Local\Programs\Arduino IDE\Arduino IDE.exe
```

Bundled Arduino CLI:

```text
C:\Users\babyj\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe
```

Teensy platform/core:

```text
C:\Users\babyj\AppData\Local\Arduino15\packages\teensy\hardware\avr\1.59.0
```

Teensy compiler:

```text
C:\Users\babyj\AppData\Local\Arduino15\packages\teensy\tools\teensy-compile\11.3.1\arm\bin\arm-none-eabi-g++.exe
```

Teensy tools:

```text
C:\Users\babyj\AppData\Local\Arduino15\packages\teensy\tools\teensy-tools\1.59.0
```

Board FQBN:

```text
teensy:avr:teensy31
```

## Compile Command

Use the bundled Arduino CLI from PowerShell. The most reliable workflow today was **not** compiling
directly into the WSL build path. Instead, copy the sketch into a correctly named Windows temp
folder, compile there, then copy the output artifacts back to the WSL build folder.

Temp sketch folder:

```text
C:\Users\babyj\AppData\Local\Temp\Chord-Organ
```

WSL path:

```text
/mnt/c/Users/babyj/AppData/Local/Temp/Chord-Organ
```

Copy these files into the temp sketch folder before compiling:

```text
Chord-Organ.ino
Settings.cpp
Settings.h
Waves.h
```

Compile command:

```powershell
& 'C:\Users\babyj\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe' compile `
  --fqbn teensy:avr:teensy31 `
  --build-path 'C:\Users\babyj\AppData\Local\Temp\Chord-Organ\build-win' `
  --output-dir 'C:\Users\babyj\AppData\Local\Temp\Chord-Organ\out' `
  'C:\Users\babyj\AppData\Local\Temp\Chord-Organ'
```

Expected output hex:

```text
C:\Users\babyj\AppData\Local\Temp\Chord-Organ\out\Chord-Organ.ino.hex
```

Copy output artifacts back to:

```text
/home/nymph/Chord-Organ-starmandeluxe-ChordBanks/Chord-Organ/build/teensy.avr.teensy31
```

## Current CV Routing Decision

The firmware should use the upstream Chord Organ CV pin routing:

```cpp
#define CHORD_CV_PIN 6
#define ROOT_CV_PIN 8
```

Reason: Radio Music schematic maps the two analogue CV paths to Teensy analog pins:

```text
TIME_CV    -> PIN20/A6
CHANNEL_CV -> PIN22/A8
```

Upstream Chord Organ uses analog `6` for Chord CV and analog `8` for Root CV. Swapping these makes
the physical Chord CV jack affect root, which is the bug we just hit.

## Current Range Decision

Root CV:

```cpp
#define ROOT_CV_VALUE_RANGE 40
```

Reason: local Modwiggler/forum notes supported `39` as the Chord Organ Root CV range for 1V/oct
tracking, but live testing suggested trying `40`. Latest built firmware uses `40`.

Chord CV:

```cpp
#define CHORD_CV_VALUE_RANGE 88
#define CHORD_CV_INPUT_OFFSET 32
#define CHORD_CV_SLOT_COUNT 12
```

Reason: live Oxi MOD lane testing found `88 + 32` was the first good-feeling calibration. Earlier
tests:

```text
39       -> too slow/wide
64       -> patterned wobble
80       -> shaky low, good upper
80 +128  -> good edges, bad middle
88       -> 0-2 shaky, 3-12 good
88 +32   -> reported as success / "we did it"
96       -> shaky cluster around 6-8
128      -> too twitchy
```

## Do Not Repeat This Mistake

The existing build artifacts mention:

```text
C:\Users\babyj\AppData\Local\arduino\sketches\...
```

That points to the Windows user profile that owns the Arduino install. If a future build says the
toolchain is missing, check `C:\Users\babyj` first.

## Incident Note: July 2, 2026

This was an enormous waste of time and should not happen again.

What went wrong:

- The build tools were incorrectly searched for under `/mnt/c/Users/nymph`.
- The actual Arduino install was under `/mnt/c/Users/babyj`.
- The first Windows Arduino CLI compile against the WSL build folder reported success but produced
  zero-byte `.hex`, `.elf`, `.lst`, `.sym`, and `.eep` files.
- A temporary Windows sketch folder was used because Arduino CLI produced valid output there, but
  the first temp folder was named `ChordOrganBuildFix`, which failed because Arduino requires the
  sketch folder name to match `Chord-Organ.ino`.

Correct recovery:

- Use the bundled Arduino CLI from the `babyj` profile.
- If compiling directly into the WSL path creates zero-byte outputs, compile from a correctly named
  Windows temp sketch folder:

```text
C:\Users\babyj\AppData\Local\Temp\Chord-Organ
```

- Then copy the real compiled outputs back to the normal WSL target folder:

```text
\\wsl$\nymphscore_lite\home\nymph\Chord-Organ-starmandeluxe-ChordBanks\Chord-Organ\build\teensy.avr.teensy31
```

- Always verify the hex size is non-zero before saying it is ready.

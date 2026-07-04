# Chord CV Mapping Note

This note records the current tested CV behaviour for the custom Chord Organ bank firmware.

## Current Behaviour

```text
Button      -> waveform select
LEDs        -> waveform by default
Chord knob  -> bank select from CHORD*.TXT files on SD
Chord CV    -> chord select within the current bank
Root CV     -> pitch/root only
Root knob   -> smooth +/-6 semitone root trim
```

The Root-CV bank-select experiment was scrapped. It was not reliable enough and interfered with
pitch behaviour.

## Chord CV

Chord CV is tuned for the Oxi One velocity lane in `Not Gated` mode.

Tested keeper mapping:

```text
velocity 75 = chord 1
velocity 76 = chord 2
velocity 77 = chord 3
...
velocity 86 = chord 12
```

This gives one Oxi velocity value per chord. It is intentionally tight because it makes chord
programming simple and repeatable.

Current firmware constants:

```cpp
#define CHORD_CV_SLOT_COUNT 12
#define CHORD_CV_VALUE_RANGE 88
#define CHORD_CV_INPUT_OFFSET -7120
#define CHORD_CV_VALUE_BASE 1
```

Hardware test result:

```text
88 / -7120 / base 1 = bang on, 12 chords reachable
```

Failed / rejected tests:

```text
88 / -7204 / base 1 = clean start around 77, but only 11 distinct chords
96 / -7204 / base 1 = still only 11 chords, chord 5 shaky
88 / -7040 / base 1 = near 75, but 74-75 boundary was shaky
```

The Chord CV read is direct: no smoothing, no filter, no latch, no hysteresis.

## Bank Selection

The chord knob selects banks across the SD card's loaded chord files.

```text
CHORD00.TXT
CHORD01.TXT
CHORD02.TXT
...
```

Files are sorted by filename before loading. The firmware uses the number of `CHORD*.TXT` files
actually present on the SD card, up to 16 banks.

Examples:

```text
4 bank files  = 4 wide knob zones
8 bank files  = 8 knob zones
16 bank files = 16 knob zones
```

`CHORDORG.TXT` is ignored by the bank loader.

The knob is mapped across a narrowed ADC range to make the ends easier to reach:

```cpp
#define BANK_POT_MIN 384
#define BANK_POT_MAX 7808
```

On startup, the firmware reads the physical chord knob for the initial bank. This means the apparent
bank is naturally sticky across reboot if the knob has not moved.

## LED Behaviour

Startup defaults to waveform LED view.

When the chord knob selects a bank, LEDs show the bank index in zero-index binary and stay in bank
view until the waveform button is pressed.

LED order is left to right:

```text
bank 0  = 0000
bank 1  = 1000
bank 2  = 0100
bank 3  = 1100
bank 4  = 0010
...
bank 15 = 1111
```

## Root CV And Root Knob

Root CV controls pitch/root only.

Current constants:

```cpp
#define ROOT_CV_BASE_OFFSET 24
#define ROOT_CV_VALUE_RANGE 40
#define ROOT_KNOB_OFFSET_RANGE 12
#define ROOT_KNOB_OFFSET_CENTER 6
```

The root knob is smooth, not stepped:

```text
fully CCW = about -6 semitones
noon      = centered
fully CW  = about +6 semitones
```

Pitch/root reads are also direct and unsmoothed for snappy changes.

## Useful Hardware Notes

Radio Music / Chord Organ hardware should be treated as a `0V` to `+5V` CV input system for these
main CV inputs.

The local forum notes supported a `39-ish` range for 1V/oct Root CV calibration. This firmware is
currently using `40`, because that tested in tune on the hardware.

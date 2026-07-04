# Chord Organ CV Banks

Custom firmware for Music Thing Modular Chord Organ / Radio Music hardware on Teensy 3.1.

This fork keeps the Chord Organ sound engine, adds SD-card chord banks, and makes the module easier
to sequence from an Oxi One: the chord knob selects banks, the Chord CV input selects chords, and the
Root CV input stays dedicated to pitch/root.

## Quick Behaviour Guide

```text
Chord knob  Changes bank across the CHORD*.TXT files actually on the SD card
Chord CV    Changes chord from Oxi velocity 75 upward, one chord per velocity step
Root knob   Tunes across a smooth 1-octave range, with C centered at 12 o'clock
Root CV     Controls root pitch at 1V/oct, using the tested range value 40
Button      Changes waveform
LEDs        Show waveform normally, then switch to bank display when the chord knob changes bank
```

Chord CV mapping:

```text
velocity 75 = chord 1
velocity 76 = chord 2
velocity 77 = chord 3
...
velocity 86 = chord 12
```

Bank selection follows the SD card. If the card has 4 bank files, the chord knob spans those 4
banks. If the card has 16 bank files, the chord knob spans those 16 banks.

## Flash

Flash this hex to a Teensy 3.1:

```text
Collateral/Hex File/Chord-Organ.ino.hex
```

Latest tested SHA256:

```text
a394afb6c8649be0db186c187231ad80bb1662881cf405a7205ae2e34ae9b7a1
```

## Controls

```text
Button      Change waveform
LEDs        Show waveform by default
Chord knob  Select SD chord bank
Chord CV    Select chord within the current bank
Root knob   Smooth 1-octave fine root trim
Root CV     Root pitch only
```

When the chord knob changes bank, the LEDs switch to bank display and stay there until the waveform
button is pressed again.

## SD Card Banks

The included bank files are in the repo's `sdCard/` directory. Copy the `CHORD*.TXT` files from
that directory to the root of the Chord Organ SD card:

```text
sdCard/CHORD00.TXT -> SD card root / CHORD00.TXT
sdCard/CHORD01.TXT -> SD card root / CHORD01.TXT
sdCard/CHORD02.TXT -> SD card root / CHORD02.TXT
```

Files are loaded in filename order. The firmware uses the number of `CHORD*.TXT` files actually on
the card, up to 16 banks, so 4 files gives 4 wide knob zones and 16 files gives 16 zones.

Each bank should contain numbered chord rows:

```text
# Bank: House1
01 [3,-2,-5,-12] Cm7
02 [4,-1,-4,-11] C#m7
...
12 [2,-3,-7,-13] Bm7b5
```

Comments are allowed. The bank name can live in a comment such as `# Bank: House1`.

## Included J6 Banks

The `sdCard/` directory contains these 20 J6-style banks. Each file has 12 chords and no custom
stack, wave, slide, or other per-bank settings.

```text
CHORD00.TXT  Oct Stack1
CHORD01.TXT  5th Stack1
CHORD02.TXT  Cinematic1
CHORD03.TXT  Cinematic2
CHORD04.TXT  EDM1
CHORD05.TXT  EDM2
CHORD06.TXT  EDM3
CHORD07.TXT  EDM4
CHORD08.TXT  House1
CHORD09.TXT  House2
CHORD10.TXT  House3
CHORD11.TXT  House4
CHORD12.TXT  Synthwave1
CHORD13.TXT  Synthwave2
CHORD14.TXT  Synthwave3
CHORD15.TXT  Synthwave4
CHORD16.TXT  Techno1
CHORD17.TXT  Trad Maj1
CHORD18.TXT  Trad Min1
CHORD19.TXT  Trance1
```

## Oxi One Setup

Best tested setup:

```text
Oxi velocity lane -> Chord CV input
Velocity mode     -> Not Gated
Velocity 75       -> chord 1
Velocity 76       -> chord 2
...
Velocity 86       -> chord 12
```

This gives one velocity step per chord, which is fast and easy to program.

Root/pitch stays separate:

```text
Oxi pitch CV -> Root CV input
Root knob    -> fine tuning around the CV pitch
```

## LED Bank Display

When the chord knob changes bank, the LEDs switch from waveform display to bank display. Bank
display is zero-indexed, so the first loaded bank is bank 0.

Read the LEDs from left to right:

```text
(O) = LED off
(X) = LED on
```

Full bank chart:

```text
bank 0   (O)(O)(O)(O)
bank 1   (X)(O)(O)(O)
bank 2   (O)(X)(O)(O)
bank 3   (X)(X)(O)(O)
bank 4   (O)(O)(X)(O)
bank 5   (X)(O)(X)(O)
bank 6   (O)(X)(X)(O)
bank 7   (X)(X)(X)(O)
bank 8   (O)(O)(O)(X)
bank 9   (X)(O)(O)(X)
bank 10  (O)(X)(O)(X)
bank 11  (X)(X)(O)(X)
bank 12  (O)(O)(X)(X)
bank 13  (X)(O)(X)(X)
bank 14  (O)(X)(X)(X)
bank 15  (X)(X)(X)(X)
```

## Current Calibration

The keeper chord CV calibration from hardware testing:

```cpp
#define CHORD_CV_SLOT_COUNT 12
#define CHORD_CV_VALUE_RANGE 88
#define CHORD_CV_INPUT_OFFSET -7120
#define CHORD_CV_VALUE_BASE 1
```

Pitch/root calibration:

```cpp
#define ROOT_CV_VALUE_RANGE 40
#define ROOT_CV_BASE_OFFSET 24
#define ROOT_KNOB_OFFSET_RANGE 12
#define ROOT_KNOB_OFFSET_CENTER 6
```

The CV reads are direct and deliberately unsmoothed for snappy modular sequencing.

## Source Notes

The sketch lives in:

```text
Chord-Organ/Chord-Organ.ino
```

Build target:

```text
teensy:avr:teensy31
```

This firmware is based on the Chord Organ / Radio Music hardware family by Tom Whitwell and the
ChordBanks work from starmandeluxe.

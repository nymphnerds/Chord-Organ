# Chord CV Mapping Note

Working idea for the J6 chord banks:

- Keep each J6 bank at 12 chords.
- Keep `!RANGE 39` as the calibration sweet spot.
- Treat the Chord CV as a calibrated semitone value.
- Select chords in 3-semitone zones: `12 chords * 3 semitones = 36 semitones`.
- Ignore/clamp the top 3 semitones of the 39-semitone range.

Suggested mapping:

```cpp
semitone = min(semitone, 35);
chordQuant = semitone / 3;
```

Result:

```text
0-2   -> chord 1
3-5   -> chord 2
6-8   -> chord 3
9-11  -> chord 4
12-14 -> chord 5
15-17 -> chord 6
18-20 -> chord 7
21-23 -> chord 8
24-26 -> chord 9
27-29 -> chord 10
30-32 -> chord 11
33-35 -> chord 12
36-38 -> held at chord 12 / ignored top range
```

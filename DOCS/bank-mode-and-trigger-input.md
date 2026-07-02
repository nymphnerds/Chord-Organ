# Bank Mode and Trigger Input Note

Working UI idea:

- Normal mode keeps the stock-feeling waveform display.
- Long press enters sticky bank mode.
- Entering bank mode gives a snappy double flash:
  - all LEDs on 80ms
  - all LEDs off 80ms
  - all LEDs on 80ms
  - all LEDs off 80ms
  - total: 320ms / 0.32 seconds
- Bank mode stays active until another long press.
- In bank mode, LEDs show the current bank number, preferably 1-16 in binary so bank 1 is not all LEDs off.
- Short press in bank mode steps to the next bank.
- Long press exits bank mode with a short confirmation flash and returns LEDs to waveform display.

External bank-change input idea:

- Repurpose the current TRIG OUT / RESET_CV jack as a bank trigger input.
- This sacrifices trigger-out behavior.
- Rising edge on the jack advances to the next bank.
- Manual sticky bank mode remains available.

Suggested behavior:

```text
Normal mode:
  short press -> next waveform
  long press  -> enter bank mode
  trigger in  -> next bank, briefly show bank, then return to waveform LEDs

Bank mode:
  LEDs show current bank
  short press -> next bank
  trigger in  -> next bank
  long press  -> exit bank mode
```

Implementation direction:

- Treat `RESET_CV` as an input, not a pulse output.
- Keep edge detection with `resetCV.rose()`.
- On each rising edge, call the same bank-advance function used by short press in bank mode.
- If trigger input fires while not in bank mode, briefly show the new bank number before restoring waveform LEDs.

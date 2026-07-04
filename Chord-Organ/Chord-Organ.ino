#include <Bounce2.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <EEPROM.h>

#include "Settings.h"
#include "Waves.h"

// #define DEBUG_STARTUP
// #define DEBUG_MODE
// #define CHECK_CPU

#define CHORD_POT_PIN 9 // pin for Chord pot
#define CHORD_CV_PIN 6 // pin for Chord CV
#define ROOT_POT_PIN 7 // pin for Root Note pot
#define ROOT_CV_PIN 8 // pin for Root Note CV
#define RESET_BUTTON 8 // Reset button 
#define RESET_LED 11 // Reset LED indicator 
#define RESET_CV 9 // Reset pulse in / out
#define BANK_BUTTON 2 // Bank Button 
#define LED0 6
#define LED1 5
#define LED2 4
#define LED3 3

// REBOOT CODES 
#define RESTART_ADDR       0xE000ED0C
#define READ_RESTART()     (*(volatile uint32_t *)RESTART_ADDR)
#define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))

#define ADC_BITS 13
#define ADC_MAX_VAL 8192
#define CHANGE_TOLERANCE 64

#define SINECOUNT 8
#define LOW_NOTE 36
#define CHORD_CV_SLOT_COUNT 12
#define CHORD_CV_VALUE_RANGE 88
#define CHORD_CV_INPUT_OFFSET -7120
#define CHORD_CV_VALUE_BASE 1
#define MAX_ACTIVE_BANKS 16
#define BANK_POT_MIN 0
#define BANK_POT_MAX (ADC_MAX_VAL - 1)
#define ROOT_KNOB_OFFSET_RANGE 12
#define ROOT_KNOB_OFFSET_CENTER 6
#define ROOT_CV_BASE_OFFSET 24
#define ROOT_CV_VALUE_RANGE 40

// For arbitrary waveform, required but unused apparently.
#define MAX_FREQ 600

#define SHORT_PRESS_DURATION 10
#define EEPROM_WAVEFORM_ADDRESS 1234

const char* settingsFileName = "CHORDORG.TXT";

//initialize bank index (0-15 possible)
int currentBank = 0;

// Target frequency of each oscillator
float FREQ[SINECOUNT] = {
    55,110, 220, 440, 880,1760,3520,7040};

// Total distance between last note and new.
// NOT distance per time step.
float deltaFrequency[SINECOUNT] = {
    0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

// Keep track of current frequency of each oscillator
float currentFrequency[SINECOUNT]  = {
    55,110, 220, 440, 880,1760,3520,7040};

float AMP[SINECOUNT] = { 
    0.9, 0.9, 0.9, 0.9,0.9, 0.9, 0.9, 0.9};

// Volume for a single voice for each chord size
float AMP_PER_VOICE[SINECOUNT] = {
  0.4,0.3,0.22,0.2,0.15,0.15,0.13,0.12};

// Store midi note number to frequency in a table
// Later can replace the table for custom tunings / scala support.
float MIDI_TO_FREQ[128];

int chordQuant;
int chordQuantOld;

int rootPotOld;
int rootCVOld;

int rootQuant;
int rootQuantOld;
int rootCVQuantOld = LOW_NOTE + ROOT_CV_BASE_OFFSET;
float rootFineTuneMultiplier = 1.0;
float rootFineTuneOld = 999.0;

float rootMapCoeff;

// Root CV Pin readings below this level are clamped to LOW_NOTE
int rootClampLow;

// Flag for either chord or root note change
boolean changed = true;
boolean rootChanged = false;

Bounce resetCV = Bounce( RESET_CV, 40 ); 
boolean resetButton = false;
int buttonState;
boolean resetCVRose;

elapsedMillis resetFlash; 

elapsedMillis buttonTimer = 0;
elapsedMillis lockOut = 0;
boolean shortPress = false;
boolean longPress = false;
boolean prevBankButton = false;
elapsedMillis pulseOutTimer = 0;
uint32_t flashTime = 10;
boolean flashing = false;

// WAVEFORM
// Default wave types
short wave_type[4] = {
    WAVEFORM_SINE,
    WAVEFORM_SQUARE,
    WAVEFORM_SAWTOOTH,
    WAVEFORM_PULSE,
};
// Current waveform index
int waveform = 0; 

// Waveform LED
boolean flashingWave = false;
elapsedMillis waveformIndicatorTimer = 0;
boolean showingBank = false;

int waveformPage = 0;
int waveformPages = 1;

// Custom wavetables
int16_t const* waveTables[8] {
    wave1,
    wave7,
    wave3,
    wave4,    

    wave8,
    wave9,
    wave10,
    wave11
};

// Per-waveform amp level
// First 4 are default waves, last 8 are custom wavetables
float WAVEFORM_AMP[12] = {
  0.8,0.6,0.8,0.6,
  0.8,0.8,0.8,0.8,
  0.8,0.8,0.8,0.8,
};

// GLIDE
// Main flag for glide on / off
boolean glide = false;
// msecs glide time. 
uint32_t glideTime = 50;
// keep reciprocal
float oneOverGlideTime = 0.02;
// Time since glide started
elapsedMillis glideTimer = 0;
// Are we currently gliding notes
boolean gliding = false;

// Stack mode replicates first 4 voices into last 4 with tuning offset
boolean stacked = false;
float stackFreqScale = 1.001;

int noteRange = 40;

// GUItool: begin automatically generated code

AudioSynthWaveform       waveform1;      //xy=215,232
AudioSynthWaveform       waveform2;      //xy=243,295
AudioSynthWaveform       waveform3;      //xy=273,354
AudioSynthWaveform       waveform4;      //xy=292,394
AudioSynthWaveform       waveform5;      //xy=215,232
AudioSynthWaveform       waveform6;      //xy=243,295
AudioSynthWaveform       waveform7;      //xy=273,354
AudioSynthWaveform       waveform8;      //xy=292,394
AudioMixer4              mixer1;         //xy=424,117
AudioMixer4              mixer2;         //xy=424,181
AudioMixer4              mixer3;         //xy=571,84
AudioEffectEnvelope      envelope1;      //xy=652,281
AudioOutputAnalog        dac1;           //xy=784,129
AudioConnection          patchCord1(waveform1, 0, mixer1, 0);
AudioConnection          patchCord2(waveform2, 0, mixer1, 1);
AudioConnection          patchCord7(waveform3, 0, mixer1, 2);
AudioConnection          patchCord8(waveform4, 0, mixer1, 3);
AudioConnection          patchCord3(waveform5, 0, mixer2, 0);
AudioConnection          patchCord4(waveform6, 0, mixer2, 1);
AudioConnection          patchCord5(waveform7, 0, mixer2, 2);
AudioConnection          patchCord6(waveform8, 0, mixer2, 3);
AudioConnection          patchCord9(mixer1, 0, mixer3, 0);
AudioConnection          patchCord10(mixer2, 0, mixer3, 1);
AudioConnection          patchCord11(mixer3, envelope1);
AudioConnection          patchCord12(envelope1, dac1);
// GUItool: end automatically generated code
// Pointers to waveforms
AudioSynthWaveform* oscillator[8];

Settings settings(settingsFileName);

void setup() {
    pinMode(BANK_BUTTON,INPUT);
    pinMode(RESET_BUTTON, INPUT);
    pinMode(RESET_CV, INPUT); 
    pinMode(RESET_LED, OUTPUT);
    pinMode(LED0,OUTPUT);
    pinMode(LED1,OUTPUT);
    pinMode(LED2,OUTPUT);
    pinMode(LED3,OUTPUT);
    AudioMemory(50);
    analogReadRes(ADC_BITS);
    
    oscillator[0] = &waveform1;
    oscillator[1] = &waveform2;
    oscillator[2] = &waveform3;
    oscillator[3] = &waveform4;
    oscillator[4] = &waveform5;
    oscillator[5] = &waveform6;
    oscillator[6] = &waveform7;
    oscillator[7] = &waveform8;

    for (int i = 0; i < 128; i++) {
        MIDI_TO_FREQ[i] = numToFreq(i);
    }

#ifdef DEBUG_STARTUP
  while( !Serial );

    Serial.println("Starting");
#endif // DEBUG_STARTUP

    // SD CARD SETTINGS FOR MODULE 
    SPI.setMOSI(7);
    SPI.setSCK(14);

    // Read waveform settings from EEPROM 
    waveform = EEPROM.read(EEPROM_WAVEFORM_ADDRESS);

#ifdef DEBUG_STARTUP
    Serial.print("Waveform from EEPROM ");
    Serial.println(waveform);
#endif

    if (waveform < 0) waveform = 0;
    
    // OPEN SD CARD 
    boolean hasSD = openSDCard();

#ifdef DEBUG_STARTUP
    Serial.print("Has SD ");
    Serial.println(hasSD);
#endif    
    // READ SETTINGS FROM SD CARD 
    settings.init(hasSD);
    waveformPages = settings.extraWaves ? 3 : 1;
    if (waveformPages > 1) {
        waveformPage = waveform >> 2;
    } else {
        // If we read a custom waveform index from EEPROM
        // but they are not enabled in the config then change back to sine
        waveform = 0;
    }

    int bankCount = getActiveBankCount();
    currentBank = bankFromPot(analogRead(CHORD_POT_PIN), bankCount);
    showingBank = false;
    ledWrite(waveform % 4);

    glide = settings.glide;
    glideTime = settings.glideTime;
    oneOverGlideTime = 1.0 / (float) glideTime;
    noteRange = settings.noteRange;
    stacked = settings.stacked;

#ifdef DEBUG_STARTUP
    Serial.print("Waveform page ");
    Serial.println(waveformPage);
    Serial.print("Waveform set to ");
    Serial.println(waveform);

    Serial.println("-- Settings --");
    Serial.print("Waveform Pages ");
    Serial.println(waveformPages);
    Serial.print("Glide ");
    Serial.println(glide);
    Serial.print("Glide Time ");
    Serial.println(glideTime);
    Serial.print("Note Range ");
    Serial.println(noteRange);
    Serial.print("Stacked ");
    Serial.println(stacked);

#endif

    // Setup audio
    for (int i = 0; i < SINECOUNT; i++) {
        oscillator[i]->pulseWidth(0.5);
    }

    for (int m = 0; m < 4; m++) {
        mixer1.gain(m,0.25);
        mixer2.gain(m,0.25);
    }

    mixer3.gain(0,0.49);
    mixer3.gain(1,0.49);
    mixer3.gain(2,0);
    mixer3.gain(3,0);

    envelope1.attack(1);
    envelope1.decay(1);
    envelope1.sustain(1.0);
    envelope1.release(1);
    envelope1.noteOn();

    if (waveformPage == 0) {
        // First page is built in waveforms
        setWaveformType(wave_type[waveform]);
    } else {
        // Second and third pages are arbitrary waves
        setupCustomWaveform(waveform);
        // Start the wave led flashing
        flashingWave = true;
        waveformIndicatorTimer = 0;
    }
    
    // This makes the CV input range for the low note half the size of the other notes.
    rootClampLow = ((float)ADC_MAX_VAL / noteRange) * 0.5;
    // Now map the rest of the range linearly across the input range
    rootMapCoeff = (float)noteRange / (ADC_MAX_VAL - rootClampLow);

#ifdef DEBUG_STARTUP
    Serial.print("Root Clamp Low ");
    Serial.println(rootClampLow);
    Serial.print("Root Map Coeff ");
    Serial.println(rootMapCoeff * 100);
#endif

}

boolean openSDCard() {
    int crashCountdown = 0; 
    if (!(SD.begin(10))) {
        while (!(SD.begin(10))) {
            ledWrite(15);
            delay(20);
            ledWrite(crashCountdown % 4);
            delay(20);
            crashCountdown++;
            if (crashCountdown > 4) {
                return false;
            }
        }
    }
    return true;
}

void loop() {

    checkInterface();

    if (changed) {
        
        // Serial.println("Changed");
        updateAmpAndFreq();
        if (glide) {
            glideTimer = 0;
            gliding = true;
            // Serial.println("Start glide");
        }

        #ifdef CHECK_CPU
        int maxCPU = AudioProcessorUsageMax();
        Serial.print("MaxCPU=");
        Serial.println(maxCPU);
        #endif // CHECK_CPU
    }

    if (shortPress) {
        waveform++;
        waveform = waveform % (4 * waveformPages);
        selectWaveform(waveform);
        changed = true;
        shortPress = false;
    }

    if (changed) {
        // Serial.println("Trig Out");
        pulseOutTimer = 0;
        flashing = true;
        pinMode(RESET_CV, OUTPUT);
        digitalWrite (RESET_LED, HIGH);
        digitalWrite (RESET_CV, HIGH);

        AudioNoInterrupts();
        updateFrequencies();
        updateAmps();
        AudioInterrupts();

        changed = false;
    }

    if (gliding) {
        if (glideTimer >= glideTime) {
            gliding = false;
        }
        AudioNoInterrupts();
        updateFrequencies();
        AudioInterrupts();
    }

    if (flashing && (pulseOutTimer > flashTime)) {
        digitalWrite (RESET_LED, LOW);
        digitalWrite (RESET_CV, LOW);
        pinMode(RESET_CV, INPUT);
        flashing = false;  
    } 

    updateStatusLEDs();
}

void updateAmpAndFreq() {
    int16_t* chord = settings.chordBanks[currentBank][chordQuant];

    #ifdef DEBUG_MODE
    Serial.print("Current Bank is: ");
    Serial.println(currentBank);
    Serial.print("Current Chord's first note is: ");
    Serial.println(chord[0]);
    #endif
    
    int noteNumber;
    int voiceCount = 0;
    int halfSinecount = SINECOUNT>>1;

    if (stacked) {
        for (int i = 0; i < halfSinecount; i++) {
            if (chord[i] != 255) {
                noteNumber = rootQuant + chord[i];
                if (noteNumber < 0) noteNumber = 0;
                if (noteNumber > 127) noteNumber = 127;
                float newFreq = MIDI_TO_FREQ[noteNumber] * rootFineTuneMultiplier;

                FREQ[i] = newFreq;
                FREQ[i+halfSinecount] = newFreq * stackFreqScale;
                // Serial.println("Stack Freq");
                // Serial.println(FREQ[i]);
                // Serial.println(FREQ[i+halfSinecount]);

                deltaFrequency[i] = newFreq - currentFrequency[i];
                deltaFrequency[i+halfSinecount] = (newFreq * stackFreqScale) - currentFrequency[i];

                voiceCount += 2;
            }            
        }
    } else {
        for (int i = 0; i < SINECOUNT; i++) {
            if (chord[i] != 255) {
                noteNumber = rootQuant + chord[i];
                if (noteNumber < 0) noteNumber = 0;
                if (noteNumber > 127) noteNumber = 127;
                float newFreq = MIDI_TO_FREQ[noteNumber] * rootFineTuneMultiplier;

                // TODO : Allow option to choose between jump from current or new?
                //deltaFrequency[i] = newFreq - FREQ[i];
                deltaFrequency[i] = newFreq - currentFrequency[i];

                // Serial.print("Delta ");
                // Serial.print(i);
                // Serial.print(" ");
                // Serial.print(deltaFrequency[i]);
                // Serial.print(" ");
                // Serial.println(newFreq);

                FREQ[i] = newFreq;
                voiceCount++;
            }
        }

    }

    float ampPerVoice = AMP_PER_VOICE[voiceCount-1];
    float totalAmp = 0;

    if (stacked) {
        for (int i = 0; i < halfSinecount; i++) {
            if (chord[i] != 255) {
                AMP[i] = ampPerVoice;
                AMP[i + halfSinecount] = ampPerVoice; 
                totalAmp += ampPerVoice;
            }
            else {
                AMP[i] = 0.0;   
            }
        }        
    } else {
        for (int i = 0; i < SINECOUNT; i++) {
            if (chord[i] != 255) {
                AMP[i] = ampPerVoice;
                totalAmp += ampPerVoice;
            }
            else {
                AMP[i] = 0.0;   
            }
        }        
    }
}

void selectWaveform(int waveform) {
    waveformPage = waveform >> 2;
    if (waveformPage > 0) {
        flashingWave = true;
        waveformIndicatorTimer = 0;
    }  
    EEPROM.write(EEPROM_WAVEFORM_ADDRESS, waveform);

    #ifdef DEBUG_MODE
    Serial.print("Waveform ");
    Serial.println(waveform);
    Serial.print("Waveform page ");
    Serial.println(waveformPage);
    #endif // DEBUG_MODE

    AudioNoInterrupts();
    if (waveformPage == 0) {
        setWaveformType(wave_type[waveform]);
    } else {
        setupCustomWaveform(waveform);    
    }
    AudioInterrupts();    
    showingBank = false;
    ledWrite(waveform % 4);
}

void setWaveformType(short waveformType) {
    for (int i = 0; i < SINECOUNT; i++) {
        oscillator[i]->begin(1.0,FREQ[i],waveformType);
    }   
}

void setupCustomWaveform(int waveselect) {
    waveselect = (waveselect - 4) % 8;

    const int16_t* wave = waveTables[waveselect];
    for (int i = 0; i < SINECOUNT; i++) {
        oscillator[i]->arbitraryWaveform(wave, MAX_FREQ);
    }

    setWaveformType(WAVEFORM_ARBITRARY);
}

void updateWaveformLEDs() {
    // Flash waveform LEDs for custom waves
    if (waveformPage > 0) {
        uint32_t blinkTime = 100 + ((waveformPage - 1) * 300);
        if (waveformIndicatorTimer >= blinkTime) {
            waveformIndicatorTimer = 0;
            flashingWave = !flashingWave;
            if (flashingWave) {
                ledWrite(waveform % 4);
            } else {
                ledWrite(15);
            }
        }
    }    
}

void showBankOnLEDs() {
    ledWriteBank(currentBank);
    showingBank = true;
}

void updateStatusLEDs() {
    if (showingBank) {
        return;
    }

    if (waveformPage > 0) {
        updateWaveformLEDs();
    } else {
        ledWrite(waveform % 4);
    }
}

void updateFrequencies() {

    if (gliding) {
        // TODO : Replace division with reciprocal multiply.
        float dt = 1.0 - (glideTimer * oneOverGlideTime);
        if (dt < 0.0) {
            dt = 0.0;
            gliding = false;
        }
        // Serial.print("dt ");
        // Serial.print(dt);
        // Serial.print(" ");
        // Serial.println(glideTimer);

        for (int i = 0; i < SINECOUNT; i++) {
            currentFrequency[i] = FREQ[i] - (deltaFrequency[i] * dt);
            oscillator[i]->frequency(currentFrequency[i]);
        }
    } else {
        for (int i = 0; i < SINECOUNT; i++) {
            oscillator[i]->frequency(FREQ[i]);
        }
    }
}

void updateAmps() {
    float waveAmp = WAVEFORM_AMP[waveform];
    mixer1.gain(0,AMP[0] * waveAmp);
    mixer1.gain(1,AMP[1] * waveAmp);
    mixer1.gain(2,AMP[2] * waveAmp);
    mixer1.gain(3,AMP[3] * waveAmp);
    mixer2.gain(0,AMP[4] * waveAmp);
    mixer2.gain(1,AMP[5] * waveAmp);
    mixer2.gain(2,AMP[6] * waveAmp);
    mixer2.gain(3,AMP[7] * waveAmp);
}

// WRITE A 4 DIGIT BINARY NUMBER TO LED0-LED3 
void ledWrite(int n) {
    digitalWrite(LED3, HIGH && (n==0));
    digitalWrite(LED2, HIGH && (n==1));
    digitalWrite(LED1, HIGH && (n==2));
    digitalWrite(LED0, HIGH && (n==3)); 
}

int getActiveBankCount() {
    int bankCount = settings.chordFileCount;
    if (bankCount < 1) bankCount = MAX_ACTIVE_BANKS;
    if (bankCount > MAX_ACTIVE_BANKS) bankCount = MAX_ACTIVE_BANKS;
    return bankCount;
}

// Show zero-index bank number in binary, with the lowest bit on the left LED.
void ledWriteBank(int n) {
  digitalWrite(LED3, HIGH && (n & B00000001));
  digitalWrite(LED2, HIGH && (n & B00000010));
  digitalWrite(LED1, HIGH && (n & B00000100));
  digitalWrite(LED0, HIGH && (n & B00001000));
}

int bankFromPot(int bankPot, int bankCount) {
    if (bankCount <= 1) return 0;

    bankPot = constrain(bankPot, BANK_POT_MIN, BANK_POT_MAX);
    long usableRange = (long)BANK_POT_MAX - BANK_POT_MIN + 1;
    int bank = ((long)(bankPot - BANK_POT_MIN) * bankCount) / usableRange;

    if (bank >= bankCount) bank = bankCount - 1;
    if (bank < 0) bank = 0;
    return bank;
}

int chordFromCV(int chordCV, int chordCount) {
    int selectableChordCount = min(chordCount, CHORD_CV_SLOT_COUNT);
    if (selectableChordCount <= 1) return 0;

    chordCV = constrain(chordCV, 0, ADC_MAX_VAL - 1);
    chordCV = constrain(chordCV + CHORD_CV_INPUT_OFFSET, 0, ADC_MAX_VAL - 1);
    int chord = map(chordCV, 0, ADC_MAX_VAL, 0, CHORD_CV_VALUE_RANGE);
    chord -= CHORD_CV_VALUE_BASE;
    if (chord >= CHORD_CV_VALUE_RANGE) chord = CHORD_CV_VALUE_RANGE - 1;
    if (chord >= selectableChordCount) chord = selectableChordCount - 1;
    if (chord < 0) chord = 0;
    return chord;
}

int readAnalogSettled(int pin) {
    return analogRead(pin);
}

void selectBank(int bank) {
    int bankCount = getActiveBankCount();
    if (bank >= bankCount) bank = bankCount - 1;
    if (bank < 0) bank = 0;
    if (bank == currentBank) return;

    currentBank = bank;

    int newBankChordCount = settings.numChords[currentBank];
    if (newBankChordCount < 1) newBankChordCount = 1;
    if (chordQuant >= newBankChordCount) {
        chordQuant = newBankChordCount - 1;
        chordQuantOld = chordQuant;
    }

    showBankOnLEDs();
    changed = true;
}

void checkInterface() {
    //get button state and save last state
    getAndSetButtonState();
    
    // Read pots + CVs
    int chordPot = readAnalogSettled(CHORD_POT_PIN);
    int chordCV = readAnalogSettled(CHORD_CV_PIN);
    int rootPot = readAnalogSettled(ROOT_POT_PIN);
    int rootCV = readAnalogSettled(ROOT_CV_PIN);

    chordPot = constrain(chordPot, 0, ADC_MAX_VAL - 1);
    chordCV = constrain(chordCV, 0, ADC_MAX_VAL - 1);
    rootPot = constrain(rootPot, 0, ADC_MAX_VAL - 1);
    rootCV = constrain(rootCV, 0, ADC_MAX_VAL - 1);

    rootChanged = false;
    int bankCount = getActiveBankCount();
    int rootCVValue = map(rootCV, 0, ADC_MAX_VAL, 0, ROOT_CV_VALUE_RANGE);
    if (rootCVValue >= ROOT_CV_VALUE_RANGE) rootCVValue = ROOT_CV_VALUE_RANGE - 1;

    int bankQuant = bankFromPot(chordPot, bankCount);
    if (bankQuant != currentBank) {
        selectBank(bankQuant);
    }

    int chordCount = settings.numChords[currentBank];
    if (chordCount < 1) chordCount = 1;
    chordQuant = chordFromCV(chordCV, chordCount);
    if (chordQuant != chordQuantOld) {
        changed = true;
        chordQuantOld = chordQuant;
    }

    int rootCVQuant = LOW_NOTE + ROOT_CV_BASE_OFFSET + rootCVValue;
    if (rootCVQuant != rootCVQuantOld) {
        rootChanged = true;
    }
    rootCVQuantOld = rootCVQuant;

    float rootFineTune = (((float)rootPot / (float)(ADC_MAX_VAL - 1)) * ROOT_KNOB_OFFSET_RANGE) - ROOT_KNOB_OFFSET_CENTER;
    if (abs(rootFineTune - rootFineTuneOld) > 0.01) {
        rootFineTuneMultiplier = pow(2.0, rootFineTune / 12.0);
        rootFineTuneOld = rootFineTune;
        changed = true;
    }

    rootQuant = rootCVQuant;
    if (rootQuant != rootQuantOld) {
        changed = true; 
        rootQuantOld = rootQuant;  
    }

#ifdef DEBUG_MODE
   if (rootChanged) {
        // printRootInfo(rootPot,rootCV);
   }
#endif

    //    resetSwitch.update();
    //    resetButton = resetSwitch.read();
    if (buttonTimer > SHORT_PRESS_DURATION && buttonState == 0 && lockOut > 999 ) {
        shortPress = true;
        //cancel long press
        longPress = false;
    }
    buttonTimer = buttonTimer * buttonState; 

    if (!flashing) {
        resetCV.update();
        resetCVRose = resetCV.rose();
        if (resetCVRose) resetFlash = 0; 

        digitalWrite(RESET_LED, (resetFlash<20));
    }
}

void getAndSetButtonState() {
    //get button state and save last state
    buttonState = digitalRead(RESET_BUTTON);
    if (buttonState == 1) {
      prevBankButton = true;
    }
    else {
      prevBankButton = false;
    }
}

void reBoot(int delayTime) {
    if (delayTime > 0)
        delay (delayTime);
    WRITE_RESTART(0x5FA0004);
}

void printRootInfo(int rootPot, int rootCV) {
    Serial.print("Root ");
    Serial.print(rootPot);
    Serial.print(" ");
    Serial.print(rootCV);
    Serial.print(" ");
    Serial.println(rootQuant);
}

void printPlaying() {
    Serial.print("Chord: ");
    Serial.print(chordQuant);
    Serial.print(" Root: ");
    Serial.print(rootQuant);
    Serial.print(" ");
    for (int i = 0; i < SINECOUNT; i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.print (FREQ[i]);
        Serial.print(" ");
        Serial.print(AMP[i]);
        Serial.print (" | ");
    }
    Serial.println("--");

}

float numToFreq(float input) {
    float number = input - 21; // set to midi note numbers = start with 21 at A0
    number = number - 48; // A0 is 48 steps below A4 = 440hz
    return 440*(pow (1.059463094359,number));
}

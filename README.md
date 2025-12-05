# ATmega328 Waveform Generator

A waveform generator for ATmega328 using Microchip Studio that generates three different signal types with button-controlled signal selection and potentiometer-controlled frequency adjustment.

## Features

- **Three Waveform Types**: Sine, Square, and Triangle waves
- **PWM-Based DAC**: 8-bit resolution with ~62.5kHz PWM frequency
- **Frequency Control**: 20Hz to 2000Hz via potentiometer
- **Button Controls**: 
  - SW1 (INT0): Wake-up from sleep mode
  - SW2 (INT1): Cycle through waveform types
- **Button Debouncing**: Software debounce for reliable button handling
- **Sleep Mode**: Power-down mode with interrupt wake-up
- **LED Indicator**: Visual indication of current waveform type

## Hardware Configuration

### Pin Assignments

| Pin | Function | Description |
|-----|----------|-------------|
| PD6 | OC0A | PWM output (8-bit DAC) |
| PD2 | INT0 | Wake-up button (SW1) |
| PD3 | INT1 | Signal select button (SW2) |
| PC0 | ADC0 | Frequency potentiometer (RV2) |
| PB5 | LED | Status indicator LED |

### Key Components

- **Microcontroller**: ATmega328P
- **Crystal**: 16MHz external crystal (Y1)
- **Output**: LM386 audio amplifier circuit
- **Power**: 5V regulated supply (L7805)

### Schematic Connections

```
                  ATmega328P
                 +----------+
       [SW1] --->| PD2 INT0 |---> Wake-up
       [SW2] --->| PD3 INT1 |---> Signal Select
                 |          |
        [RV2] -->| PC0 ADC0 |---> Frequency Control
                 |          |
                 | PD6 OC0A |---> PWM Output --> [LM386] --> Speaker
                 |          |
                 | PB5      |---> LED Indicator
                 +----------+
```

## Build Instructions

### Using Microchip Studio (Atmel Studio)

1. Open the solution file `SignalGenerator.atsln` in Microchip Studio
2. Set the target device to ATmega328P
3. Build the project (F7 or Build → Build Solution)
4. Program the device using your preferred programmer

### Using avr-gcc Command Line

```bash
# Compile
avr-gcc -mmcu=atmega328p -Os -DF_CPU=16000000UL -Wall -o SignalGenerator.elf main.c

# Generate HEX file
avr-objcopy -O ihex -R .eeprom SignalGenerator.elf SignalGenerator.hex

# Upload to device (using avrdude with USBasp programmer)
avrdude -c usbasp -p m328p -U flash:w:SignalGenerator.hex:i
```

### Using Make

```bash
cd SignalGenerator/Debug
make all
```

## Usage Guide

### Waveform Selection

Press **SW2** (PD3/INT1) to cycle through waveforms:
- **Sine wave** → 1 LED blink
- **Square wave** → 2 LED blinks
- **Triangle wave** → 3 LED blinks

### Frequency Adjustment

Turn the **RV2** potentiometer connected to PC0/ADC0:
- Counter-clockwise: Lower frequency (minimum 20Hz)
- Clockwise: Higher frequency (maximum 2000Hz)

### Sleep Mode

1. Press **SW1** (PD2/INT0) to enter power-down sleep mode
2. The LED will turn off and signal generation stops
3. Press **SW1** again to wake up
4. Upon wake-up, the LED will indicate the current waveform

### LED Indicator Patterns

| Waveform | LED Blinks |
|----------|------------|
| Sine     | 1          |
| Square   | 2          |
| Triangle | 3          |

## Technical Specifications

### Waveform Characteristics

| Waveform | Samples | Resolution | Description |
|----------|---------|------------|-------------|
| Sine     | 256     | 8-bit      | Smooth sinusoidal signal |
| Square   | 256     | 8-bit      | 50% duty cycle, sharp transitions |
| Triangle | 256     | 8-bit      | Linear ramp up and down |

### Frequency Range

- **Minimum**: 20 Hz
- **Maximum**: 2000 Hz
- **Resolution**: Continuous (ADC-based)

### PWM Configuration

- **Timer**: Timer0 in Fast PWM mode
- **PWM Frequency**: ~62.5 kHz (F_CPU / 256)
- **Resolution**: 8-bit (256 levels)
- **Output Pin**: PD6 (OC0A)

### Sample Rate Timing

- **Timer**: Timer2 in CTC mode
- **Sample Rate**: Frequency × 256 (TABLE_SIZE)
- **Example**: 1000 Hz output = 256,000 samples/second

### ADC Configuration

- **Reference**: AVcc (5V)
- **Resolution**: 10-bit (0-1023)
- **Clock**: 125 kHz (F_CPU / 128)
- **Channel**: ADC0 (PC0)

## File Structure

```
SignalGenerator/
├── SignalGenerator.atsln      # Microchip Studio solution file
├── SignalGenerator/
│   ├── main.c                 # Main application code
│   ├── config.h               # Configuration defines
│   ├── SignalGenerator.cproj  # Project configuration
│   └── Debug/
│       ├── Makefile           # Build makefile
│       ├── SignalGenerator.hex # Compiled HEX file
│       └── SignalGenerator.elf # Compiled ELF file
└── README.md                  # This file
```

## Configuration Options

Edit `config.h` to customize:

```c
/* Frequency range */
#define FREQ_MIN            20      /* Minimum frequency in Hz */
#define FREQ_MAX            2000    /* Maximum frequency in Hz */

/* Debounce timing */
#define DEBOUNCE_TIME_MS    50      /* Debounce time in milliseconds */
#define DEBOUNCE_SAMPLES    5       /* Number of stable samples */

/* LED timing */
#define LED_BLINK_ON_TIME   100     /* LED on duration (ms) */
#define LED_BLINK_OFF_TIME  100     /* LED off duration (ms) */
```

## Testing

### Waveform Verification

1. Connect an oscilloscope to the PWM output (PD6)
2. Use a low-pass filter (RC filter) to smooth the PWM signal
3. Verify each waveform type displays correctly
4. Measure frequency at different potentiometer positions

### Button Testing

1. Press SW2 repeatedly and verify waveform changes
2. Verify LED blinks correspond to waveform type
3. Check that debouncing prevents multiple triggers

### Sleep Mode Testing

1. Press SW1 to enter sleep mode
2. Verify signal generation stops
3. Press SW1 to wake up
4. Verify signal generation resumes

### Suggested Test Points

| Test | Expected Result |
|------|-----------------|
| Initial power-up | Sine wave at ~1kHz, 1 LED blink |
| Press SW2 once | Square wave, 2 LED blinks |
| Press SW2 twice | Triangle wave, 3 LED blinks |
| Turn pot clockwise | Frequency increases toward 2kHz |
| Turn pot counter-clockwise | Frequency decreases toward 20Hz |
| Press SW1 | Device enters sleep, LED off |
| Press SW1 again | Device wakes, LED indicates waveform |

## Memory Usage

Typical memory usage (with -Os optimization):

| Section | Size | Description |
|---------|------|-------------|
| .text   | ~1.8 KB | Program code + lookup tables |
| .data   | 0 B | Initialized data |
| .bss    | ~7 B | Uninitialized data (variables) |

Total flash usage: ~1.8 KB of 32 KB available (5.6%)

## License

This project is provided as-is for educational purposes.

## Troubleshooting

### No output signal
- Check PWM output pin (PD6) connection
- Verify crystal is oscillating at 16MHz
- Check power supply is stable at 5V

### Button not responding
- Verify button connections to GND
- Check internal pull-ups are enabled
- Increase debounce time if needed

### Frequency not changing
- Verify potentiometer wiper is connected to PC0
- Check potentiometer end connections to VCC and GND
- Verify ADC is reading correctly

### Distorted waveform
- Add RC low-pass filter to smooth PWM output
- Recommended: 10kΩ resistor + 100nF capacitor
- Cutoff frequency: ~160Hz (adjust as needed)

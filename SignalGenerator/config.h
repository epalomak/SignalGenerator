/**
 * @file config.h
 * @brief Configuration defines for ATmega328 Waveform Generator
 * 
 * Hardware Configuration:
 * - MCU: ATmega328P with 16MHz external crystal
 * - PWM Output: PD6 (OC0A) - Timer0 Fast PWM
 * - Button SW1: PD2 (INT0) - Wake-up from sleep
 * - Button SW2: PD3 (INT1) - Signal type selector
 * - ADC Input: PC0 (ADC0) - Frequency potentiometer
 * - LED Indicator: PB5
 */

#ifndef CONFIG_H
#define CONFIG_H

/* Clock frequency - 16MHz external crystal */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* ========== Waveform Configuration ========== */
#define TABLE_SIZE          256     /* Samples per waveform cycle */
#define NUM_WAVEFORMS       3       /* Sine, Square, Triangle */

/* Waveform type indices */
#define WAVEFORM_SINE       0
#define WAVEFORM_SQUARE     1
#define WAVEFORM_TRIANGLE   2

/* ========== Frequency Configuration ========== */
#define FREQ_MIN            20      /* Minimum frequency in Hz */
#define FREQ_MAX            2000    /* Maximum frequency in Hz */

/* ========== Pin Configuration ========== */
/* PWM Output - Timer0 OC0A */
#define PWM_PORT            PORTD
#define PWM_DDR             DDRD
#define PWM_PIN             PD6     /* OC0A pin */

/* Button SW1 - Wake-up (INT0) */
#define BTN_WAKEUP_PORT     PORTD
#define BTN_WAKEUP_DDR      DDRD
#define BTN_WAKEUP_PINR     PIND
#define BTN_WAKEUP_PIN      PD2     /* INT0 */

/* Button SW2 - Signal Select (INT1) */
#define BTN_SELECT_PORT     PORTD
#define BTN_SELECT_DDR      DDRD
#define BTN_SELECT_PINR     PIND
#define BTN_SELECT_PIN      PD3     /* INT1 */

/* LED Indicator */
#define LED_PORT            PORTB
#define LED_DDR             DDRB
#define LED_PIN             PB5

/* ADC Input - Frequency Potentiometer */
#define ADC_CHANNEL         0       /* PC0/ADC0 */

/* ========== Debounce Configuration ========== */
#define DEBOUNCE_TIME_MS    50      /* Debounce time in milliseconds */
#define DEBOUNCE_SAMPLES    5       /* Number of stable samples required */

/* ========== Sleep Mode Configuration ========== */
#define SLEEP_ENABLE        1       /* Enable sleep mode functionality */

/* ========== LED Blink Patterns ========== */
/* Number of blinks to indicate waveform type */
#define LED_BLINK_SINE      1       /* 1 blink for sine */
#define LED_BLINK_SQUARE    2       /* 2 blinks for square */
#define LED_BLINK_TRIANGLE  3       /* 3 blinks for triangle */

/* LED timing in milliseconds */
#define LED_BLINK_ON_TIME   100     /* LED on duration */
#define LED_BLINK_OFF_TIME  100     /* LED off duration between blinks */
#define LED_BLINK_PAUSE     500     /* Pause after blink pattern */

/* ========== Timer Configuration ========== */
/* Timer0: PWM generation (Fast PWM mode) */
/* Timer2: Sample rate timing */

/* Timer2 prescaler options for sample rate */
#define TIMER2_PRESCALER_1      1
#define TIMER2_PRESCALER_8      8
#define TIMER2_PRESCALER_32     32
#define TIMER2_PRESCALER_64     64
#define TIMER2_PRESCALER_128    128
#define TIMER2_PRESCALER_256    256
#define TIMER2_PRESCALER_1024   1024

#endif /* CONFIG_H */

/**
 * @file main.c
 * @brief ATmega328 Waveform Generator
 * 
 * Generates sine, square, and triangle waveforms with:
 * - PWM-based DAC output on PD6 (OC0A)
 * - Button-controlled signal selection (SW2 on PD3/INT1)
 * - Wake-up from sleep (SW1 on PD2/INT0)
 * - Potentiometer-controlled frequency (PC0/ADC0)
 * - LED indicator for waveform type (PB5)
 * 
 * @author Signal Generator Project
 * @date 2024
 */

#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <util/delay.h>

/* ========== Waveform Lookup Tables (256 samples, 8-bit) ========== */

/* 8-bit sine table (0-255), 256 samples - stored in program memory */
const uint8_t sine_table[TABLE_SIZE] PROGMEM = {
	127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,
	176,179,182,184,187,190,193,195,198,200,203,205,208,210,213,215,
	217,219,221,224,226,228,229,231,233,235,236,238,239,241,242,244,
	245,246,247,248,249,250,251,251,252,253,253,254,254,254,254,254,
	255,254,254,254,254,254,253,253,252,251,251,250,249,248,247,246,
	245,244,242,241,239,238,236,235,233,231,229,228,226,224,221,219,
	217,215,213,210,208,205,203,200,198,195,193,190,187,184,182,179,
	176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,
	127,124,121,118,115,111,108,105,102,99,96,93,90,87,84,81,
	78,75,72,70,67,64,61,59,56,54,51,49,46,44,41,39,
	37,35,33,30,28,26,25,23,21,19,18,16,15,13,12,10,
	9,8,7,6,5,4,3,3,2,1,1,0,0,0,0,0,
	0,0,0,0,0,0,1,1,2,3,3,4,5,6,7,8,
	9,10,12,13,15,16,18,19,21,23,25,26,28,30,33,35,
	37,39,41,44,46,49,51,54,56,59,61,64,67,70,72,75,
	78,81,84,87,90,93,96,99,102,105,108,111,115,118,121,124
};

/* 8-bit square table (0 or 255), 256 samples - stored in program memory */
const uint8_t square_table[TABLE_SIZE] PROGMEM = {
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/* 8-bit triangle table (0-255), 256 samples - stored in program memory */
const uint8_t triangle_table[TABLE_SIZE] PROGMEM = {
	0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,
	32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,
	64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,
	96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,
	128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,
	160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,
	192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,
	224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,254,
	254,252,250,248,246,244,242,240,238,236,234,232,230,228,226,224,
	222,220,218,216,214,212,210,208,206,204,202,200,198,196,194,192,
	190,188,186,184,182,180,178,176,174,172,170,168,166,164,162,160,
	158,156,154,152,150,148,146,144,142,140,138,136,134,132,130,128,
	126,124,122,120,118,116,114,112,110,108,106,104,102,100,98,96,
	94,92,90,88,86,84,82,80,78,76,74,72,70,68,66,64,
	62,60,58,56,54,52,50,48,46,44,42,40,38,36,34,32,
	30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0
};

/* ========== Global Variables ========== */
volatile uint8_t waveform = WAVEFORM_SINE;   /* Current waveform type */
volatile uint8_t sample_index = 0;           /* Current sample index in table */
volatile uint8_t sleep_flag = 0;             /* Flag to enter sleep mode */
volatile uint8_t wakeup_flag = 0;            /* Flag indicating wake-up occurred */
volatile uint8_t waveform_changed = 0;       /* Flag for LED update */

/* Debounce variables */
volatile uint8_t btn_select_debounce = 0;    /* Debounce counter for select button */
volatile uint8_t btn_wakeup_debounce = 0;    /* Debounce counter for wake-up button */

/* ========== Function Prototypes ========== */
static void gpio_init(void);
static void pwm_init(void);
static void timer2_init(void);
static void adc_init(void);
static void interrupts_init(void);
static void set_sample_rate(uint16_t freq_hz);
static uint16_t adc_read(void);
static void led_indicate_waveform(void);
static void enter_sleep(void);

/* ========== PWM Output Functions ========== */

/**
 * @brief Write value to PWM output (acts as DAC)
 * @param value 8-bit value (0-255)
 */
static inline void pwm_write(uint8_t value)
{
	OCR0A = value;
}

/* ========== Initialization Functions ========== */

/**
 * @brief Initialize GPIO pins
 */
static void gpio_init(void)
{
	/* PWM output pin (PD6/OC0A) as output */
	PWM_DDR |= (1 << PWM_PIN);
	
	/* LED pin (PB5) as output */
	LED_DDR |= (1 << LED_PIN);
	LED_PORT &= ~(1 << LED_PIN);  /* LED off initially */
	
	/* Button SW1 (PD2/INT0) as input with pull-up */
	BTN_WAKEUP_DDR &= ~(1 << BTN_WAKEUP_PIN);
	BTN_WAKEUP_PORT |= (1 << BTN_WAKEUP_PIN);
	
	/* Button SW2 (PD3/INT1) as input with pull-up */
	BTN_SELECT_DDR &= ~(1 << BTN_SELECT_PIN);
	BTN_SELECT_PORT |= (1 << BTN_SELECT_PIN);
}

/**
 * @brief Initialize Timer0 for Fast PWM mode
 * 
 * Timer0 generates 8-bit PWM at ~62.5kHz (F_CPU/256)
 * This high frequency provides smooth analog-like output
 */
static void pwm_init(void)
{
	/* Fast PWM mode, non-inverting output on OC0A */
	TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
	
	/* No prescaler (full speed PWM) */
	TCCR0B = (1 << CS00);
	
	/* Initial PWM value (mid-point) */
	OCR0A = 127;
}

/**
 * @brief Initialize Timer2 for sample rate timing
 * 
 * Timer2 controls how fast we step through the waveform table.
 * Sample rate = F_CPU / (prescaler * (OCR2A + 1))
 */
static void timer2_init(void)
{
	/* CTC mode */
	TCCR2A = (1 << WGM21);
	
	/* Prescaler 8 for good resolution */
	TCCR2B = (1 << CS21);
	
	/* Default compare value (will be updated by set_sample_rate) */
	OCR2A = 77;  /* ~1kHz output frequency initially */
	
	/* Enable compare match interrupt */
	TIMSK2 = (1 << OCIE2A);
}

/**
 * @brief Initialize ADC for reading frequency potentiometer
 */
static void adc_init(void)
{
	/* Reference = AVcc, channel ADC0 (PC0) */
	ADMUX = (1 << REFS0) | ADC_CHANNEL;
	
	/* Enable ADC, prescaler 128 (125kHz ADC clock @ 16MHz) */
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

/**
 * @brief Initialize external interrupts for buttons
 */
static void interrupts_init(void)
{
	/* INT0 (SW1 wake-up): Falling edge trigger */
	EICRA |= (1 << ISC01);   /* ISC01=1, ISC00=0 -> falling edge */
	EICRA &= ~(1 << ISC00);
	
	/* INT1 (SW2 select): Falling edge trigger */
	EICRA |= (1 << ISC11);   /* ISC11=1, ISC10=0 -> falling edge */
	EICRA &= ~(1 << ISC10);
	
	/* Enable both external interrupts */
	EIMSK = (1 << INT0) | (1 << INT1);
}

/* ========== Utility Functions ========== */

/**
 * @brief Set the sample rate to achieve desired output frequency
 * @param freq_hz Desired output frequency in Hz
 * 
 * Sample rate = freq_hz * TABLE_SIZE
 * OCR2A = F_CPU / (prescaler * sample_rate) - 1
 */
static void set_sample_rate(uint16_t freq_hz)
{
	/* Clamp frequency to valid range */
	if (freq_hz < FREQ_MIN) freq_hz = FREQ_MIN;
	if (freq_hz > FREQ_MAX) freq_hz = FREQ_MAX;
	
	/* Calculate required sample rate */
	uint32_t sample_rate = (uint32_t)freq_hz * TABLE_SIZE;
	
	/* Calculate OCR2A value with prescaler 8 */
	uint32_t ocr_val = F_CPU / (8UL * sample_rate);
	
	if (ocr_val > 0) ocr_val--;
	if (ocr_val > 255) ocr_val = 255;
	if (ocr_val < 1) ocr_val = 1;
	
	/* Atomic update of OCR2A */
	uint8_t sreg = SREG;
	cli();
	OCR2A = (uint8_t)ocr_val;
	SREG = sreg;
}

/**
 * @brief Read ADC value (blocking)
 * @return 10-bit ADC value (0-1023)
 */
static uint16_t adc_read(void)
{
	/* Start conversion */
	ADCSRA |= (1 << ADSC);
	
	/* Wait for completion */
	while (ADCSRA & (1 << ADSC));
	
	return ADC;
}

/**
 * @brief Blink LED to indicate current waveform type
 */
static void led_indicate_waveform(void)
{
	uint8_t blinks = 0;
	
	switch (waveform) {
		case WAVEFORM_SINE:
			blinks = LED_BLINK_SINE;
			break;
		case WAVEFORM_SQUARE:
			blinks = LED_BLINK_SQUARE;
			break;
		case WAVEFORM_TRIANGLE:
			blinks = LED_BLINK_TRIANGLE;
			break;
		default:
			blinks = 1;
			break;
	}
	
	/* Blink LED the specified number of times */
	for (uint8_t i = 0; i < blinks; i++) {
		LED_PORT |= (1 << LED_PIN);   /* LED on */
		_delay_ms(LED_BLINK_ON_TIME);
		LED_PORT &= ~(1 << LED_PIN);  /* LED off */
		if (i < blinks - 1) {
			_delay_ms(LED_BLINK_OFF_TIME);
		}
	}
}

/**
 * @brief Enter power-down sleep mode
 */
static void enter_sleep(void)
{
	/* Turn off LED before sleep */
	LED_PORT &= ~(1 << LED_PIN);
	
	/* Stop Timer2 to stop waveform generation */
	TIMSK2 &= ~(1 << OCIE2A);
	
	/* Set PWM to mid-point (no signal) */
	pwm_write(127);
	
	/* Configure sleep mode (Power-down) */
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	
	/* Enter sleep - will wake on INT0 */
	sei();
	sleep_cpu();
	
	/* Woken up! */
	sleep_disable();
	
	/* Re-enable Timer2 interrupt */
	TIMSK2 |= (1 << OCIE2A);
	
	/* Reset sample index for clean waveform start */
	sample_index = 0;
	
	/* Indicate wake-up with LED */
	waveform_changed = 1;
}

/* ========== Interrupt Service Routines ========== */

/**
 * @brief Timer2 Compare Match A ISR - generates waveform samples
 */
ISR(TIMER2_COMPA_vect)
{
	uint8_t value;
	
	/* Get sample from appropriate waveform table */
	switch (waveform) {
		case WAVEFORM_SINE:
			value = pgm_read_byte(&sine_table[sample_index]);
			break;
		case WAVEFORM_SQUARE:
			value = pgm_read_byte(&square_table[sample_index]);
			break;
		case WAVEFORM_TRIANGLE:
			value = pgm_read_byte(&triangle_table[sample_index]);
			break;
		default:
			value = pgm_read_byte(&sine_table[sample_index]);
			break;
	}
	
	/* Output sample via PWM */
	pwm_write(value);
	
	/* Advance to next sample */
	sample_index++;
	/* No need to check for overflow - 8-bit wraps automatically with 256 samples */
}

/**
 * @brief INT0 ISR - Wake-up button (SW1)
 */
ISR(INT0_vect)
{
	/* Simple debounce: ignore if recently triggered */
	if (btn_wakeup_debounce == 0) {
		btn_wakeup_debounce = DEBOUNCE_SAMPLES;
		wakeup_flag = 1;
	}
}

/**
 * @brief INT1 ISR - Signal select button (SW2)
 */
ISR(INT1_vect)
{
	/* Simple debounce: ignore if recently triggered */
	if (btn_select_debounce == 0) {
		btn_select_debounce = DEBOUNCE_SAMPLES;
		
		/* Cycle through waveforms: sine -> square -> triangle -> sine */
		waveform++;
		if (waveform >= NUM_WAVEFORMS) {
			waveform = WAVEFORM_SINE;
		}
		
		/* Reset sample index for clean waveform transition */
		sample_index = 0;
		
		/* Set flag to update LED indicator */
		waveform_changed = 1;
	}
}

/* ========== Main Function ========== */

int main(void)
{
	/* Initialize all peripherals */
	gpio_init();
	pwm_init();
	timer2_init();
	adc_init();
	interrupts_init();
	
	/* Set initial frequency (1kHz) */
	set_sample_rate(1000);
	
	/* Enable global interrupts */
	sei();
	
	/* Indicate initial waveform (sine) */
	led_indicate_waveform();
	
	/* Main loop */
	while (1) {
		/* Read potentiometer and update frequency */
		uint16_t adc_val = adc_read();
		
		/* Map ADC value (0-1023) to frequency range (FREQ_MIN-FREQ_MAX) */
		uint32_t freq = FREQ_MIN + ((uint32_t)adc_val * (FREQ_MAX - FREQ_MIN) / 1023);
		set_sample_rate((uint16_t)freq);
		
		/* Handle debounce counters (decrement each loop iteration) */
		if (btn_select_debounce > 0) {
			btn_select_debounce--;
			_delay_ms(10);
		}
		if (btn_wakeup_debounce > 0) {
			btn_wakeup_debounce--;
			_delay_ms(10);
		}
		
		/* Update LED if waveform changed */
		if (waveform_changed) {
			waveform_changed = 0;
			led_indicate_waveform();
		}
		
		/* Check if sleep requested (SW1 pressed while awake) */
		if (sleep_flag) {
			sleep_flag = 0;
			enter_sleep();
		}
		
		/* Small delay to prevent excessive ADC readings */
		_delay_ms(10);
	}
	
	return 0;
}

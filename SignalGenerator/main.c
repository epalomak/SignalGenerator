#define F_CPU 8000000UL   // 8 MHz

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define TABLE_SIZE 64
#define DAC_MASK   0x3F    // PB0..PB5

// 6-bit sini-taulukko (0..63), 64 näytettä
const uint8_t sine_table[TABLE_SIZE] = {
	32,35,38,41,44,46,49,51,
	54,56,58,59,61,62,62,63,
	63,63,62,62,61,59,58,56,
	54,51,49,46,44,41,38,35,
	32,28,25,22,19,17,14,12,
	9,7,5,4,2,1,1,0,
	0,0,1,1,2,4,5,7,
	9,12,14,17,19,22,25,28
};

// Neliö: 32 näytettä ylhäällä, 32 alhaalla
const uint8_t square_table[TABLE_SIZE] = {
	[0 ... 31] = 63,
	[32 ... 63] = 0
};

// Saha: nouseva 0..63
const uint8_t saw_table[TABLE_SIZE] = {
	0,1,2,3,4,5,6,7,
	8,9,10,11,12,13,14,15,
	16,17,18,19,20,21,22,23,
	24,25,26,27,28,29,30,31,
	32,33,34,35,36,37,38,39,
	40,41,42,43,44,45,46,47,
	48,49,50,51,52,53,54,55,
	56,57,58,59,60,61,62,63
};

volatile uint8_t waveform = 0;      // 0 = sini, 1 = neliö, 2 = saha
volatile uint8_t sample_index = 0;


// --- Pieni apufunktio DAC:lle (PB0..PB5) ---

static inline void dac_write(uint8_t value)
{
	PORTB = (PORTB & ~DAC_MASK) | (value & DAC_MASK);
}


// --- Timer1: taajuuden säätö ---

void timer1_set_frequency(uint16_t freq_hz)
{
	if (freq_hz < 60)     freq_hz = 60;
	if (freq_hz > 15000)  freq_hz = 15000;

	// näytetaajuus = f_out * TABLE_SIZE
	// OCR1A = F_CPU / (prescaler * f_out * TABLE_SIZE) - 1
	const uint16_t prescaler = 1;   // CS10 = 1
	uint32_t ticks = (uint32_t)F_CPU / ((uint32_t)prescaler * freq_hz * TABLE_SIZE);

	if (ticks < 1)      ticks = 1;
	if (ticks > 65535)  ticks = 65535;

	uint16_t ocr = (uint16_t)(ticks - 1);

	uint8_t sreg = SREG;
	cli();
	OCR1A = ocr;
	SREG = sreg;
	sei();
}


// --- ADC (RV2 potikka, ADC2 / PC2) ---

void adc_init(void)
{
	// Referenssi = AVcc, kanava ADC2 (PC2)
	ADMUX = (1 << REFS0) | 2;

	// ADC päälle, jakaja 128 (125 kHz ADC clock @ 16 MHz)
	ADCSRA = (1 << ADEN) |
	(1 << ADPS2) | (1 << ADPS1);
	ADCSRA &= ~(1 << ADPS0);
}

uint16_t adc_read(void)
{
	ADCSRA |= (1 << ADSC);            // start
	while (ADCSRA & (1 << ADSC)) {}   // odota
	return ADC;
}


// --- Keskeytykset ---

// Timer1: uusi DAC-näyte
ISR(TIMER1_COMPA_vect)
{
	uint8_t value;

	switch (waveform) {
		case 0:
		value = square_table[sample_index];
		break;
		case 1:
		value = square_table[sample_index];
		break;
		default:
		value = saw_table[sample_index];
		break;
	}

	dac_write(value);

	sample_index++;
	if (sample_index >= TABLE_SIZE) {
		sample_index = 0;
	}
}

// INT0: nappi vaihtaa aaltomuodon
ISR(INT1_vect)
{
	waveform++;
	if (waveform > 2) {
		waveform = 0;
	}
	sample_index = 0; // halutessa voi nollata
}


// --- main ---

int main(void)
{
	// PB0..PB5 DAC-lähdöiksi
	DDRB |= DAC_MASK;

	// Nappi SW1: PD2 / INT1, sisäinen pull-up
	MCUCR &= ~(1 << PUD);
	DDRD &= ~(1 << DDD3);
	PORTD |= (1 << PORTD3);

	// Timer1 CTC, prescaler 1
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS10);
	timer1_set_frequency(1000);    // aloitustaajuus 1 kHz
	TIMSK1 = (1 << OCIE1A);        // OCR1A keskeytys päälle

	// INT0 laskeva reuna
	EICRA = (1 << ISC11);          // ISC01=1, ISC00=0 → falling edge
	EIMSK = (1 << INT1);

	adc_init();

	while (1) {
		// luetaan potikka ja säädetään taajuus
		uint16_t adc_val = adc_read();   // 0..1023

		// mapataan 0..1023 → 60..15000 Hz
		uint32_t freq = 60 + ((uint32_t)adc_val * (15000 - 60) / 1023);

		timer1_set_frequency((uint16_t)freq);

		// ADC:n ja loopin kesto on jo pieni "viive",
		// erillistä _delay_ms() ei tässä tarvita.
	}
}

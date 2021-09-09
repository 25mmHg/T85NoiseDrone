/*
 * T85NoiseDrone.c
 *
 * Created: 02.09.2021 13:51:47
 * Author : 25mmHg
 * Fuses: lfuse=c2, hfuse=DF, efuse=FF
 *
 *  VCC o-----o--------------o--------.
 *            |              |        |
 *            |              |        |
 *            |              |        |
 *            |              |        |
 *            |              |        |  ATtiny85
 *           ---             | 1 __ 8 |
 *           ---             '-o|  |o-'
 *        100n|         OUT3---o|  |o---OUT2
 *            |         OUT4---o|  |o---------------o---MISO
 *            |              .-o|__|o---OUT0        |
 *            |              |                      |
 *            |              |                      |
 *            |              |                      |   ____
 *            |              |                      `---°  °---.
 *            |              |                                 |
 *            |              |                                 |
 *           ---            ---                               ---
 *
 * (created by AACircuit v1.28.7 beta 10/23/16 www.tech-chat.de)
 *
 */ 

#define F_CPU 8000000UL
#define OUT3 PB3
#define OUT4 PB4
#define OUT2 PB2
#define OUT0 PB0
#define BUTTON PB1
#define BUTTON_RELEASED (MYPIN & (1<<BUTTON))
#define BUTTON_PRESSED   !BUTTON_RELEASED
#define MYDDR DDRB
#define MYPORT PORTB
#define MYPIN PINB
#define DOWN 0U
#define UP 1U

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	MYDDR  |=  ((1<<OUT3) | (1<<OUT4) | (1<<OUT2) | (1<<OUT0));
	MYPORT |=  ((1<<OUT3) | (1<<OUT4) | (1<<OUT2) | (1<<OUT0));
	MYPORT |=  (1<<BUTTON);            // set pullup
	static uint32_t lfsrA = 203822843; // Startwert für A
	static uint32_t lfsrB = 238279927; // Wert 360 Schritte vor A (B hat  hier 4,1ms Vorsprung vor A)
	static uint32_t lfsrC = 210161983; // Wert 540 Schritte vor A...
	static uint32_t lfsrD = 126041972; // Wert 809 Schritte vor A...
	const uint32_t lfsr_tapsA  = 0xE100000;
	const uint32_t lfsr_tapsB  = 0xE100000;
	const uint32_t lfsr_tapsC  = 0xE100000;
	const uint32_t lfsr_tapsD  = 0xE100000;
	int16_t lfsr_diff = 540;
	uint8_t lfsr_dir = DOWN;

	// Rauschgenerator mit 4LFSR
	while(1) 
    {		
		if      (lfsr_diff >=  809) lfsr_dir = DOWN;
		else if (lfsr_diff <= -180) lfsr_dir = UP;
		else;
		static uint8_t rememberTheButton = 0;
		
		// Wenn Taste nicht gedrückt 4freilaufende LFSR mit gleichbleibender Distanz zueinander
		while(BUTTON_RELEASED)
		{
			// gleiche Looplänge wie Loop mit gedrücktem Taster
			_delay_us(2);
			// Umkehrung der Verschiebungrichtung der Folgen nach Tastendruck
			if (rememberTheButton==1)
			{
				rememberTheButton = 0;
				lfsr_dir = !lfsr_dir;
			}
			// 4LFSR mit bis zu 32Bit Länge nach E.Galois
			lfsrA=(lfsrA >> 1)^(-(lfsrA & 1) & lfsr_tapsA);
			lfsrB=(lfsrB >> 1)^(-(lfsrB & 1) & lfsr_tapsB);
			lfsrC=(lfsrC >> 1)^(-(lfsrC & 1) & lfsr_tapsC);
			lfsrD=(lfsrD >> 1)^(-(lfsrD & 1) & lfsr_tapsD);
			// PowerMixer
			MYPIN |= ((lfsrA & 1) << OUT3) | ((lfsrB & 1) << OUT4) | ((lfsrC & 1) << OUT2) | ((lfsrD & 1) << OUT0);
		}
		// Nur wenn Taste gedrückt aller 2000 Schritte Abstand der Folgen A<->B, B<->C, C<->D variieren
		for(uint16_t k=0; k<2000 && BUTTON_PRESSED; k++)
		{
			rememberTheButton = 1;
			if(k!=1)
			{
				lfsrA=(lfsrA >> 1)^(-(lfsrA & 1) & lfsr_tapsA);
				lfsrB=(lfsrB >> 1)^(-(lfsrB & 1) & lfsr_tapsB);
				lfsrC=(lfsrC >> 1)^(-(lfsrC & 1) & lfsr_tapsC);
				lfsrD=(lfsrD >> 1)^(-(lfsrD & 1) & lfsr_tapsD);
			}
			else
			{
				if(lfsr_dir == DOWN)
				{
					lfsr_diff--;
					//lfsrA 3Schritte
					lfsrA=(lfsrA >> 1)^(-(lfsrA & 1) & lfsr_tapsA);
					lfsrA=(lfsrA >> 1)^(-(lfsrA & 1) & lfsr_tapsA);
					lfsrA=(lfsrA >> 1)^(-(lfsrA & 1) & lfsr_tapsA);
					//lfsrB 2Schritte
					lfsrB=(lfsrB >> 1)^(-(lfsrB & 1) & lfsr_tapsB);
					lfsrB=(lfsrB >> 1)^(-(lfsrB & 1) & lfsr_tapsB);
					//lfsrC 1Schritt
					lfsrC=(lfsrC >> 1)^(-(lfsrC & 1) & lfsr_tapsC);
					//lfsrD Halt
				}
				else
				{
					lfsr_diff++;
					//lfsrA Halt
					//lfsrB 1Schritt
					lfsrB=(lfsrB >> 1)^(-(lfsrB & 1) & lfsr_tapsB);
					//lfsrC 2Schritte
					lfsrC=(lfsrC >> 1)^(-(lfsrC & 1) & lfsr_tapsC);
					lfsrC=(lfsrC >> 1)^(-(lfsrC & 1) & lfsr_tapsC);
					//lfsrD 3Schritte
					lfsrD=(lfsrD >> 1)^(-(lfsrD & 1) & lfsr_tapsD);
					lfsrD=(lfsrD >> 1)^(-(lfsrD & 1) & lfsr_tapsD);
					lfsrD=(lfsrD >> 1)^(-(lfsrD & 1) & lfsr_tapsD);
				}
			}
			// PowerMixer
			MYPIN |= ((lfsrA & 1) << OUT3) | ((lfsrB & 1) << OUT4) | ((lfsrC & 1) << OUT2) | ((lfsrD & 1) << OUT0);
		}		
	}
}
#include <msp430.h>

volatile unsigned int i;
int main(void){
	WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer
	P1DIR |= 0x01;							// Set P1.0 to output direction
	P3DIR |= 0x02;							// Set P3.0 to input, Set P3.1 to output
	for (;;){
		if((P3IN & 0x01) == 0x00){
			P3OUT ^= 0x02;
		}
		else P3OUT = 0x00;
	}
}

#include <msp430.h>

volatile unsigned int i;
int main(void){
	WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer
	P3DIR |= 0xFF;
	P3OUT |= 0xFF;
	for (;;){




	}
}

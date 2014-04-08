#include <msp430.h>

volatile unsigned int i;
int main(void){
	WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog time
	P3DIR |= 0x01;							// Set P1.0 to input, Set P3.0 to output
	P1DIR |= 0x00;
	P1IE |= 0x01;
	_BIS_SR(GIE);
	//while(1);
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void)
{
 P3OUT ^= 0x01;
 P1IFG &= ~0x01;
}

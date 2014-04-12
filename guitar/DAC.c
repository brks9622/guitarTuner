#include <msp430.h>
#include <math.h>

#define samples 50
#define PI 3.14159265

//INIT METHODS
void init_adc(void);
void init_spi(void);
void init_interrupt(void);
void init_waves(int[4][samples]);

//WAVE VARIABLES
int waves[4][samples];
unsigned int conv[2];
int control;
int conv1;
int conv0;
float attenuation;
int frequency;

//MSP
unsigned int index = 0;
float clock_frequency = 16000000;
int conv_result[2];

/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    DCOCTL = CALDCO_16MHZ; // set internal oscillator at 16MHz
    BCSCTL1 = CALBC1_16MHZ; // set internal oscillator at 16MHz
	//init_adc();
	init_spi();
	init_waves(waves);
	init_interrupt();
	_BIS_SR(GIE);
	float damp;
	while(1){
		damp = .5;
	    frequency = (32000-3200)*damp + 3200;
		damp = 1;
		attenuation = 1 * (float)damp + 0xCD;

	}
}


// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	if(index == samples) index = 0;

	control = (int)((waves[0][index]/(float)0x3FF)*(float)attenuation) << 2;
	control |= 0xF000;

	P1OUT &= 0xFE;
	while ((IFG2 & UCB0TXIFG) == 0);
	UCB0TXBUF = (control & 0xFF00) >> 8;

	while ((IFG2 & UCB0TXIFG) == 0);
	UCB0TXBUF = (control & 0x00FF);

	while ((IFG2 & UCB0TXIFG) == 0);
	while ((IFG2 & UCB0TXIFG) == 0);
	P1OUT |= 0x01;
	index++;
	TA0CCR0 = frequency;
}

//ADC10 interrupt Service Routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void) {
	conv0 = conv_result[0];
	conv1 = conv_result[1];
	ADC10CTL0 &= ~ADC10IFG;  // clear interrupt flag
	ADC10CTL0 |= ENC + ADC10SC;
	ADC10SA = (short)&conv_result[0]; // ADC10 data transfer starting address.
}

void init_interrupt(void){
	TA0CCTL0 += CCIE;
	TA0CTL = TASSEL_2 + MC_1 + ID_0;
	TA0CCR0 = 5000;
}

void init_adc(void){
	 ADC10CTL1	|= INCH_1 + CONSEQ_1; //continuous sample mode, MUST BE SET FIRST!
	 ADC10CTL0	|= ADC10SHT_0 + ADC10ON + MSC + ADC10IE ; //sample and hold time, adc on, cont. sample
	 ADC10AE0  	|= 0x03; // select channel A0, A1
	 ADC10DTC1 = 2;
	 ADC10SA = (int)&conv_result[0];
	 ADC10CTL0 	|= ADC10SC + ENC; // start conversions

}

void init_spi(void){
	UCB0CTL1 = UCSWRST;
	P3SEL |= 0x0A;
	P1DIR |= 0x01;
	P1OUT = 0x00;
	UCB0CTL0 |= UCMSB + UCMST + UCSYNC;
	UCB0CTL1 |= UCSSEL_2;
	UCB0BR0 = 0x02;
	UCB0BR1 = 0;
	UCB0CTL1 &= ~UCSWRST;
	UCB0CTL0 |= 0x40;
}

void init_waves(int waves[4][samples]){
	int i;
	float degree = 360.0/samples;
	float radians = degree*PI/180.0;
	float half_samples = (float)samples/2;
	float triangle_slope = (float)0x3FF/(half_samples-1);
	float triangle_yint = (0x3FF*((float)samples - 1))/(half_samples-1);
	float sawtooth_slope = 0x3FF/(samples-1);
	//Sin wave
	for(i = 0; i < samples; i++){
		waves[0][i] = (float)0x1FF*sin((float)radians*(float)i)+(float)0x1FF;
	}

	//Square wave
	for(i = 0; i < samples; i++){
			if(i < half_samples) waves[1][i] = 0;
			else waves[1][i] = 0x3FF;
	}
	//triangle wave
	for(i = 0; i < samples; i++){
			if(i < half_samples) waves[2][i] = triangle_slope*i;
			else waves[2][i] = -triangle_slope*i + triangle_yint;
	}
	//Sawtooth wave
	for(i = 0; i < samples; i++){
		waves[3][i] = sawtooth_slope * i;
	}

}

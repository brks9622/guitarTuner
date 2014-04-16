#include <msp430.h>
#include <math.h>
#define PI 3.14159265
//GLOBAL
volatile int zeroCrossing = 0;
int frequencyMatch = 0;
float reading1;
float reading2;
const int readings = 2;
int msCounter = 9;

//Frequencies

	const int left = 0;
	const int leftMiddle = 0;
	const int middle = 0;
	const int rightMiddle = 0;
	const int right = 0;
	int differential = 0;
	void show_scale(void);


//LCD
void lcd_init(void);
void lcd_command(char);
void lcd_char(char);
char uf_lcd_temp;
char uf_lcd_temp2;
char uf_lcd_x;
void print_note(int);
int hundredDigit;
int tenDigit;
int oneDigit;

//ADC
void adc_init(void);
volatile int previous_read = 0;
volatile int new_read = 0;
void findEarFrequency(int);
void findClosestFrequency(int);
int updateRead = 0;

//SPI
void init_spi(void);
int spi_freq = 0;
unsigned int index = 0;
int control = 0;
int samples = 200;
const int low_E = 82;
	const int A 	= 110;
	const int D		= 147;
	const int G		= 196;
	const int B		= 247;
	const int high_E= 330;

	const int lefreq = 975;
	const int afreq =  727;
	const int dfreq =  544;
	const int gfreq =  408;
	const int bfreq =  322;
	const int hefreq = 242;
const int waves[]={0x200,0x210,0x220,0x230,0x240,0x250,0x25f,0x26f,0x27f,0x28e,0x29e,0x2ad,0x2bc,0x2cb,0x2d9,0x2e8,
		0x2f6,0x304,0x312,0x31f,0x32c,0x339,0x346,0x352,0x35e,0x369,0x374,0x37f,0x38a,0x394,0x39d,0x3a7,
		0x3af,0x3b8,0x3c0,0x3c7,0x3ce,0x3d5,0x3db,0x3e1,0x3e6,0x3eb,0x3ef,0x3f3,0x3f6,0x3f9,0x3fb,0x3fd,
		0x3fe,0x3ff,0x3ff,0x3ff,0x3fe,0x3fd,0x3fb,0x3f9,0x3f6,0x3f3,0x3ef,0x3eb,0x3e6,0x3e1,0x3db,0x3d5,
		0x3ce,0x3c7,0x3c0,0x3b8,0x3af,0x3a7,0x39d,0x394,0x38a,0x37f,0x374,0x369,0x35e,0x352,0x346,0x339,
		0x32c,0x31f,0x312,0x304,0x2f6,0x2e8,0x2d9,0x2cb,0x2bc,0x2ad,0x29e,0x28e,0x27f,0x26f,0x25f,0x250,
		0x240,0x230,0x220,0x210,0x200,0x1ef,0x1df,0x1cf,0x1bf,0x1af,0x1a0,0x190,0x180,0x171,0x161,0x152,
		0x143,0x134,0x126,0x117,0x109,0xfb,0xed,0xe0,0xd3,0xc6,0xb9,0xad,0xa1,0x96,0x8b,0x80,
		0x75,0x6b,0x62,0x58,0x50,0x47,0x3f,0x38,0x31,0x2a,0x24,0x1e,0x19,0x14,0x10,0xc,
		0x9,0x6,0x4,0x2,0x1,0x0,0x0,0x0,0x1,0x2,0x4,0x6,0x9,0xc,0x10,0x14,
		0x19,0x1e,0x24,0x2a,0x31,0x38,0x3f,0x47,0x50,0x58,0x62,0x6b,0x75,0x80,0x8b,0x96,
		0xa1,0xad,0xb9,0xc6,0xd3,0xe0,0xed,0xfb,0x109,0x117,0x126,0x134,0x143,0x152,0x161,0x171,
		0x180,0x190,0x1a0,0x1af,0x1bf,0x1cf,0x1df,0x1ef,0x200};

//INTERRUPT FOR SPI
void init_timerA_interrupt(void);

//INTERRUPT TO COUNT FREQUENCY
void init_timerB_interrupt(void);

//P1.3 INTERRUPT
void init_port_interrupt(void);


int main(void){
	// Stop watchdog timer
	WDTCTL = WDTPW + WDTHOLD;
	// Stop watchdog timer
	//Configure Clocks
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;

	P1DIR &= 0xFB;
	P2DIR &= 0x02;
	int temp = P1IN & 0x04;
		//LCD
		lcd_init();

		//ADC
		adc_init();

		//SPI
		init_spi();

		//INTERRUPT
		init_timerA_interrupt();
		init_timerB_interrupt();
		init_port_interrupt();

	_BIS_SR(GIE);

	for (;;){
		//Tune by ear
		temp = P2IN & 0x02;
		if(temp == 0x02){
			//adc_init();
			ADC10CTL0 |= ADC10IE;	//Enable ADC
			TA0CTL |= MC_1;	//Enable Timer A
			TB0CTL &= ~MC_3;//Disable Timer B

			//possible new note selection
			if(updateRead == 1){
				findEarFrequency(ADC10MEM);
				print_note(frequencyMatch);
			}
		}
		//Tune by line in
		else{
			TB0CTL |= MC_3;	//Enable Timer B
			ADC10CTL0 &= ~ADC10IE;
			TA0CTL &= ~MC_1;//Disable Timer A
			if(updateRead == 1){
				print_note(frequencyMatch);
				lcd_char(0x20);
				lcd_char(hundredDigit+0x30);
				lcd_char(tenDigit+0x30);
				lcd_char(oneDigit+0x30);

				lcd_char(0x20);
				show_scale();
			}
		}
	}
}

//ADC10 interrupt Service Routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void) {
	ADC10CTL0 &= ~ADC10IFG;  // clear interrupt flag
	ADC10CTL0 |= ENC + ADC10SC;
	new_read = ADC10MEM;
	if((new_read > previous_read && new_read - previous_read > 10) || (previous_read > new_read && previous_read-new_read > 10)){
		previous_read = new_read;
		updateRead = 1;
	}
	else updateRead = 0;
}

void findEarFrequency(int newRead){
	if(newRead <= 200){
		frequencyMatch = low_E;
	}

	else if(newRead <= 400){
		frequencyMatch = A;
	}

	else if(newRead <= 600){
		frequencyMatch = D;
	}

	else if(newRead <= 800){
		frequencyMatch = G;
	}

	else if(newRead <= 950){
		frequencyMatch = B;
	}

	else{
		frequencyMatch = high_E;
	}
}

//Zero Crossing Interrupt
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)

{
	zeroCrossing++;
	P1IFG &= ~0x02;                          // P1.1 IFG cleared to catch next rising edge
}

void show_scale(){
	P3DIR |= 0x70;	//3.4,3.5,3.6 output;
	P4DIR |= 0xC0;
	if(differential >=0){
		P3OUT &= 0x00;
		P4OUT &= 0x00;
		if(differential <= 1){
			P3OUT = 0x10;	//Middle
		}
		else if(differential <= 3){
			P3OUT = 0x10;	//Middle
			P3OUT |= 0x20;	//rightMiddle
		}
		else if(differential <= 6){
			P3OUT = 0x20;
		}
		else if(differential <= 10){
			P3OUT = 0x20;
			P3OUT |= 0x40;
		}
		else{
			P3OUT = 0x40;
		}
	}
	else{
		P3OUT &= 0x00;
		P4OUT &= 0x00;
		if(differential >= -1){
			P3OUT = 0x10;	//Middle
		}
		else if(differential >= -3){
			P3OUT = 0x10;	//Middle
			P4OUT |= 0x80;	//leftMiddle
		}
		else if(differential >= -6){
			P4OUT = 0x80;
		}
		else if(differential >= -10){
			P4OUT = 0x80;
			P4OUT |= 0x40;
		}
		else{
			P4OUT = 0x40;
		}
	}
}

//SPI Interrupt
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	control = (waves[index])<<2;
	control |= 0xF000;
	P1OUT &= 0xFE;
	while ((IFG2 & UCB0TXIFG) == 0);
	UCB0TXBUF = (control & 0xFF00) >> 8;

	while ((IFG2 & UCB0TXIFG) == 0);
	UCB0TXBUF = (control & 0x0FF);

	while ((IFG2 & UCB0TXIFG) == 0);
	while ((IFG2 & UCB0TXIFG) == 0);
	P1OUT |= 0x01;
	if(spi_freq == low_E){
		TA0CCR0 = lefreq;
	}
	else if(spi_freq == A){
		TA0CCR0 = afreq;
	}
	else if(spi_freq == D){
		TA0CCR0 = dfreq;
	}
	else if(spi_freq == G){
		TA0CCR0 = gfreq;
	}
	else if(spi_freq == B){
		TA0CCR0 = bfreq;
	}
	else {
		TA0CCR0 = hefreq;
	}
	index++;
	if(index == samples) index = 0;
}

#pragma vector=TIMERB0_VECTOR
__interrupt void Timer_B (void)
{
	int temp = zeroCrossing<<1;	//Calibrated over a .5 second period
	if(msCounter == 0){
			if(temp <= 95){
				frequencyMatch = low_E;
				differential = temp - frequencyMatch;
			}

			else if(temp >= 96 && temp <= 128){
				frequencyMatch = A;
				differential = temp - frequencyMatch;
			}

			else if(temp >= 129 && temp <= 171){
				frequencyMatch = D;
				differential = temp - frequencyMatch;
			}

			else if(temp >= 172 && temp <= 221){
				frequencyMatch = G;
				differential = temp - frequencyMatch;
			}

			else if(temp >= 222 && temp <= 288){
				frequencyMatch = B;
				differential = temp - frequencyMatch;
			}

			else{
				frequencyMatch = high_E;
				differential = temp - frequencyMatch;
			}
			if((temp > previous_read && temp - previous_read > 1) || (previous_read > temp && previous_read-temp > 1)){
				previous_read = temp;
				updateRead = 1;
				oneDigit = temp %10;
				temp = temp/10;
				tenDigit = temp %10;
				temp = temp/10;
				hundredDigit = temp%10;
			}
			else updateRead = 0;
			msCounter = 9;
			zeroCrossing = 0;
	}
	else msCounter--;


}


void init_port_interrupt(void){
	P1IE |= 0x02;		//Pin 1.1
	P1IES|= ~0x02;		//low to hi
	P1IFG &= ~0x20;
}

void init_spi(void){
	UCB0CTL1 |= UCSWRST;  //RESET
	P3SEL |= 0x0A;
	P1SEL |= 0x00;
	P1DIR |= 0x01;
	P1OUT &= 0xFE;
	UCB0CTL0 |= UCMSB + UCMST + UCSYNC;
	UCB0CTL1 |= UCSSEL_2;
	UCB0BR0 = 0x02;
	UCB0BR1 = 0;
	UCB0CTL1 &= ~UCSWRST;
	UCB0CTL0 |= 0x40;
}

void init_timerA_interrupt(void){
	TA0CCTL0 += CCIE;
	TA0CTL = TASSEL_2 + MC_1 + ID_0;
	TACCR0 = 5000;
}

void init_timerB_interrupt(void){
	TB0CCTL0 += CCIE;
	TB0CTL = TASSEL_2 + MC_3 + ID_3;	//Counts up then counts down
	TBCCR0 = 50000;		//Interrupt approximately every .5ms
}

void print_note(int reading){
	lcd_command(0x01);
	//E
	if(reading == low_E){
		lcd_char(0x4C);
		lcd_char(0x4F);
		lcd_char(0x57);
		lcd_char(0x20);
		lcd_char(0x45);

		spi_freq = low_E;
	}
	//A
	else if(reading == A){
		lcd_char(0x41);
		spi_freq = A;
	}
	//D
	else if(reading == D){
		lcd_char(0x44);
		spi_freq = D;
	}
	//G
	else if(reading == G){
		lcd_char(0x47);
		spi_freq = G;
	}
	//B
	else if(reading == B){
		lcd_char(0x42);
		spi_freq = B;
	}
	//E
	else{
		lcd_char(0x48);
		lcd_char(0x49);
		lcd_char(0x47);
		lcd_char(0x48);
		lcd_char(0x20);
		lcd_char(0x45);
		spi_freq = high_E;
	}

}

void adc_init(void){
	ADC10CTL0 =  SREF_0 + ADC10SHT_2 + ADC10ON + ADC10IE + REFON + REF2_5V; // ADC10ON
	ADC10AE0 |= 0x01;                         // P2.0 ADC option select
	ADC10AE0 |= 0x04;
	ADC10CTL0 	|= ADC10SC + ENC; // start conversions
}

void lcd_init(void){
	lcd_command(0x33);
	lcd_command(0x32);
	lcd_command(0x2C);
	lcd_command(0x0C);
	lcd_command(0x01);
}



void lcd_command(char uf_lcd_x){
	P4DIR = 0xFF;
	uf_lcd_temp = uf_lcd_x;
	P4OUT = 0x00;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_x >> 4;
	uf_lcd_x = uf_lcd_x & 0x0F;
	uf_lcd_x = uf_lcd_x | 0x20;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_x & 0x0F;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
	P4OUT = 0x00;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_temp;
	uf_lcd_x = uf_lcd_x & 0x0F;
	uf_lcd_x = uf_lcd_x | 0x20;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_x & 0x0F;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
}


void lcd_char(char uf_lcd_x){
	P4DIR = 0xFF;
	uf_lcd_temp = uf_lcd_x;
	P4OUT = 0x10;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_x >> 4;
	uf_lcd_x = uf_lcd_x & 0x0F;
	uf_lcd_x = uf_lcd_x | 0x30;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_x & 0x1F;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
	P4OUT = 0x10;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_temp;
	uf_lcd_x = uf_lcd_x & 0x0F;
	uf_lcd_x = uf_lcd_x | 0x30;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_x & 0x1F;
	P4OUT = uf_lcd_x;
}

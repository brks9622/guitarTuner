#include <msp430.h>

void lcd_init(void);
void lcd_command(char);
void lcd_char(char);

char uf_lcd_temp;
char uf_lcd_temp2;
char uf_lcd_x;

int main(void){
	WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
	//Configure Clocks
	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL = CALDCO_8MHZ;
	lcd_init();


	lcd_char(0x4F);	//O
	lcd_char(0x75); //u
	lcd_char(0x74); //t
	lcd_char(0x20); //
	lcd_char(0x6F); //o
	lcd_char(0x66); //f
	lcd_char(0x20); //
	lcd_char(0x72); //r
	lcd_char(0x61); //a
	lcd_char(0x6E); //n
	lcd_char(0x67); //g
	lcd_char(0x65); //e
	lcd_char(0x21); //!
	while(1){

}


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

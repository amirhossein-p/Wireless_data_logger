/*
 * TR_v3.c
 *
 * Created: 12/24/2015 8:37:55 AM
 *  Author: Amirhosein
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "nrf24l01p.h"
#include "lcd.h"
#include "eeprom.h"
#include "MicroSD.h"
#include "interrupt.h"

int mode = 1;				// 1: Recv -  0: Trns
unsigned char bufi[11];		// Receive Buffer
unsigned char bufo[11];		// Transmit Buffer
int rcve = 0;				// Receive Flag
unsigned int r11;			// Data From Receive Buffer - Sign 1
unsigned int r12;			// Data From Receive Buffer - Digit 1
unsigned int r13;			// Data From Receive Buffer - Decimal 1
unsigned int r21;			// Data From Receive Buffer - Sign 2
unsigned int r22;			// Data From Receive Buffer - Digit 2
unsigned int r23;			// Data From Receive Buffer - Decimal 2
unsigned int sgn1;			// Temp Sign 1
unsigned int dig1;			// Temp Digit 1
unsigned int dec1;			// Temp Decimal 1
unsigned int sgn2;			// Temp Sign 2
unsigned int dig2;			// Temp Digit 2
unsigned int dec2;			// Temp Decimal 2
unsigned int sgn1u;			// Send To USART
unsigned int dig1u;			// Send To USART
unsigned int r12u;			// Send To USART
unsigned int r13u;			// Send To USART
unsigned int sgn2u;			// Send To USART
unsigned int dig2u;			// Send To USART
unsigned int r22u;			// Send To USART
unsigned int r23u;			// Send To USART
unsigned int dec1u;			// Send To USART
unsigned int dec2u;			// Send To USART
char tmp1[8];				// Temp 1 String
char tmp2[8];				// Temp 2 String
char cid[6];				// DataLogger ID Char
char ch1[6];				// [Temporary]
char ch2[6];				// [Temporary]
unsigned int id;			// DataLogger ID
unsigned int cnt1 = 0;		// Receive data Timeout Counter
unsigned int onst = 0;		// On State
unsigned int ofst = 0;		// Off State
unsigned int ofhl = 0;		// Off State Hold
unsigned int cnt2 = 0;		// Off Command Timeout Counter
unsigned int ofak = 0;		// Off Acknowledge Flag
unsigned int tiot[5] = {100 , 225 , 425 , 690 , 1335};	// Cycle Count For Receive Timeout Err
unsigned int reon[5] = {20 , 45 , 85 , 230 , 455};	    // Resend On Command Count
div_t redi;					// Division Result For Resend On Command
unsigned int tots = 0;		// Timeout State -> 0: No Error , 1: Receive Timeout , 2: Off Ack Timeout
unsigned int ofid = 0;		// Resend Off Command Indicator
unsigned int onid = 0;		// Receive Data Indicator
unsigned int fcs1 = 0;		// Finder Change State - On Mode
unsigned int fcs2 = 0;		// Finder Change State - Off Mode
unsigned int rerv = 0;		// Action Retry Key at Low

unsigned int pg = 0;		// Page Index
int pt1 = 0;				// Pointer - Setting Page
int pt2 = 0;				// Pointer - Interval Page
unsigned int pt3 = 0;		// Pointer - Finder LED
unsigned int fin = 0;		// Finder LED Command
unsigned int bat = 0;		// Battery Status
unsigned int bats = 0;		// Battery Scaled
unsigned int batss = 0;		// Battery Scaled
uint8_t EEMEM eedly;		// EEPROM Memory for Delay Step
uint8_t dly;				// Delay Step

unsigned int usrv = 0;		// USART Receive
unsigned int disp1 = 0;		// Send Display Signs on USART
unsigned int disp2 = 0;		// Send Display Data on USART
unsigned int rsid = 0;		// Resend Indicator
uint8_t EEMEM eeiac;		// EEPROM Memory for Ext. Interface Activate
unsigned int eiac = 0;		// External Interface Activate
unsigned int pt4 = 0;		// Pointer Ext. Interface Activate
uint8_t EEMEM edlac;		// EEPROM Memory for Data Log Activate
unsigned int dlac = 0;		// Data Log Activate
unsigned int pt5 = 0;		// Pointer Data Log Activate
unsigned int dlst = 0;		// Data Log Start
unsigned int pt6 = 0;		// Pointer Data Log Start

unsigned int sdrsp;			// MicroSD Response Code
char sdrpc[6];				// MicroSD Response Code Char
int fnex = 0;				// MicroSD FileNameExist Status
char fncc[9];				// MicroSD File Name Counter Char
char fncd[4];				// MicroSD File Name Counter Char Display
unsigned int fnc = 1;		// MicroSD File Name Counter
unsigned int mfs = 0;		// MicroSD Make File Start - PC.5 Holder
unsigned int mfe = 0;		// MicroSD Make File End
//char da1[13];				// Data To MicroSD Part1	{'1' , 0x09 , '2' , 0x0D , 0x0A}
char da1[23];				// Data To MicroSD 
//char da2[11];				// Data To MicroSD Part2
char dscc[4];				// MicroSD Data String Counter Char
unsigned int dsc = 0;		// MicroSD Data String Counter
unsigned int mehl = 0;		// MicroSD On Err Turn Off
unsigned int dsc1 = 0;		// MicroSD Data String Counter LSB
unsigned int dsc2 = 0;		// MicroSD Data String Counter MSB
unsigned int mems = 0;		// MicroSD Memory Sign Indicator
unsigned int errs = 0;		// MicroSD Error Indicator
unsigned int erak = 0;		// MicroSD Error Acknowledge
unsigned int cnex = 0;		// MicroSD Data String Counter Exceed 9998 Error
unsigned int cnte = 0;		// MicroSD Auto Err Reset Counter
unsigned int onhl = 0;		// MicroSD Auto Err On State Hold

#define pdly			200					// Press Key Delay
#define plc_disable		(PORTC |= (1<<7))	// PLC Disable
#define plc_enable		(PORTC &= ~(1<<7))	// PLC Enable
#define msd_disable		(PORTC |= (1<<6))	// MicroSD Disable
#define msd_enable		(PORTC &= ~(1<<6))	// MicroSD Enable

void receive (void)
{
	if (mode != 1) { mode = 1; nrf_config(mode); }
	if (nrf_state == 1)
	{
		PORTA |= (1<<6);
		rcve = 1;
		for (int i=1; i<11; i++)
		{
			bufi[i] = payload[i];
			_delay_ms(1);
		}
		nrf_state = 0;
		PORTA &= ~(1<<6);
	}
	_delay_ms(10);
}
void transmit (void)
{
	if (mode != 0) { mode = 0; nrf_config(mode); }
	PORTA |= (1<<7);
	for(int j=1; j<11; j++)
	{
		if (nrf_state != 0)
		{
			sent_actived = 1;
			nrf_state = 0;
		}
		payload[0] = j;
		payload[j] = bufo[j];
		_delay_ms(1);
	}
	sent();
	PORTA &= ~(1<<7);
}
void usartsnd (void)
{
	if (eiac != 1)
	{
		disp1 = ((dly << 4) | (rsid << 3) | (fin << 2) | (onid << 1) | ofak);
		disp2 = ((pg << 4) | tots);
		dsc1 = (dsc & 0xFF); 	
		dsc2 = ((errs << 7) | (mems << 6) | (dsc >> 8));
		//batss = ((bat - 222) / 4);
		UDR = 'C';
		_delay_ms(1);
		UDR = bats;
		_delay_ms(1);
		UDR = sgn1u;
		_delay_ms(1);
		UDR = dig1u;
		_delay_ms(1);
		UDR = r12u;
		_delay_ms(1);
		UDR = r13u;
		_delay_ms(1);
		UDR = sgn2u;
		_delay_ms(1);
		UDR = dig2u;
		_delay_ms(1);
		UDR = r22u;
		_delay_ms(1);
		UDR = r23u;
		_delay_ms(1);
		UDR = id;
		_delay_ms(1);
		UDR = disp1;
		_delay_ms(1);
		UDR = disp2;
		_delay_ms(1); 
		UDR = fnc;
		_delay_ms(1);
		UDR = sdrsp; 
		_delay_ms(1);
		UDR = dsc2;  
		_delay_ms(1);
		UDR = dsc1; 
		_delay_ms(1);
		UDR = 'D';
		_delay_ms(1);
		rsid = 0;
		usrv = sd_usrcv(); 
	}
	else _delay_ms(15);
}
void bcksgn(void)
{
	lcd_command(0x40);
	lcd_data(0x04);
	lcd_data(0x08);
	lcd_data(0x1F);
	lcd_data(0x09);
	lcd_data(0x05);
	lcd_data(0x01);
	lcd_data(0x09);
	lcd_data(0x07);
	lcd_command(0x8F);
	lcd_data(0x00);
}
void entmemsgn(void)
{
	if ((pg == 2) | (pg == 3))
	{
		lcd_command(0x48);
		lcd_data(0x01);
		lcd_data(0x01);
		lcd_data(0x05);
		lcd_data(0x0D);
		lcd_data(0x1F);
		lcd_data(0x0C);
		lcd_data(0x04);
		lcd_data(0x00);
		lcd_command(0x80);
		lcd_data(0x01);
	}
	if ((pg == 1) | (pg == 5))
	{
		lcd_command(0x48);
		lcd_data(0x00);
		lcd_data(0x1C);
		lcd_data(0x1E);
		lcd_data(0x1F);
		lcd_data(0x1F);
		lcd_data(0x1F);
		lcd_data(0x1F);
		lcd_data(0x1F);
		lcd_command(0xDC);
		lcd_data(0x01);
	}
}
void selsgn(void)
{
	lcd_command(0x50);
	lcd_data(0x00);
	lcd_data(0x04);
	lcd_data(0x04);
	lcd_data(0x1F);
	lcd_data(0x0E);
	lcd_data(0x04);
	lcd_data(0x00);
	lcd_data(0x00);
	lcd_command(0x8A);
	lcd_data(0x02);
	
	lcd_command(0x58);
	lcd_data(0x00);
	lcd_data(0x00);
	lcd_data(0x04);
	lcd_data(0x0E);
	lcd_data(0x1F);
	lcd_data(0x04);
	lcd_data(0x04);
	lcd_data(0x00);
	lcd_command(0x85);
	lcd_data(0x03);
}
void batsgn(void)
{
	if ((bat > 221) && (bat < 255)) bats = ((bat - 222) / 4);		// DL Battery Scale: Vmax = 3.7v = 254 , Vmin = 3.3v = 222
	else if (bat > 254) bats = 8;
	else bats = 0;
	lcd_command(0x60);
	if (bats > 7) lcd_data(0x09); else lcd_data(0x08);
	if (bats > 6) lcd_data(0x1D); else lcd_data(0x1C);
	if (bats > 5) lcd_data(0x15); else lcd_data(0x14);
	if (bats > 4) lcd_data(0x15); else lcd_data(0x14);
	if (bats > 3) lcd_data(0x15); else lcd_data(0x14);
	if (bats > 2) lcd_data(0x15); else lcd_data(0x14);
	if (bats > 1) lcd_data(0x15); else lcd_data(0x14);
	if (bats > 0) lcd_data(0x1D); else lcd_data(0x1C);
	lcd_command(0xDF);
	lcd_data(0x04);
	
}
void rsdsgn(void)
{
	rsid = 1;
	lcd_command(0x68);
	lcd_data(0x04);
	lcd_data(0x06);
	lcd_data(0x07);
	lcd_data(0x08);
	lcd_data(0x02);
	lcd_data(0x1C);
	lcd_data(0x0C);
	lcd_data(0x04);
	lcd_command(0xDE);
	lcd_data(0x05);
}
void runsgn(unsigned int sel)
{
	if (sel == 0)
	{
		lcd_gotoxy(0,3);
		lcd_puts(" ");
	}
	if (sel == 1)
	{
		lcd_command(0x70);
		lcd_data(0x00);
		lcd_data(0x00);
		lcd_data(0x00);
		lcd_data(0x00);
		lcd_data(0x00);
		lcd_data(0x1C);
		lcd_data(0x1C);
		lcd_data(0x1C);
		lcd_command(0xD0);
		lcd_data(0x06);
	}
	if (sel == 2)
	{
		lcd_command(0x70);
		lcd_data(0x00);
		lcd_data(0x00);
		lcd_data(0x00);
		lcd_data(0x10);
		lcd_data(0x18);
		lcd_data(0x1C);
		lcd_data(0x18);
		lcd_data(0x10);
		lcd_command(0xD0);
		lcd_data(0x06);
	}
}
void ledsgn(void)
{
	lcd_command(0x78);
	lcd_data(0x00);
	lcd_data(0x04);
	lcd_data(0x15);
	lcd_data(0x0E);
	lcd_data(0x0E);
	lcd_data(0x15);
	lcd_data(0x04);
	lcd_data(0x00);
	lcd_command(0xDD);
	lcd_data(0x07);
}
void msdfile(void)
{
	if (dlac != 1)
	{
		plc_disable; msd_enable;
		_delay_ms(100);
		mfs = 1;
		while (mfs == 1)
		{
			sprintf(fncc , "WDL_%1u.xlt" , fnc);
			itoa(fnc,fncd,10); lcd_gotoxy(11,3); lcd_puts(fncd);
			fnex = sd_fnexist(9 , fncc);
			_delay_ms(200);
			if (fnex == 107)
			{
				sd_new(9 , fncc);
				_delay_ms(200);
				sdrsp = sd_response();
				_delay_ms(100);
				mfs = 0;
				mfe = 1;
			}
			if (fnex == 127) fnc++;
			sd_open(9 , fncc);          //////////////
			_delay_ms(100);             //////////////
			sdrsp = sd_response();
			_delay_ms(100);
			if (sdrsp != 0) mfs = 0;
		}
		plc_enable; msd_disable;
		dsc = 1;
	}
}
void msddata(void)
{
	if (sgn1u == 0) sprintf(tmp1,"+%2u.%4u",dig1u,dec1u);
	if (sgn1u == 1) sprintf(tmp1,"-%2u.%4u",dig1u,dec1u);
	if (sgn2u == 0) sprintf(tmp2,"+%2u.%4u",dig2u,dec2u);
	if (sgn2u == 1) sprintf(tmp2,"-%2u.%4u",dig2u,dec2u);
	if (tmp1[1] == ' ') tmp1[1] = '0';
	if (tmp1[4] == ' ') tmp1[4] = '0';
	if (tmp1[5] == ' ') tmp1[5] = '0';
	if (tmp1[6] == ' ') tmp1[6] = '0';
	if (tmp1[7] == ' ') tmp1[7] = '0';
	if (tmp2[1] == ' ') tmp2[1] = '0';
	if (tmp2[4] == ' ') tmp2[4] = '0';
	if (tmp2[5] == ' ') tmp2[5] = '0';
	if (tmp2[6] == ' ') tmp2[6] = '0';
	if (tmp2[7] == ' ') tmp2[7] = '0';	
	if (dlst == 1)
	{
		itoa(dsc,dscc,10);
		/*da1[0] = dscc[0]; da1[1] = dscc[1]; da1[2] = dscc[2]; da1[3] = dscc[3];
		da1[4] = 0x09; 
		da1[5]  = tmp1[0];     da1[6]  = tmp1[1]; da1[7]  = tmp1[2];     da1[8]  = tmp1[3];      da1[9]  = tmp1[4]; da1[10]  = tmp1[5]; da1[11]  = tmp1[6]; da1[12]  = tmp1[7]; 
		da2[0] = 0x09;		
		da2[1]  = tmp2[0];     da2[2]  = tmp2[1]; da2[3]  = tmp2[2];     da2[4]  = tmp2[3];      da2[5]  = tmp2[4]; da2[6]   = tmp2[5]; da2[7]   = tmp2[6]; da2[8]   = tmp2[7];	
		da2[9] = 0x0D; da2[10] = 0x0A;*/	
		
		da1[0]  = dscc[0]; 
		da1[1]  = dscc[1]; 
		da1[2]  = dscc[2]; 
		da1[3]  = dscc[3];
		da1[4]  = 0x09;
		da1[5]  = tmp1[0];     
		da1[6]  = tmp1[1]; 
		da1[7]  = tmp1[2];     
		da1[8]  = tmp1[3];      
		da1[9]  = tmp1[4]; 
		da1[10] = tmp1[5]; 
		da1[11] = tmp1[6]; 
		da1[12] = tmp1[7];
		da1[13] = 0x09;
		da1[14] = tmp2[0];
		da1[15] = tmp2[0];
		da1[16] = tmp2[1]; 
		da1[17] = tmp2[2];     
		da1[18] = tmp2[3];      
		da1[19] = tmp2[4]; 
		da1[20] = tmp2[5]; 
		da1[21] = tmp2[6]; 
		da1[22] = tmp2[7];
		//da1[23] = 0x0D; 
		//da1[24] = 0x0A;
		
		
		plc_disable; msd_enable;
		_delay_ms(50);
		if (mfe == 1)
		{
			//sd_open(9 , fncc);
			//_delay_ms(50);
			//sdrsp = sd_response();
			//_delay_ms(50);
			if (sdrsp == 0)
			{
				/*sd_append(13 , da1);
				_delay_ms(50);	
				sdrsp = sd_response();
				_delay_ms(50);
				sd_append(11 , da2);
				_delay_ms(100);
				sdrsp = sd_response();
				_delay_ms(50);*/
				
				sd_append(23 , da1);
				_delay_ms(50);
				sdrsp = sd_response();
				_delay_ms(50);					
			}
		}
		plc_enable; msd_disable;
		dsc++;
	}
}

void interface (void)
{
	if (pg == 0)									// Home
	{
		lcd_clrscr();
		lcd_home();
		lcd_puts("Run         Sett");
		lcd_gotoxy(5,2);
		lcd_puts("WDL v3");	
			//lcd_gotoxy(0,3);
			//lcd_puts(ch1);
			//lcd_gotoxy(4,3);
			//lcd_puts(ch2);	
			//lcd_gotoxy(8,3);
			//lcd_puts(dscc);
		if (onid == 1) runsgn(2);
		if (ofak == 1) runsgn(1);
		if ((PINA & (1<<0)) || (usrv == 1)) {pg = 2; usrv = 0; _delay_ms(pdly);}  
		_delay_ms(100);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
	}
	if (pg == 1)									// Run
	{
		lcd_clrscr();
		lcd_home();
		lcd_puts("Stop        Fin.");
		lcd_gotoxy(0,1);
		lcd_puts("T1:");
		lcd_gotoxy(3,1);
		lcd_puts(tmp1);		// String in msddata				
		lcd_gotoxy(0,2);							
		lcd_puts("T2:");
		lcd_gotoxy(3,2);
		lcd_puts(tmp2);		// String in msddata
		if (dlst == 1) {lcd_gotoxy(8,3); lcd_puts(dscc);}	
		if (dlst == 0) {lcd_gotoxy(8,3); lcd_puts("    "); mems = 0;}	
		batsgn();
		//itoa(bat,ch1,10); lcd_gotoxy(11,3); lcd_puts(ch1);
		if (onid == 1) runsgn(2);
		if (ofak == 1) runsgn(1);
		if ((redi.rem == (reon[dly] - 1)) && (onst == 1)) {rsdsgn(); onid = 0; runsgn(0);}
		if (((PINA & (1<<0)) && (fin == 0) && (pt3 == 0)) || ((usrv == 1) && (fin == 0) && (pt3 == 0)))	{fin = 1; pt3 = 1; fcs1 = 1; usrv = 0; _delay_ms(10);}
		if (((PINA & (1<<0)) == 0)/* || ((pt3 == 1) && (usrv == 0))*/) pt3 = 0;
		if (((PINA & (1<<0)) && (fin == 1) && (pt3 == 0)) || ((usrv == 1) && (fin == 1) && (pt3 == 0)))	{fin = 0; pt3 = 1; fcs1 = 1; usrv = 0; _delay_ms(10);}
		if (((PINA & (1<<0)) == 0)/* || ((pt3 == 1) && (usrv == 0))*/) pt3 = 0;						
		if (fin == 1) ledsgn();		
		if (dlst == 1) {entmemsgn(); mems = 1;}
		_delay_ms(100);
	}
	if (pg == 2)									// Sett
	{
		lcd_clrscr();
		bcksgn();
		entmemsgn();
		selsgn();
		if ((pt1 == 0) | (pt1 == 1) | (pt1 == 2))
		{
			lcd_gotoxy(1,1);
			lcd_puts("Interval");
			lcd_gotoxy(1,2);
			lcd_puts("Finder");
			lcd_gotoxy(1,3);
			lcd_puts("ID");
			if (pt1 == 0)
			{
				lcd_gotoxy(0,1);
				lcd_puts(">");
				if ((PINA & (1<<3)) || (usrv == 4)) {pg = 3; usrv = 0; _delay_ms(pdly);}
			}
			if (pt1 == 1)
			{
				lcd_gotoxy(0,2);
				lcd_puts(">");
				if (((PINA & (1<<3)) && (fin == 0) && (pt3 == 0)) || ((usrv == 4) && (fin == 0) && (pt3 == 0)))	{fin = 1; pt3 = 1; fcs2 = 1; usrv = 0;}
				if (((PINA & (1<<3)) == 0) /*|| ((pt3 == 1) && (usrv == 0))*/) pt3 = 0;
				if (((PINA & (1<<3)) && (fin == 1) && (pt3 == 0)) || ((usrv == 4) && (fin == 1) && (pt3 == 0)))	{fin = 0; pt3 = 1; fcs2 = 1; usrv = 0;}
				if (((PINA & (1<<3)) == 0)/* || ((pt3 == 1) && (usrv == 0))*/) pt3 = 0;
				if (fin == 1) ledsgn();
			}
			if (pt1 == 2)
			{
				lcd_gotoxy(0,3);
				lcd_puts(">");
				if ((PINA & (1<<3)) || (usrv == 4)) {pg = 4; usrv = 0; _delay_ms(pdly);}
			}
		}
		if ((pt1 == 3) | (pt1 == 4))
		{
			lcd_gotoxy(1,1);
			lcd_puts("ID");
			lcd_gotoxy(1,2);
			lcd_puts("Interface");
			lcd_gotoxy(1,3);
			lcd_puts("Data Log");
			if (pt1 == 3)
			{
				lcd_gotoxy(0,2);
				lcd_puts(">");
				eiac = eeprom_read_byte(&eeiac);
				if ((PINA & (1<<3)) && (eiac != 1) && (pt4 == 0))	{eeprom_write_byte(&eeiac, 1); pt4 = 1;}
				if ((PINA & (1<<3)) == 0) pt4 = 0;
				if ((PINA & (1<<3)) && (eiac == 1) && (pt4 == 0))	{eeprom_write_byte(&eeiac, 0); pt4 = 1;}
				if ((PINA & (1<<3)) == 0) pt4 = 0;	
			}
			if (pt1 == 4)
			{
				lcd_gotoxy(0,3);
				lcd_puts(">");
				dlac = eeprom_read_byte(&edlac);
				if ((PINA & (1<<3)) && (dlac != 1) && (pt5 == 0))	{eeprom_write_byte(&edlac, 1); pt5 = 1;}
				if ((PINA & (1<<3)) == 0) pt5 = 0;
				if ((PINA & (1<<3)) && (dlac == 1) && (pt5 == 0))	{eeprom_write_byte(&edlac, 0); pt5 = 1;}
				if ((PINA & (1<<3)) == 0) pt5 = 0;
			}
			if (eiac == 1) {lcd_gotoxy(11,2); lcd_puts("Off");}
			if (eiac == 0) {lcd_gotoxy(11,2); lcd_puts("On");}
			if (dlac == 1) {lcd_gotoxy(11,3); lcd_puts("Off");}
			if (dlac == 0) {lcd_gotoxy(11,3); lcd_puts("On");}
		}
		_delay_ms(100);
		if ((PINA & (1<<1)) || (usrv == 2)) {pt1++; usrv = 0; _delay_ms(pdly);}
		if (pt1 > 4) pt1 = 0;
		if ((PINA & (1<<2)) || (usrv == 3)) {pt1--; usrv = 0; _delay_ms(pdly);}
		if (pt1 < 0) pt1 = 4;
		if ((PINA & (1<<0)) || (usrv == 1)) {pg = 0; usrv = 0; _delay_ms(pdly);}
	}
	if (pg == 3)									// Interval
	{
		lcd_clrscr();
		lcd_gotoxy(11,3);
		dly = eeprom_read_byte(&eedly);
		if (dly == 0) lcd_puts("(2s )");
		if (dly == 1) lcd_puts("(5s )");
		if (dly == 2) lcd_puts("(10s)");
		if (dly == 3) lcd_puts("(30s)");
		if (dly == 4) lcd_puts("(1m )");
		if ((pt2 == 0) | (pt2 == 1) | (pt2 == 2))
		{
			bcksgn();
			entmemsgn();
			selsgn();
			lcd_gotoxy(1,1);
			lcd_puts("2s");
			lcd_gotoxy(1,2);
			lcd_puts("5s");
			lcd_gotoxy(1,3);
			lcd_puts("10s");
			if (pt2 == 0)
			{
				lcd_gotoxy(0,1);
				lcd_puts(">");
				if ((PINA & (1<<3)) || (usrv == 4)) {eeprom_write_byte(&eedly, 0); usrv = 0; _delay_ms(pdly);}
			}
			if (pt2 == 1)
			{
				lcd_gotoxy(0,2);
				lcd_puts(">");
				if ((PINA & (1<<3)) || (usrv == 4)) {eeprom_write_byte(&eedly, 1); usrv = 0; _delay_ms(pdly);}
			}
			if (pt2 == 2)
			{
				lcd_gotoxy(0,3);
				lcd_puts(">");
				if ((PINA & (1<<3)) || (usrv == 4)) {eeprom_write_byte(&eedly, 2); usrv = 0; _delay_ms(pdly);}
			}
		}
		if ((pt2 == 3) | (pt2 == 4))
		{
			bcksgn();
			entmemsgn();
			selsgn();
			lcd_gotoxy(1,1);
			lcd_puts("10s");
			lcd_gotoxy(1,2);
			lcd_puts("30s");
			lcd_gotoxy(1,3);
			lcd_puts("1m");
			if (pt2 == 3)
			{
				lcd_gotoxy(0,2);
				lcd_puts(">");
				if ((PINA & (1<<3)) || (usrv == 4)) {eeprom_write_byte(&eedly, 3); usrv = 0; _delay_ms(pdly);}
			}
			if (pt2 == 4)
			{
				lcd_gotoxy(0,3);
				lcd_puts(">");
				if ((PINA & (1<<3)) || (usrv == 4)) {eeprom_write_byte(&eedly, 4); usrv = 0; _delay_ms(pdly);}
			}
		}
		if ((PINA & (1<<1)) || (usrv == 2)) {pt2++; usrv = 0; _delay_ms(pdly);}
		if (pt2 > 4) pt2 = 0;
		if ((PINA & (1<<2)) || (usrv == 3)) {pt2--; usrv = 0; _delay_ms(pdly);}
		if (pt2 < 0) pt2 = 4;
		if ((PINA & (1<<0)) || (usrv == 1)) {pg = 2; usrv = 0; _delay_ms(pdly);}
		_delay_ms(100);
	}
	if (pg == 4)									// ID
	{
		itoa(id, cid, 10);
		lcd_clrscr();
		bcksgn();
		lcd_gotoxy(0,2);
		lcd_puts("ID:");
		lcd_gotoxy(3,2);
		lcd_puts(cid);
		if ((PINA & (1<<0)) || (usrv == 1)) {pg = 2; usrv = 0; _delay_ms(pdly);}
		_delay_ms(100);
	}
	if (pg == 5)									// Stop
	{
		lcd_clrscr();
		lcd_home();
		lcd_puts("Run DL");
		bcksgn();
		lcd_gotoxy(0,1);
		lcd_puts("T1: --");
		lcd_gotoxy(0,2);
		lcd_puts("T2: --");
		if (onid == 1) runsgn(2);
		if (ofak == 1) runsgn(1);
		if (ofid == 1) {rsdsgn(); onid = 0; runsgn(0);}
		if (dlst == 1) {entmemsgn(); mems = 1;}
		if (dlst == 0) mems = 0;	
		_delay_ms(100);
		ofid = 0;
		if (ofid == 0) {lcd_gotoxy(14,3); lcd_puts(" ");}
		if ((PINA & (1<<0)) || (usrv == 1)) {pg = 0; usrv = 0; _delay_ms(pdly);}
			
		if (((PINA & (1<<2)) && (dlac != 1) && (dlst == 0) && (pt6 == 0)) || ((usrv == 3) && (dlst == 0) && (pt6 == 0)))	{dlst = 1; pt6 = 1; usrv = 0; erak = 0; msdfile(); _delay_ms(pdly);}
		if (((PINA & (1<<2)) == 0)  && (dlac != 1)) pt6 = 0;
		if (((PINA & (1<<2)) && (dlac != 1) && (dlst == 1) && (pt6 == 0)) || ((usrv == 3) && (dlst == 1) && (pt6 == 0)))	{dlst = 0; pt6 = 1; usrv = 0; fnc = 1;   _delay_ms(pdly);}
		if (((PINA & (1<<2)) == 0)  && (dlac != 1)) pt6 = 0;
	}
	if (pg == 6)									// Error - Timeout
	{
		lcd_clrscr();
		lcd_home();
		lcd_puts("Retry      Reset");
		lcd_gotoxy(5,2);
		lcd_puts("ERROR!");
		if (tots == 1) {lcd_gotoxy(0,3); lcd_puts("Receive Timeout."); onst = 0; _delay_ms(pdly);}
		if (tots == 2) {lcd_gotoxy(0,3); lcd_puts("Off Ack Timeout."); ofhl = 0; _delay_ms(pdly);}
		if ((PINA & (1<<0)) || (usrv == 1)) {cnt1 = 0; cnt2 = 0; pg = 0; usrv = 0; _delay_ms(pdly);}
		if (((PINA & (1<<3)) && (tots == 1)) || ((usrv == 4) && (tots == 1))) {rerv = 1; usrv = 0; _delay_ms(pdly);}
		if (((rerv == 1) && ((PINA & (1<<3)) == 0)) || ((rerv == 1) && (usrv == 0))) {cnt1 = 0; pg = 1; onst = 1; tots = 0; rerv = 0;}
		if (((PINA & (1<<3)) && (tots == 2)) || ((usrv == 4) && (tots == 2))) {cnt2 = 0; pg = 5; ofhl = 1; usrv = 0; _delay_ms(pdly);}
	}
	if (pg == 7)									// Error - MicroSD
	{
		lcd_clrscr();
		lcd_gotoxy(1,2);
		lcd_puts("MicroSD Error!");
		if (cnex != 1)
		{
			lcd_home();
			lcd_puts("Rst  Rsp");
			lcd_gotoxy(1,3);
			lcd_puts("Code:");
			lcd_gotoxy(6,3);
			itoa(sdrsp, sdrpc, 10);
			lcd_puts(sdrpc);
		}
		if (cnex == 1)
		{
			onst = 0;
			lcd_gotoxy(1,3);
			lcd_puts("Counter > 9999");
		}
		bcksgn();
		if (onid == 1) runsgn(2);
		if (ofak == 1) runsgn(1);
		if (ofid == 1) {rsdsgn(); onid = 0; runsgn(0);}
		_delay_ms(100);	
		//onst = 0;
		dlst = 0;
		if ((PINA & (1<<0)) || (usrv == 1)) {usrv = 0; erak = 1; dsc = 0; cnex = 0; pg = 0; _delay_ms(pdly);}
		if (cnex != 1)
		{
			if (onst == 1) {onhl = 1; onst = 0;}
			if ((PINA & (1<<2)) || (usrv == 3))
			{
				usrv = 0;
				plc_disable; msd_enable;
				sdrsp = sd_response();
				_delay_ms(500);
				plc_enable; msd_disable;
			}
			if ((PINA & (1<<3)) || (usrv == 4))
			{
				usrv = 0;
				sd_rset;
				_delay_ms(200);
				sd_set;
			}				
			if ((onhl == 1) && (cnte > 20))							// Auto Reset
			{
				sd_rset;
				_delay_ms(200);
				sd_set;
				_delay_ms(50);
				plc_disable; msd_enable;
				_delay_ms(50);
				sdrsp = sd_response();
				_delay_ms(50);
				plc_enable; msd_disable;
				lcd_gotoxy(6,3);
				itoa(sdrsp, sdrpc, 10);
				lcd_puts(sdrpc);
				_delay_ms(50);
				cnte = 0;
				if (sd_err == 0)
				{
					cnte = 0;
					msdfile();
					onst = 1;
					dlst = 1;
					onhl = 0;
					pg = 1;
				}
			}
			cnte++;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(void)
{
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
	UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);
	UBRRL = 51; // 9600 @ 8MHz
	DDRA = 0b11000000;
	DDRC = 0b11100000;
	nrf_config(mode);
	lcd_init(LCD_DISP_ON);
	dly = eeprom_read_byte(&eedly);
	eiac = eeprom_read_byte(&eeiac);
	dlac = eeprom_read_byte(&edlac);
	sei();
	PORTA |= (1<<6);
	PORTA |= (1<<7);
	_delay_ms(100);
	PORTA &= ~(1<<6);
	PORTA &= ~(1<<7);
	sd_init();	
	plc_enable; 
	msd_disable;
	while(1)
	{
		interface();
		receive();
		if(onst == 1) cnt1++;
		//_delay_ms(10);
		usartsnd();													// Write Data to USART & 15ms Delay
		if (rcve == 1)												// Receive...
		{
			//usartsnd();												// Write Data to USART									
			if ((bufi[1] == 'A') &&  (bufi[10] == 'B'))				// Receive & Decode Data
			{
				cnt1 = 0;
				onid = 1;
				r11 = bufi[2]; 
				r12 = bufi[3];
				r13 = bufi[4];
				bat = bufi[5];
				r21 = bufi[6];
				r22 = bufi[7];
				r23 = bufi[8];
				id  = bufi[9];							
				sgn1 = (r11 >> 7);
				dig1 = (r11 & 0b1111111);
				dec1 = ((r12 << 8) | r13);				
				sgn2 = (r21 >> 7);
				dig2 = (r21 & 0b1111111);
				dec2 = ((r22 << 8) | r23);	
				sgn1u = sgn1;
				dig1u = dig1;
				r12u  = r12;
				r13u  = r13;
				sgn2u = sgn2;
				dig2u = dig2;
				r22u  = r22;
				r23u  =	r23;
				dec1u = dec1;
				dec2u = dec2;	
				rcve = 0;
				if (onst == 1) msddata();
			}
			if ((bufi[1] == 'K') && (bufi[10] == 'K'))				// Receive Off Acknowledge
			{
				ofak = 1;
				ofhl = 0;
				cnt2 = 0;
				ofid = 0;
				onid = 0;
				rcve = 0;
			}
			if ((bufi[1] != 'K') | (bufi[10] != 'K')) ofak = 0;
		}
		if ( ((pg == 0) && (PINA & (1<<3))) ||					// Send On Command
		     ((pg == 5) && (PINA & (1<<3))) ||
		     ((pg == 1) && (redi.rem == (reon[dly] - 1)) && (onst == 1)) ||
			 ((pg == 0) && (usrv == 4)) ||
			 ((pg == 5) && (usrv == 4)) )
		{
			pg = 1;
			onst = 1;
			usrv = 0;
			bufo[1]  = 'N';
			bufo[2]  = fin;
			bufo[3]  = dly;
			bufo[4]  = 'N';
			bufo[5]  = 'N';
			bufo[6]  = 'N';
			bufo[7]  = 'N';
			bufo[8]  = 'N';
			bufo[9]  = 'N';
			bufo[10] = 'N';
			transmit();
			_delay_ms(100);
		}
		if (fcs1 == 1)
		{
			for (int i=0; i<10; i++)
			{
				bufo[1]  = 'N';
				bufo[2]  = fin;
				bufo[3]  = dly;
				bufo[4]  = 'N';
				bufo[5]  = 'N';
				bufo[6]  = 'N';
				bufo[7]  = 'N';
				bufo[8]  = 'N';
				bufo[9]  = 'N';
				bufo[10] = 'N';
				transmit();
				_delay_ms(100);
			}
			fcs1 = 0;
		}
		if (ofhl == 1) ofst = 1;									// Send Off Command
		if (((pg == 1) && (PINA & (1<<3))) || ((pg == 1) && (usrv == 4)) || (mehl == 1)) {ofst = 1; ofhl = 1; usrv = 0;}
		if ((ofst == 1) || (fcs2 == 1))
		{
			if (fcs2 != 1) pg = 5;
			onst = 0;
			mehl = 0;
			cnt2++;
			if (cnt2 > 1) ofid = 1;
			bufo[1]  = 'F';
			bufo[2]  = fin;
			bufo[3]  = dly;
			bufo[4]  = 'F';
			bufo[5]  = 'F';
			bufo[6]  = 'F';
			bufo[7]  = 'F';
			bufo[8]  = 'F';
			bufo[9]  = 'F';
			bufo[10] = 'F';
			transmit();
			usartsnd();
			for (int i=0; i<100; i++)
			{
				receive();
				_delay_ms(2);
				if (rcve == 1) i = 99;
			}
			ofst = 0;
			fcs2 = 0;
		}
		if (cnt1 > tiot[dly]) {tots = 1;	pg = 6;}							// Timeout Error
		if (cnt2 > 6) {tots = 2; pg = 6;}
		if (((cnt1 > tiot[dly]) || (cnt2 > 6)) == 0) tots = 0;
		redi = div(cnt1, reon[dly]);											// Resend On Command
		if ( ((dlac != 1) && sd_err && (erak == 0)) ||
		     ((dlac != 1) && (cnex == 1))            )							// MicroSD Error
		{
			pg = 7;	
			errs = 1;					
			//if (onst == 1) mehl = 1;
		}
		if (sd_err == 0) errs = 0;
		if (dsc > 9998) cnex = 1;												// MicroSD Counter Exceeded
	}
	
}
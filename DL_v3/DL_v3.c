/*
 * DL_v3.c
 *
 * Created: 12/24/2015 8:35:35 AM
 *  Author: Amirhosein
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include "nrf24l01p.h"
#include "ds18b20.h"

int mode = 1; // 1: Recv -  0: Trns
unsigned char bufi[11];
unsigned char bufo[11];
unsigned char ofack[11] = {'K' , 'K' , 'K' , 'K' , 'K' , 'K' , 'K' , 'K' , 'K' , 'K' , 'K' };
unsigned int dltb[5] = {42 , 340 , 840 , 2840 , 5830};
int rcv = 0;						// Receive State
unsigned int d1;					// Temp1 - Sign
unsigned int d2;					// Temp1 - Digit
unsigned int d3;					// Temp1 - Decimal
unsigned int d4;					// Temp2 - Sign
unsigned int d5;					// Temp2 - Digit
unsigned int d6;					// Temp2 - Decimal
unsigned int adl;					// Analog LSB
unsigned int adh;					// Analog MSB
unsigned int ad;					// Analog / 8
unsigned int ad1;					// Analog / 8
unsigned int id = 0xA0;				// Data Logger ID
unsigned int dly = 0;				// Delay Step
div_t fin;							// Finder LED Command

void receive (void)
{
	if (mode != 1) { mode = 1; nrf_config(mode); }
	if (nrf_state == 1)
	{
		PORTC |= (1<<4);
		rcv = 1;
		for (int i=1; i<11; i++)
		{
			bufi[i] = payload[i];
			_delay_ms(1);
		}
		nrf_state = 0;
		PORTC &= ~(1<<4);
	}
	_delay_ms(10);
}

void transmit (void)
{
	if (mode != 0) { mode = 0; nrf_config(mode); }
	PORTC |= (1<<5);
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
	PORTC &= ~(1<<5);
}


int main(void)
{
	unsigned int tmpr1[3];				// DS18B20 -1 Data
	unsigned int tmpr2[3];				// DS18B20 -2 Data
	ADMUX = 0b11000000;
	ADCSRA = 0b11100000;
	UCSRB = (1 << RXEN) | (1 << TXEN);
	UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);
	UBRRL = 25; // 19200 @ 8MHz
	DDRC |= (1<<4) | (1<<5);
	DDRB |= (1<<1);
	nrf_config(mode);
	PORTC |= (1<<4);
	PORTC |= (1<<5);
	_delay_ms(100);
	PORTC &= ~(1<<4);
	PORTC &= ~(1<<5);
	while(1)
	{
		adh = ADCH;
		adl = ADCL;
		receive();
		for (int z=0; z<dltb[dly]; z++)						// Active Delay , Finder LED
		{
			receive();
			adh = ADCH;
			adl = ADCL;
			dly = bufi[3];
			PORTB &= ~(1<<1);
			fin = div(z,40);
			if ((fin.rem == 0) && bufi[2] == 1) PORTB |= (1<<1);
			if (rcv == 1) z = dltb[dly];
		}
		if (rcv == 1)									// Write to USART
		{
			for (int x=1; x < 11; x++)
			{
				UDR = bufi[x];
				//_delay_ms(1);
			}
			rcv = 0;
		}
		if ((bufi[1] == 'N') && (bufi[10] == 'N'))		// Send Data
		{
			ds18b20_read_1(tmpr1);
			_delay_ms(2);
			ds18b20_read_2(tmpr2);
			d1 = ((tmpr1[0] << 7) | tmpr1[1]);
			d2 = (tmpr1[2] >> 8);
			d3 = (tmpr1[2] & 0b11111111);
			d4 = ((tmpr2[0] << 7) | tmpr2[1]);
			d5 = (tmpr2[2] >> 8);
			d6 = (tmpr2[2] & 0b11111111);
			//ad = (((adh << 8) | adl) / 128);
			ad = (((adh << 8) | adl) / 4);
			bufo[1]  = 'A';
			bufo[2]  = d1;
			bufo[3]  = d2;
			bufo[4]  = d3;
			bufo[5]  = ad;
			bufo[6]  = d4;
			bufo[7]  = d5;
			bufo[8]  = d6;
			bufo[9]  = id;
			bufo[10] = 'B';
			transmit();
		}
		if ((bufi[1] == 'F') && (bufi[10] == 'F'))		// Off ACK
		{
			_delay_ms(10);
			for (int x=1; x < 11; x++)
			{
				bufo[x] = ofack[x];
				_delay_ms(1);
			}
			bufi[1] = 0;
			transmit();
			_delay_ms(10);
		}
	}
	
}

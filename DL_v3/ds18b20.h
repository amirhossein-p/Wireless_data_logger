/*
 * ds18b20.h
 * DS18B20 Thermometer Library
 *
 * Function  ds18b20_read() Returns a data array including 
 * Sign, Integer & Decimal value of temperature.
 *
 * Created: 11/24/2015 3:55:05 PM
 *  Author: Amirhosein
 */ 



#include <util/delay.h>

#define thr_pin_1			PINC
#define thr_dq_1			2
#define thr_input_1			DDRC &= ~(1<<thr_dq_1)
#define thr_output_1		DDRC |= (1<<thr_dq_1)	
#define thr_low_1			PORTC &= ~(1<<thr_dq_1)
#define thr_high_1			PORTC |= (1<<thr_dq_1)

#define thr_pin_2			PINC
#define thr_dq_2			3
#define thr_input_2			DDRC &= ~(1<<thr_dq_2)
#define thr_output_2		DDRC |= (1<<thr_dq_2)
#define thr_low_2			PORTC &= ~(1<<thr_dq_2)
#define thr_high_2			PORTC |= (1<<thr_dq_2)

#define cmd_convtemp		0x44
#define cmd_rscratchpad		0xbe
#define cmd_wscratchpad		0x4e
#define cmd_cpyscratchpad	0x48
#define cmd_receeprom		0xb8
#define cmd_prwrsupply		0xb4
#define cmd_searchrom		0xf0
#define cmd_readrom			0x33
#define cmd_matchrom		0x55
#define cmd_skiprom			0xcc
#define cmd_alarmsearch		0xec
//////////////////////////////////////////////////////////////////////////
uint8_t thr_reset_1(void)
{
	uint8_t i;
	thr_low_1;
	thr_output_1;
	_delay_us(480);
	thr_input_1;
	_delay_us(60);
	i = (thr_pin_1 & (1<<thr_dq_1));
	_delay_us(420);
	return i;
}

uint8_t thr_reset_2(void)
{
	uint8_t i;
	thr_low_2;
	thr_output_2;
	_delay_us(480);
	thr_input_2;
	_delay_us(60);
	i = (thr_pin_2 & (1<<thr_dq_2));
	_delay_us(420);
	return i;
}
//////////////////////////////////////////////////////////////////////////
void thr_wrbit_1 (uint8_t bi)
{
	thr_low_1;
	thr_output_1;
	_delay_us(1);
	if (bi) thr_input_1;
	_delay_us(60);
	thr_input_1;
}

void thr_wrbit_2 (uint8_t bi)
{
	thr_low_2;
	thr_output_2;
	_delay_us(1);
	if (bi) thr_input_2;
	_delay_us(60);
	thr_input_2;
}
//////////////////////////////////////////////////////////////////////////
uint8_t thr_rdbit_1 (void)
{
	uint8_t bit = 0;
	thr_low_1;
	thr_output_1;
	_delay_us(1);
	thr_input_1;
	_delay_us(14);
	if (thr_pin_1 & (1<<thr_dq_1)) bit = 1;
	_delay_us(45);
	return bit;
}

uint8_t thr_rdbit_2 (void)
{
	uint8_t bit = 0;
	thr_low_2;
	thr_output_2;
	_delay_us(1);
	thr_input_2;
	_delay_us(14);
	if (thr_pin_2 & (1<<thr_dq_2)) bit = 1;
	_delay_us(45);
	return bit;
}
//////////////////////////////////////////////////////////////////////////
uint8_t thr_rdbyte_1 (void)
{
	uint8_t i = 8;
	uint8_t n = 0;
	while (i--)
	{
		n >>= 1;
		n |= (thr_rdbit_1() << 7);
	}
	return n;
}

uint8_t thr_rdbyte_2 (void)
{
	uint8_t i = 8;
	uint8_t n = 0;
	while (i--)
	{
		n >>= 1;
		n |= (thr_rdbit_2() << 7);
	}
	return n;
}
//////////////////////////////////////////////////////////////////////////
void thr_wrbyte_1 (uint8_t by)
{
	uint8_t i = 8;
	while (i--)
	{
		thr_wrbit_1( by & 1 );
		by >>= 1;
	}
}

void thr_wrbyte_2 (uint8_t by)
{
	uint8_t i = 8;
	while (i--)
	{
		thr_wrbit_2( by & 1 );
		by >>= 1;
	}
}
//////////////////////////////////////////////////////////////////////////
void ds18b20_read_1 (unsigned int *data)
{
	uint8_t temp[2];
	unsigned int dig;
	unsigned int dec;
	unsigned int sgn;
	thr_reset_1();
	thr_wrbyte_1(cmd_skiprom);
	thr_wrbyte_1(cmd_convtemp);
	while (!thr_rdbit_1());
	thr_reset_1();
	thr_wrbyte_1(cmd_skiprom);
	thr_wrbyte_1(cmd_rscratchpad);
	temp[0] = thr_rdbyte_1();
	temp[1] = thr_rdbyte_1();
	dig = temp[0] >> 4;
	dig |= (temp[1] & 0x07) << 4;
	dec = temp[0] & 0x0f;
	dec *= 625;
	sgn = temp[1] >> 7;
	data[0] = sgn;
	data[1] = dig;
	data[2] = dec;
}

void ds18b20_read_2 (unsigned int *data)
{
	uint8_t temp[2];
	unsigned int dig;
	unsigned int dec;
	unsigned int sgn;
	thr_reset_2();
	thr_wrbyte_2(cmd_skiprom);
	thr_wrbyte_2(cmd_convtemp);
	while (!thr_rdbit_2());
	thr_reset_2();
	thr_wrbyte_2(cmd_skiprom);
	thr_wrbyte_2(cmd_rscratchpad);
	temp[0] = thr_rdbyte_2();
	temp[1] = thr_rdbyte_2();
	dig = temp[0] >> 4;
	dig |= (temp[1] & 0x07) << 4;
	dec = temp[0] & 0x0f;
	dec *= 625;
	sgn = temp[1] >> 7;
	data[0] = sgn;
	data[1] = dig;
	data[2] = dec;
}
//////////////////////////////////////////////////////////////////////////

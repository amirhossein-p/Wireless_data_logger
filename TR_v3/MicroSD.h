/*
 * MicroSD.h
 * MicroSD Driver Module Library
 *
 * Created: 11/24/2015 3:55:05 PM
 *  Author: Amirhosein
 */ 


#define  sd_rset		(PORTC &= ~(1<<5))
#define  sd_set			(PORTC |= (1<<5))
#define  sd_err			(PINC & (1<<4))

char rcv[10];
int i = 0;
unsigned int rsp[5] =		{ 'r' , 's' , 'p' , 0x0D , 0x0A };	
unsigned int open[6] =		{ 'o' , 'p' , 'e' , 'n' , 0x0D , 0x0A };
unsigned int append[8] =	{ 'a' , 'p' , 'p' , 'e' , 'n' , 'd' , 0x0D , 0x0A };
unsigned int appendx[9] =	{ 'a' , 'p' , 'p' , 'e' , 'n' , 'd' , 'x' , 0x0D , 0x0A };	
unsigned int cd[4] =		{ 'c' , 'd' , 0x0D , 0x0A };	
unsigned int ckopn[13] =	{ 'c' , 'h' , 'e' , 'c' , 'k' , 'f' , 'n' , 'o' , 'p' , 'e' , 'n' , 0x0D , 0x0A };
unsigned int close[7] =		{ 'c' , 'l' , 'o' , 's' , 'e' , 0x0D , 0x0A };	
unsigned int end[5] =		{ 'e' , 'n' , 'd' , 0x0D , 0x0A };
unsigned int exist[9] =		{ 'f' , 'n' , 'e' , 'x' , 'i' , 's' , 't' , 0x0D , 0x0A };	
unsigned int new[5] =		{ 'n' , 'e' , 'w' , 0x0D , 0x0A };		
unsigned int start[7] =		{ 's' , 't' , 'a' , 'r' , 't' , 0x0D , 0x0A };	
unsigned int usrcv = 0;
unsigned int aa = 0;

ISR (USART_RXC_vect)
{
	usrcv = UDR;
	rcv[i++] = usrcv;
}

unsigned int sd_usrcv (void)
{
	return usrcv;
}

void sd_init(void)
{
	sd_set;
}
void sd_reset(void)
{
	if (sd_err == 1)
	{
		sd_rset;
		_delay_ms(100);
		sd_set;
	}
}
int sd_response (void)
{
	i = 0;
	int rspn = 0;
	for (int a=0; a<5; a++)
	{
		UDR = rsp[a];
		_delay_ms(1);
	}
	_delay_ms(10);
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '0')) rspn = 100;		// File System isn't compatible
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '1')) rspn = 101;		// Card is dismount
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '2')) rspn = 102;		// Card isn't present
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '3')) rspn = 103;		// Write Protected
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '4')) rspn = 104;		// Drive not initialized
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '5')) rspn = 105;		// Sector size isn't available
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '6')) rspn = 106;		// Sector count isn't available
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '7')) rspn = 107;		// File or directory not exist
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '8')) rspn = 108;		// File isn't open
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '9')) rspn = 109;		// File already is open
	if ((rcv[0] == '1') & (rcv[1] == '1') & (rcv[2] == '0')) rspn = 110;		// Invalid Filename & File extension or Directory Name
	if ((rcv[0] == '1') & (rcv[1] == '1') & (rcv[2] == '1')) rspn = 111;		// Capacity does not Initialize
	if ((rcv[0] == '1') & (rcv[1] == '1') & (rcv[2] == '2')) rspn = 112;		// Append Length >512 byte
	if ((rcv[0] == '1') & (rcv[1] == '1') & (rcv[2] == '3')) rspn = 113;		// Error in saving data
	if ((rcv[0] == '1') & (rcv[1] == '1') & (rcv[2] == '4')) rspn = 114;		// File Not Close
	if ((rcv[0] == '1') & (rcv[1] == '1') & (rcv[2] == '5')) rspn = 115;		// Error in get free size
	if ((rcv[0] == '1') & (rcv[1] == '1') & (rcv[2] == '6')) rspn = 116;		// Read Length >512 byte
	if ((rcv[0] == '1') & (rcv[1] == '1') & (rcv[2] == '7')) rspn = 117;		// Start point is out of file > file size
	if ((rcv[0] == '1') & (rcv[1] == '1') & (rcv[2] == '8')) rspn = 118;		// Body is out of EOF
	if ((rcv[0] == '1') & (rcv[1] == '1') & (rcv[2] == '9')) rspn = 119;		// Invalid replace length or >512 bytes
	if ((rcv[0] == '1') & (rcv[1] == '2') & (rcv[2] == '0')) rspn = 120;		// Dismount error
	if ((rcv[0] == '1') & (rcv[1] == '2') & (rcv[2] == '1')) rspn = 121;		// Long file name
	if ((rcv[0] == '1') & (rcv[1] == '2') & (rcv[2] == '2')) rspn = 122;		// File is read only
	if ((rcv[0] == '1') & (rcv[1] == '2') & (rcv[2] == '3')) rspn = 123;		// Directory isn't empty
	if ((rcv[0] == '1') & (rcv[1] == '2') & (rcv[2] == '4')) rspn = 124;		// Old & new file or directory name error
	if ((rcv[0] == '1') & (rcv[1] == '2') & (rcv[2] == '5')) rspn = 125;		// Old file or directory name error
	if ((rcv[0] == '1') & (rcv[1] == '2') & (rcv[2] == '6')) rspn = 126;		// New file or directory name error
	if ((rcv[0] == '1') & (rcv[1] == '2') & (rcv[2] == '7')) rspn = 127;		// File or directory already is exist
	if ((rcv[0] == '1') & (rcv[1] == '2') & (rcv[2] == '8')) rspn = 128;		// Path is not exist
	if ((rcv[0] == '1') & (rcv[1] == '2') & (rcv[2] == '9')) rspn = 129;		// File is not open
	if ((rcv[0] == '1') & (rcv[1] == '3') & (rcv[2] == '0')) rspn = 130;		// Set attribute error
	if ((rcv[0] == '1') & (rcv[1] == '3') & (rcv[2] == '1')) rspn = 131;		// Clear attribute error
	if ((rcv[0] == '1') & (rcv[1] == '3') & (rcv[2] == '2')) rspn = 132;		// Attribute parameter error
	if ((rcv[0] == '1') & (rcv[1] == '3') & (rcv[2] == '3')) rspn = 133;		// Time attribute is not set
	if ((rcv[0] == '1') & (rcv[1] == '3') & (rcv[2] == '4')) rspn = 134;		// Hour >23
	if ((rcv[0] == '1') & (rcv[1] == '3') & (rcv[2] == '5')) rspn = 135;		// Minute >59
	if ((rcv[0] == '1') & (rcv[1] == '3') & (rcv[2] == '6')) rspn = 136;		// Second >59
	if ((rcv[0] == '1') & (rcv[1] == '3') & (rcv[2] == '7')) rspn = 137;		// Month is not valid >12 or = 0
	if ((rcv[0] == '1') & (rcv[1] == '3') & (rcv[2] == '8')) rspn = 138;		// Day is not valid >31 or = 0
	if ((rcv[0] == '1') & (rcv[1] == '3') & (rcv[2] == '9')) rspn = 139;		// Dir error
	if ((rcv[0] == '1') & (rcv[1] == '4') & (rcv[2] == '0')) rspn = 140;		// Access denied
	if ((rcv[0] == '1') & (rcv[1] == '9') & (rcv[2] == '9')) rspn = 199;		// Wrong Command
	i = 0;
	return rspn;	
}
void sd_open (unsigned int len , char *fil)
{
	for (int a=0; a<6; a++)
	{
		UDR = open[a];
		_delay_ms(1);
	}
	for (int b=0; b<len; b++)
	{
		UDR = fil[b];
		_delay_ms(1);
	}
	UDR = 0x0D;
	_delay_ms(1);
	UDR = 0x0A;
	_delay_ms(1);
}
/*void sd_append (unsigned int len , char *dat)
{
	unsigned int le = 0;
	char lec[8];
	if (len < 10) le = 1;
	if (len > 9)  le = 2;
	if (len > 99) le = 3;	
	itoa(len , lec , 10);
	
	for (int a=0; a<8; a++)
	{
		UDR = append[a];
		_delay_ms(1);
	}
	for (int b=0; b<le; b++)
	{
		UDR = lec[b];
		_delay_ms(1);
	}
	UDR = 0x0D;
	_delay_ms(1);
	UDR = 0x0A;
	_delay_ms(1);
	for (int c=0; c<len; c++)
	{
		UDR = dat[c];
		_delay_ms(1);
	}
}*/


void sd_append (unsigned int len , char *dat)
{
	//unsigned int le = 0;
	char lec[2];
	//if (len < 10) le = 1;
	//if (len > 9)  le = 2;
	//if (len > 99) le = 3;
	itoa(len , lec , 10);
	
	for (int a=0; a<8; a++)
	{
		UDR = append[a];
		_delay_ms(1);
	}
	for (int b=0; b<2; b++)
	{
		UDR = lec[b];
		_delay_ms(1);
	}
	UDR = 0x0D;
	_delay_ms(1);
	UDR = 0x0A;
	_delay_ms(1);
	for (int c=0; c<len; c++)
	{
		UDR = dat[c];
		_delay_ms(1);
	}
	UDR = 0x0D;
	_delay_ms(1);
	UDR = 0x0A;
	_delay_ms(1);
}

void sd_appendx (unsigned int len , int tim , char *dat)
{
	char lec[2];
	char tic[1];
	itoa(len , lec , 10);
	itoa(tim , tic , 10);	
	for (int a=0; a<9; a++)
	{
		UDR = appendx[a];
		_delay_ms(1);
	}
	for (int b=0; b<2; b++)
	{
		UDR = lec[b];
		_delay_ms(1);
	}
	UDR = 0x0D;
	_delay_ms(1);
	UDR = 0x0A;
	_delay_ms(1);
	for (int c=0; c<1; c++)
	{
		UDR = tic[c];
		_delay_ms(1);
	}
	UDR = 0x0D;
	_delay_ms(1);
	UDR = 0x0A;
	_delay_ms(1);	
	for (int d=0; d<len; d++)
	{
		UDR = dat[d];
		_delay_ms(1);
	}
	UDR = 0x0D;
	_delay_ms(1);
	UDR = 0x0A;
	_delay_ms(1);
}




void sd_cd (unsigned int len , char *dir)
{
	for (int a=0; a<4; a++)
	{
		UDR = cd[a];
		_delay_ms(1);
	}
	for (int b=0; b<len; b++)
	{
		UDR = dir[b];
		_delay_ms(1);
	}
	UDR = 0x0D;
	_delay_ms(1);
	UDR = 0x0A;
	_delay_ms(1);
}
int sd_checkfnopen (unsigned int len , char *dir)
{
	i = 0;
	int rspn = 0;
	for (int a=0; a<13; a++)
	{
		UDR = ckopn[a];
		_delay_ms(1);
	}
	for (int b=0; b<len; b++)
	{
		UDR = dir[b];
		_delay_ms(1);
	}
	UDR = 0x0D;
	_delay_ms(1);
	UDR = 0x0A;
	_delay_ms(10);
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '8')) rspn = 108;		// File isn't open
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '9')) rspn = 109;		// File already is open
	i = 0;
	return rspn;
}	
void sd_close (void)
{
	for (int a=0; a<7; a++)
	{
		UDR = close[a];
		_delay_ms(1);
	}
}
void sd_end (void)
{
	for (int a=0; a<5; a++)
	{
		UDR = close[a];
		_delay_ms(1);
	}
}
int sd_fnexist (unsigned int len , char *dir)
{
	i = 0;
	int rspn = 0;
	for (int a=0; a<9; a++)
	{
		UDR = exist[a];
		_delay_ms(1);
	}
	for (int b=0; b<len; b++)
	{
		UDR = dir[b];
		_delay_ms(1);
	}
	UDR = 0x0D;
	_delay_ms(1);
	UDR = 0x0A;
	_delay_ms(200);
	if ((rcv[0] == '1') & (rcv[1] == '0') & (rcv[2] == '7')) rspn = 107;		// File or directory not exist
	if ((rcv[0] == '1') & (rcv[1] == '2') & (rcv[2] == '7')) rspn = 127;		// File or directory already is exist	
	i = 0;
	return rspn;
}
void sd_new (unsigned int len , char *fil)
{
	for (int a=0; a<5; a++)
	{
		UDR = new[a];
		_delay_ms(1);
	}
	for (int b=0; b<len; b++)
	{
		UDR = fil[b];
		_delay_ms(1);
	}
	UDR = 0x0D;
	_delay_ms(1);
	UDR = 0x0A;
	_delay_ms(1);
}
void sd_start (void)
{	
	for (int a=0; a<7; a++)
	{
		UDR = start[a];
		_delay_ms(1);
	}
}
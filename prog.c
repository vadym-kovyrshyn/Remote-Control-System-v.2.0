//#include <stdio.h>
#include <stdlib.h>
//#include <p18F2620.h>


// CONFIG1H
#pragma config OSC = HSPLL      // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = ON        // Internal/External Oscillator Switchover bit (Oscillator Switchover mode enabled)

// CONFIG2L
#pragma config PWRT = ON        // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config LPT1OSC = ON     // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for low-power operation)
#pragma config MCLRE = OFF      // MCLR Pin Enable bit (RE3 input pin enabled; MCLR disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-003FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (004000-007FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (008000-00BFFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (00C000-00FFFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-003FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (004000-007FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (008000-00BFFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (00C000-00FFFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (004000-007FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (008000-00BFFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (00C000-00FFFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0007FFh) not protected from table reads executed in other blocks)

volatile unsigned char digits[2][16];

typedef struct {
	unsigned blink: 1;
	unsigned modifided: 1;
} digits_atributes;
volatile digits_atributes digitsAtrib[2][16];
long int Clock;
long int Minutes = 0;
unsigned int LCD_ON_TIMEOUT = 60000; // 3 min
volatile unsigned char KeyCode;
unsigned int CorrectTimeConst;
unsigned int AdressOfNextStartCell = 240;
unsigned long int NearTimeStart;
unsigned long int NearTimeStop;
unsigned int CurrentReadingCell = 0;
unsigned char CurrentSignalsData = 0;
volatile unsigned char SignalsOut = 0;
volatile unsigned char SignalsForInd = 0;

struct {
	unsigned DetailModeOfViewSheduler: 1;
	unsigned RelevanceOfNextStartCell: 1;
	unsigned ModeOfFirstLine: 1;
	unsigned TimeIsRead: 1;
	unsigned isTimeSetting: 1;
	unsigned LCD_Light_On: 1;
	unsigned LCD_Power_On: 1;
	unsigned LockSignals: 1;
	unsigned IsLCDModified: 1;
	unsigned GlobalBlink: 1;
} flags;

unsigned char cMinutes = 0;
unsigned char cHours = 0;
unsigned char cWeekDay = 1;
unsigned char cDays = 1;
unsigned char cMonths = 1;
unsigned char cYears = 0x01; //BCD

unsigned char const cMinutesAdress = 240;
unsigned char const cHoursAdress = 241;
unsigned char const cWeekDayAdress = 242;
unsigned char const cDaysAdress = 243;
unsigned char const cMonthsAdress = 244;
unsigned char const cYearsAdress = 245;

unsigned char CurrentSignals = 0;
unsigned char GlobalBlinkCycleTime = 70;

typedef struct {
	unsigned char fio[16];
	unsigned char num [12];
} tel;

//	1 - 9	����	����
//			abc		def
//
//	����4	����	����
//	ghi		jkl		mno
//	
//	����	����	����
//	pqrs	tuv		wxyz


#define _XTAL_FREQ 39400000

#define E					RC0
#define RS					RC2
//char LCD_Power_On_PIN = 0;
//#define LCD_Power_On_PIN	RC2
#define EMPTY_SYMBOL_VALUE ' '
#define UNDERSCORE_SYMBOL_VALUE '_' //_
#define EMPTY_STRING_16 "                "

void main2(void);
void lcd_init(void);
void lcd_on(void);
void lcd_off(void);
void SendArrayToLCD(unsigned char A[], char B, char C);
void ParseTime(long int TimeValue, unsigned int *Time, unsigned char *Day);
void ParseDataRecord(unsigned long int Data, unsigned int *TimeStart, unsigned int *TimeStop, unsigned char *Days, unsigned char *Signals);
void EEWR(unsigned int adress, unsigned char data);
void Scheduler(unsigned int StartFrom);
void SchedulerNew(unsigned int StartFrom);
void SignalsOnOff(void);
void FillMinutes(void);
unsigned char EERD(unsigned int adress);
unsigned long int ReadFourBytesEE(unsigned int adress);
unsigned char *GetDayOfWeek(unsigned char day);
unsigned char EditSchedule(unsigned int adress, unsigned int SourceOfRecord);
unsigned int FindNextTimeStart(unsigned long int *TimeFrom);
void I2CInit(void);
void I2CStart(void);
void I2CStop(void);
void I2CRestart(void);
void I2CAck(void);
void I2CNak(void);
void I2CWait(void);
void I2CSend(unsigned char dat);
unsigned char I2CRead(void);
void ReadTime();
void WriteTime(unsigned char Minutes, unsigned char Hours, unsigned char DayOfWeek);
unsigned char BCD_to_DEC(unsigned char BCD);
unsigned char DEC_to_BCD(unsigned char DEC);



unsigned char getDigit(char line, char symbol) {
	return digits[line][symbol];
}

void setDigit(char line, char symbol, unsigned char data) {
	unsigned char value = data;
	unsigned char value0 = digits[line][symbol];
	digitsAtrib[line][symbol].modifided = digitsAtrib[line][symbol].modifided > 0 || (value != value0);
	flags.IsLCDModified = flags.IsLCDModified || (value != value0);
	digits[line][symbol] = value;
}

void setBlink(char line, char symbol, unsigned char value) {
	if (value != 0) {
		digitsAtrib[line][symbol].blink = 1;
	} else {
		digitsAtrib[line][symbol].blink = 0;
	}
	digitsAtrib[line][symbol].modifided = 1;
	flags.IsLCDModified = 1;
}

void clrInd() {
	for (char line = 0; line < 2; line++) {
		for (char symb = 0; symb < 16; symb++) {
			digits[line][symb] = ' ';
			digitsAtrib[line][symb].blink = 0;
			digitsAtrib[line][symb].modifided = 1;
		}
	}
	flags.IsLCDModified = 1;
}

unsigned char getLcdCodeOfChar(unsigned char dig) {
	switch (dig) {
	case ' ': return 0x20;
	case '0': return 0x30;
	case '1': return 0x31;
	case '2': return 0x32;
	case '3': return 0x33;
	case '4': return 0x34;
	case '5': return 0x35;
	case '6': return 0x36;
	case '7': return 0x37;
	case '8': return 0x38;
	case '9': return 0x39;
	case '-': return 0x2D;
	case '�': return 0x2D;
	case '!': return 0x21;
	case '"': return 0x22;
	case '#': return 0x23;
	case '$': return 0x24;
	case '%': return 0x25;
	case '&': return 0x26;
	case '(': return 0x28;
	case ')': return 0x29;
	case '*': return 0x2A;
	case ',': return 0x2C;
	case '.': return 0x2E;
	case '/': return 0x2F;
	case ':': return 0x3A;
	case ';': return 0x3B;
	case '?': return 0x3F;
	case '@': return 0x40;
	case '[': return 0x5B;
	case ']': return 0x5D;
	case '^': return 0x5E;
	case '_': return 0x5F;
	case '`': return 0x60;
	case '�'	: return 0x27;
	case '+': return 0x2B;
	case '<': return 0x3C;
	case '=': return 0x3D;
	case '>': return 0x3E;
	case '�': return 0xEF;
	case '�': return 0xC8;
	case '�': return 0xC9;
	case '\\':return 0x2F;
	case '~': return 0xE9;
	case 'A': return 0x41;
	case 'a': return 0x61;
	case 'B': return 0x42;
	case 'b': return 0x62;
	case 'C': return 0x43;
	case 'c': return 0x63;
	case 'D': return 0x44;
	case 'd': return 0x64;
	case 'E': return 0x45;
	case 'e': return 0x65;
	case 'F': return 0x46;
	case 'f': return 0x66;
	case 'G': return 0x47;
	case 'g': return 0x67;
	case 'H': return 0x48;
	case 'h': return 0x68;
	case 'I': return 0x49;
	case 'i': return 0x69;
	case 'J': return 0x4A;
	case 'j': return 0x6A;
	case 'K': return 0x4B;
	case 'k': return 0x6B;
	case 'L': return 0x4C;
	case 'l': return 0x6C;
	case 'M': return 0x4D;
	case 'm': return 0x6D;
	case 'N': return 0x4E;
	case 'n': return 0x6E;
	case '�': return 0xCC;
	case 'O': return 0x4F;
	case 'o': return 0x6F;
	case 'P': return 0x50;
	case 'p': return 0x70;
	case 'Q': return 0x51;
	case 'q': return 0x71;
	case 'R': return 0x52;
	case 'r': return 0x72;
	case 'S': return 0x53;
	case 's': return 0x73;
	case 'T': return 0x54;
	case 't': return 0x74;
	case 'U': return 0x55;
	case 'u': return 0x75;
	case 'V': return 0x56;
	case 'v': return 0x76;
	case 'W': return 0x57;
	case 'w': return 0x77;
	case 'X': return 0x58;
	case 'x': return 0x78;
	case 'Y': return 0x59;
	case 'y': return 0x79;
	case 'Z': return 0x5A;
	case 'z': return 0x7A;
	case '�': return 0x41;
	case '�': return 0x61;
	case '�': return 0xA0;
	case '�': return 0xB2;
	case '�': return 0x42;
	case '�': return 0xB3;
	case '�': return 0xA1;
	case '�': return 0xB4;
	case '�': return 0xE0;
	case '�': return 0xE3;
	case '�': return 0x45;
	case '�': return 0x65;
	case '�': return 0xA2;
	case '�': return 0xB5;
	case '�': return 0xA3;
	case '�': return 0xB6;
	case '�': return 0xA4;
	case '�': return 0xB7;
	case '�': return 0xA5;
	case '�': return 0xB8;
	case '�': return 0xA6;
	case '�': return 0xB9;
	case '�': return 0x4B;
	case '�': return 0xBA;
	case '�': return 0xA7;
	case '�': return 0xBB;
	case '�': return 0xBC;
	case '�': return 0x4D;
	case '�': return 0x48;
	case '�': return 0xBD;
	case '�': return 0x4F;
	case '�': return 0x6F;
	case '�': return 0xA8;
	case '�': return 0xBE;
	case '�': return 0x50;
	case '�': return 0x70;
	case '�': return 0x43;
	case '�': return 0x63;
	case '�': return 0xBF;
	case '�': return 0x54;
	case '�': return 0xA9;
	case '�': return 0x79;
	case '�': return 0xAA;
	case '�': return 0xE4;
	case '�': return 0x58;
	case '�': return 0x78;
	case '�': return 0xE1;
	case '�': return 0xE5;
	case '�': return 0xAB;
	case '�': return 0xC0;
	case '�': return 0xAC;
	case '�': return 0xC1;
	case '�': return 0xE2;
	case '�': return 0xE6;
	case '�': return 0xAD;
	case '�': return 0xC2;
	case '�': return 0xAE;
	case '�': return 0xC3;
	case '�': return 0xC4;
	case '�': return 0xC4;
	case '�': return 0xAF;
	case '�': return 0xC5;
	case '�': return 0xB0;
	case '�': return 0xC6;
	case '�': return 0xB1;
	case '�': return 0xC7;

	default: return 0x3F; // '?'
	}
}

unsigned char entNum(char borderDown, char borderUp) { //0-9		
	if (KeyCode >= 30 && KeyCode <= 39) {
		char KeyValue = KeyCode - 30;
		if (borderDown <= KeyValue && borderUp >= KeyValue) {
			KeyCode = 0;
			return (unsigned char) KeyValue;
		} else return 255;
	} else return 255;
}

void NumericToIndicator(unsigned long int n, unsigned char displacement) {
	clrInd();
	displacement++;
	do {
		setDigit(0, displacement, n % 10);
		n /= 10;
		displacement++;
	} while ((displacement < 10)&(n > 0));
}

unsigned char *GetDayOfWeek(unsigned char day) {
	static unsigned char array[3] = {0, 0, 255};
	if (day == 1) {
		array[1] = 57;
		array[0] = 55;
		//	array = "��";
	}// ��
	else if (day == 2) {
		array[1] = 43;
		array[0] = 60;
	}// ��
	else if (day == 3) {
		array[1] = 59;
		array[0] = 58;
	}// ��
	else if (day == 4) {
		array[1] = 65;
		array[0] = 60;
	}// ��
	else if (day == 5) {
		array[1] = 57;
		array[0] = 60;
	}// ��
	else if (day == 6) {
		array[1] = 59;
		array[0] = 42;
	}// ��
	else if (day == 7) {
		array[1] = 43;
		array[0] = 59;
	}// ��
	else {
		array[1] = 34;
		array[0] = 34;
	}
	return array;
}

void TimeToInd() {
	if (KeyCode == 38 && (AdressOfNextStartCell != 240 || flags.DetailModeOfViewSheduler)) {
		KeyCode = 0;
		clrInd();
		flags.DetailModeOfViewSheduler = !flags.DetailModeOfViewSheduler;
	}
	if (KeyCode == 45 && AdressOfNextStartCell != 240 && flags.DetailModeOfViewSheduler) {
		KeyCode = 0;
		Scheduler(AdressOfNextStartCell);
	}
	if (!flags.RelevanceOfNextStartCell) {
		NearTimeStart = Clock;
		AdressOfNextStartCell = FindNextTimeStart(&NearTimeStart);
		if (flags.DetailModeOfViewSheduler) {
			flags.DetailModeOfViewSheduler = 0;
			clrInd();
		}
		flags.RelevanceOfNextStartCell = 1;
	}
	if (AdressOfNextStartCell == 240 || !flags.DetailModeOfViewSheduler) {
		char line = 0;
		char symbol = 3;
		unsigned char SignalsFinal = CurrentSignals | SignalsForInd;
		if (flags.ModeOfFirstLine != (SignalsFinal > 0)) {
			clrInd();
			flags.ModeOfFirstLine = SignalsFinal > 0;
		}
		if (SignalsFinal > 0) {
			symbol = 4;

			unsigned char D[4] = {34, 34, 34, 255};
			for (unsigned char i = 1; i < 4; i++) {
				unsigned char SignalOn = SignalsFinal % 2;
				SignalsFinal /= 2;
				if (SignalOn == 1) {
					D[2] = D[1];
					D[1] = D[0];
					D[0] = getNumChar(i);
				}
			}
			outputString(D, line, 13);
		}

		setDigit(line, symbol + 3, 35);
		setDigit(line, symbol + 6, 35);
		long int temp = Clock / 100;
		setDigit(line, symbol + 1, getNumChar(temp % 10));
		temp /= 10;
		setDigit(line, symbol + 2, getNumChar(temp % 6));
		temp /= 6;
		setDigit(line, symbol + 4, getNumChar(temp % 10));
		temp /= 10;
		setDigit(line, symbol + 5, getNumChar(temp % 6));
		temp /= 6;
		unsigned char day = temp / 24 + 1;
		temp %= 24;
		setDigit(line, symbol + 7, getNumChar(temp % 10));
		setDigit(line, symbol + 8, getNumChar(temp / 10));

		SendArrayToLCD(GetDayOfWeek(day), line, symbol + 10);
	}

	unsigned char DataArray[17] = EMPTY_STRING_16;

	if (flags.LockSignals && !flags.DetailModeOfViewSheduler) {
		outputString("���������� ����.", 1, 0);
		return;
	}

	if (AdressOfNextStartCell != 240) {

		DataArray[15] = 90;

		char line = 1;
		unsigned int TimeStart;
		unsigned int TimeStop;
		unsigned char Days;
		unsigned char Signals;
		ParseDataRecord(ReadFourBytesEE(AdressOfNextStartCell), &TimeStart, &TimeStop, &Days, &Signals);

		unsigned int Time;
		unsigned char Day;
		ParseTime(NearTimeStart, &Time, &Day);

		char nSymb = 0;
		if (flags.DetailModeOfViewSheduler) {
			line = 0;
			nSymb = 2;
			DataArray[14] = 34;
			DataArray[10] = 59; // c
		}
		DataArray[8 - nSymb] = 35; //:
		DataArray[6 - nSymb] = Time % 10;
		Time /= 10;
		DataArray[7 - nSymb] = Time % 6;
		Time /= 6;
		DataArray[9 - nSymb] = Time % 10;
		Time /= 10;
		DataArray[10 - nSymb] = Time % 10;

		for (unsigned char i = 1; i < 4; i++) {
			unsigned char SignalOn = Signals % 2;
			Signals /= 2;
			if (SignalOn == 1) {
				char j = nSymb / 2;
				DataArray[3 - j] = DataArray[2 - j];
				DataArray[2 - j] = DataArray[1 - j];
				DataArray[1 - j] = i;
			}
		}
		unsigned char *DataArray2 = GetDayOfWeek(Day);
		DataArray[12] = DataArray2[0];
		DataArray[13] = DataArray2[1];

		SendArrayToLCD(DataArray, line, 1);

		if (flags.DetailModeOfViewSheduler) {
			unsigned char DataArrayOfLine2[13] = {34, 34, 35, 34, 34, 34, 56, 57, 34, 34, 34, 76, 255};
			DataArrayOfLine2[0] = TimeStop % 10;
			TimeStop /= 10;
			DataArrayOfLine2[1] = TimeStop % 6;
			TimeStop /= 6;
			DataArrayOfLine2[3] = TimeStop % 10;
			TimeStop /= 10;
			DataArrayOfLine2[4] = TimeStop % 10;

			unsigned char CellsNumber = AdressOfNextStartCell / 4 + 1;
			do {
				char d = CellsNumber % 10;
				DataArrayOfLine2[9] = DataArrayOfLine2[10];
				DataArrayOfLine2[10] = d;
				CellsNumber /= 10;
			} while (CellsNumber > 0);

			SendArrayToLCD(DataArrayOfLine2, 1, 5);
		}
	} else {
		if (flags.DetailModeOfViewSheduler) {
			clrInd();
			flags.DetailModeOfViewSheduler = 0;
		}
		SendArrayToLCD(DataArray, 1, 1);
	}
}

void EEWR(unsigned int adress, unsigned char data) {
	volatile unsigned char INTCON_BUP = INTCON;
	INTCONbits.GIEH = 0;
	INTCONbits.GIEL = 0;
	ClrWdt();
	EEADRH = adress >> 8;
	EEADR = adress % 255;
	EEDATA = data;
	EECON1bits.EEPGD = 0;
	EECON1bits.CFGS = 0;
	EECON1bits.WREN = 1;
	EECON2 = 0x55;
	EECON2 = 0xAA;
	EECON1bits.WR = 1;
	EECON1bits.WREN = 0;
	while (EECON1bits.WR == 1) {
	}
	PIR2bits.EEIF = 0;
	INTCON = INTCON_BUP;
}

unsigned char EERD(unsigned int adress) {
	volatile unsigned char INTCON_BUP = INTCON;
	volatile unsigned char EEDATA_BUP;
	INTCONbits.GIEH = 0;
	INTCONbits.GIEL = 0;
	ClrWdt();
	EEADRH = adress >> 8;
	EEADR = adress % 255;
	EECON1bits.EEPGD = 0;
	EECON1bits.CFGS = 0;
	EECON1bits.RD = 1;
	EEDATA_BUP = EEDATA;
	INTCON = INTCON_BUP;
	return EEDATA_BUP;
}

void ReIndTimeEdit(long int n) {
	long int temp = n;
	setDigit(0, 2, temp % 10);
	temp /= 10;
	setDigit(0, 3, temp % 6);
	temp /= 6;
	temp %= 24;
	setDigit(0, 4, 35);
	setDigit(0, 5, temp % 10);
	setDigit(0, 6, temp / 10);
	setDigit(0, 8, 1 + n / 1440);


	setDigit(1, 1, cYears % 16);
	setDigit(1, 2, cYears / 16);
	setDigit(1, 3, 0);
	setDigit(1, 4, 2);
	setDigit(1, 5, 86);
	setDigit(1, 6, cMonths % 16);
	setDigit(1, 7, cMonths / 16);
	setDigit(1, 8, 86);
	setDigit(1, 9, cDays % 16);
	setDigit(1, 10, cDays / 16);
}

void TimeEdit() {
	long int t;
	unsigned char c;
	flags.isTimeSetting = 1;
	clrInd();
	t = Clock / 6000;
	setBlink(0, 6, 1);
	char j;
	c = 2;
	unsigned char borders[] = {0, 2};

	while (1) {
		j = entNum(borders[0], borders[1]);
		if (j != 255 && c == 1) {
			t = t + (j - getDigit(0, 8))*1440;
			borders[0] = 0;
			borders[1] = 2;
			c++;
			setBlink(0, 8, 0);
			setBlink(0, 6, 1);
		} else if (j != 255 && c == 2) {
			t = t + (j - getDigit(0, 6))*600;
			if (j > 1) {
				borders[0] = 0;
				borders[1] = 3;
				if (getDigit(0, 5) > 3) {
					t = t + (3 - getDigit(0, 5))*60;
				}
			} else {
				borders[0] = 0;
				borders[1] = 9;
			}
			c++;
			setBlink(0, 6, 0);
			setBlink(0, 5, 1);
		} else if (j != 255 && c == 3) {
			t = t + (j - getDigit(0, 5))*60;
			borders[0] = 0;
			borders[1] = 5;
			c++;
			setBlink(0, 5, 0);
			setBlink(0, 3, 1);
		} else if (j != 255 && c == 4) {
			t = t + (j - getDigit(0, 3))*10;
			borders[0] = 0;
			borders[1] = 9;
			c++;
			setBlink(0, 3, 0);
			setBlink(0, 2, 1);
		} else if (j != 255 & c == 5) {
			t = t + j - getDigit(0, 2);
			borders[0] = 0;
			borders[1] = 2;
			c++;
			setBlink(0, 2, 0);
			setBlink(1, 10, 1);
		} else if (j != 255 && c == 6) {
			cDays = (j * 16) + getDigit(1, 9);
			if (j == 0) {
				borders[0] = 1;
				borders[1] = 9;
				if (getDigit(1, 9) == 0) setDigit(1, 9, 1);
			} else if (j < 3) {
				borders[0] = 0;
				borders[1] = 9;
			} else if (j == 3) {
				borders[0] = 0;
				borders[1] = 1;
			}
			c++;
			setBlink(1, 10, 0);
			setBlink(1, 9, 1);
		} else if (j != 255 && c == 7) {
			cDays = (getDigit(1, 10)*16) + j;
			borders[0] = 0;
			borders[1] = 1;
			c++;
			setBlink(1, 9, 0);
			setBlink(1, 7, 1);
		} else if (j != 255 && c == 8) {
			cMonths = (j * 16) + getDigit(1, 6);
			if (j == 0) {
				borders[0] = 1;
				borders[1] = 9;
				if (getDigit(1, 7) == 0) setDigit(1, 6, 1);
			} else if (j == 1) {
				borders[0] = 0;
				borders[1] = 2;
			}
			c++;
			setBlink(1, 7, 0);
			setBlink(1, 6, 1);
		} else if (j != 255 && c == 9) {
			cMonths = (getDigit(1, 7)*16) + j;
			borders[0] = 0;
			borders[1] = 9;
			c = 10;
			setBlink(1, 6, 0);
			setBlink(1, 2, 1);
		} else if (j != 255 && c == 10) {
			cYears = (j * 16) + getDigit(1, 1);
			borders[0] = 0;
			borders[1] = 9;
			c++;
			setBlink(1, 2, 0);
			setBlink(1, 1, 1);
		} else if (j != 255 && c == 11) {
			cYears = (getDigit(1, 2)*16) + j;
			borders[0] = 0;
			borders[1] = 2;
			c = 2;
			setBlink(1, 1, 0);
			setBlink(0, 6, 1);
		}

		if (KeyCode == 44) {
			KeyCode = 0;
			clrInd();
			if (c == 1) {
				c = 2;
				borders[0] = 0;
				borders[1] = 2;
				setBlink(0, 8, 0);
				setBlink(0, 6, 1);
			} else if (c > 1 && c < 6) {
				c = 6;
				borders[0] = 0;
				borders[1] = 3;
				setBlink(1, 10, 1);
			} else if (c > 5) {
				c = 1;
				borders[0] = 1;
				borders[1] = 7;
				setBlink(0, 8, 1);
			}
		} else if (KeyCode == 43) {
			KeyCode = 0;
			clrInd();
			return;
		} else if (KeyCode == 42) {
			KeyCode = 0;
			WriteTime(getDigit(0, 3) * 16 + getDigit(0, 2), getDigit(0, 6) * 16 + getDigit(0, 5), getDigit(0, 8));
			clrInd();
			return;
		}
		ReIndTimeEdit(t);
	}
}

unsigned long int ReadFourBytesEE(unsigned int adress) {
	unsigned long int x;
	for (int i = 3; i >= 0; i--) {
		x = x * 256 + EERD(adress + i);
	}
	return x;
}

void WriteFourBytesEE(unsigned int adress, unsigned long int data) {
	for (char i = 0; i < 4; i++) {
		EEWR(adress + i, data % 256);
		data /= 256;
	}
}

void delay(unsigned char del, unsigned int count) {
	while (count > 0) {
		TMR2 = 0;
		PR2 = del;
		TMR2IF = 0;
		while (!TMR2IF);
		count--;
	};
	return;
}

void SendSymbolToLCD(unsigned char Symb) {
	RS = 1;
	PORTA = (0b11110000 & Symb) / 8 | (flags.LCD_Light_On == 1 ? 0b00100000 : 0b00000000);
	E = 1;
	delay(10, 1);
	E = 0;
	RS = 0;
	delay(25, 5);

	RS = 1;
	PORTA = (0b11110000 & (Symb * 16)) / 8 | (flags.LCD_Light_On == 1 ? 0b00100000 : 0b00000000);
	E = 1;
	delay(10, 1);
	E = 0;
	RS = 0;
	delay(25, 5);
}

void SendArrayToLCD(unsigned char *Symb, char line, char position) {
	unsigned int i = 0;
	while (Symb[i] != 255 && position < 17) {
		setDigit(line, position, Symb[i]);
		i++;
		position++;
	}
}

void lcd_send_initial_half_byte(unsigned char data) {
	RS = 0;
	PORTA = (0b00011110 & (data * 2)) | (flags.LCD_Light_On == 1 ? 0b00100000 : 0b00000000);
	E = 1;
	E = 0;
}

void lcd_send_byte(unsigned char data) {
	lcd_send_initial_half_byte((data & 0b11110000) / 16);
	__delay_us(40);
	lcd_send_initial_half_byte(data & 0b00001111);
	__delay_us(40);
}

void lcd_off() {
	flags.LCD_Light_On = 0;
	lcd_send_byte(0b00001000);
	flags.LCD_Power_On = 0;
	//LCD_Power_On_PIN = 0;
}

void lcd_on() {
	lcd_init();
	lcd_send_byte(0b00001100);
	flags.LCD_Power_On = 1;
	//LCD_Power_On_PIN = 1;
}

void lcd_init() {
	unsigned char INTCON_BUF = INTCON;
	INTCONbits.GIEH = 0;
	INTCONbits.GIEL = 0;
	E = 0;
	RS = 0;
	// http://cxem.net/mcmc89.php
	__delay_ms(20);
	lcd_send_initial_half_byte(0b00000011);
	__delay_ms(5);
	lcd_send_initial_half_byte(0b00000011);
	__delay_us(110);
	lcd_send_initial_half_byte(0b00000011);
	__delay_us(40);
	lcd_send_initial_half_byte(0b00000010);
	__delay_us(40);
	lcd_send_byte(0b00101100);
	__delay_us(40);
	lcd_send_byte(0b00001000);
	__delay_us(40);
	lcd_send_byte(0b00000001);
	__delay_us(40);
	lcd_send_byte(0b00010010);
	__delay_us(40);

	//lcd_clear
	E = 0;
	RS = 0;
	//RW = 0;
	lcd_send_byte(0b00000001);
	__delay_ms(2);

	INTCON = (INTCON_BUF | 0b10000000) | (INTCON & 0b01111111);
}

unsigned char TestKeyGroup() {
	if (PORTBbits.RB6 == 0) {
		return 1;
	} else if (PORTBbits.RB4 == 0) {
		return 2;
	} else if (PORTBbits.RB2 == 0) {
		return 3;
	} else if (PORTBbits.RB0 == 0) {
		return 4;
	} else {
		return 17;
	}
}

unsigned char ConvKeyNum(unsigned char Num) {
	switch (Num) {
	case 0: return 33;
	case 1: return 43;
	case 2: return 42;
	case 3: return 41;
	case 4: return 40;
	case 5: return 45;
	case 6: return 39;
	case 7: return 36;
	case 8: return 37;
	case 9: return 38;
	case 10: return 35;
	case 11: return 34;
	case 12: return 32;
	case 13: return 30;
	case 14: return 44;
	case 15: return 31;
	default: return 0;
	}
}

unsigned char FindCell(unsigned int adressStart, char New, unsigned char previous) {
	if (adressStart == 240)adressStart = 236;
	unsigned char adress = adressStart;
	unsigned char adressNew = 240;
	do {
		if (!previous) {
			adress == 236 ? adress = 0 : adress += 4;
		} else {
			adress == 0 ? adress = 236 : adress -= 4;
		}
		unsigned char D = EERD(adress + 2);
		if (New != 1 ? D != 0xFF : D == 0xFF) {
			adressNew = adress;
			break;
		}
	} while (adress != adressStart);
	return adressNew;
}

unsigned char RefreshSchedulerIndicator(unsigned int adress, char New, unsigned int CopyFrom) {
	if (adress != 240 && (New != 1 ? EERD(adress + 2) != 0xFF : EERD(adress + 2) == 0xFF)) {
		NumericToIndicator(adress / 4 + 1, 1);
	} else {
		adress = FindCell(adress, New, 0);
		if (adress != 240) {
			NumericToIndicator(adress / 4 + 1, 1);
		} else {
			setDigit(0, 3, 32);
			setDigit(0, 2, 32);
		}
	}
	if (New == 1) {
		if (CopyFrom != 240) {
			setDigit(0, 15, 26); //t
			setDigit(0, 14, 22); //o

			setDigit(1, 15, 12); //c
			setDigit(1, 14, 22); //o
			setDigit(1, 13, 23); //p
			setDigit(1, 12, 29); //y
			setDigit(1, 11, 34); // 
			setDigit(1, 10, 15); //f
			setDigit(1, 9, 24); //r
			setDigit(1, 8, 22); //o
			setDigit(1, 7, 37); //m

			setDigit(1, 2, 91); //^
			unsigned char NumberFrom = CopyFrom / 4 + 1;
			if (NumberFrom > 9) {
				setDigit(1, 5, NumberFrom / 10);
			}
			setDigit(1, 4, NumberFrom % 10);
		}
		setDigit(0, 12, 21);
		setDigit(0, 11, 14);
		setDigit(0, 10, 36);
	}
	setDigit(0, 8, 12);
	setDigit(0, 7, 14);
	setDigit(0, 6, 20);
	setDigit(0, 5, 20);
	return adress;
}

void TimesToIndicator(unsigned int Times[]) {
	unsigned int temp;
	for (char j = 0; j < 7; j += 6) {
		temp = Times[j / 5];
		setDigit(0, 1 + j, temp % 10);
		temp /= 10;
		setDigit(0, 2 + j, temp % 6);
		setDigit(0, 3 + j, 35);
		temp = temp / 6;
		setDigit(0, 4 + j, temp % 10);
		setDigit(0, 5 + j, temp / 10);
	}
	setDigit(0, 6, 32);
}

void ItemsToIndicator(unsigned char items, unsigned char max_i, unsigned char firstDigit) {
	for (char i = 1; i < max_i; i++) {
		items % 2 == 1 ? setDigit(0, firstDigit - i, i) : setDigit(0, firstDigit - i, 32);
		items /= 2;
	}
	//	ItemsToIndicator(Days, 8, 9)
	//	ItemsToIndicator(Signals, 4, 8)
}

unsigned char getBorderUp(char n) {
	unsigned char BorderUp;
	switch (n) {
	case 11: case 5: BorderUp = 2;
		break;
	case 10: case 4: getDigit(0, n + 1) == 2 ? BorderUp = 3: BorderUp = 9;
		break;
	case 8: case 2: BorderUp = 5;
		break;
	case 7: case 1: BorderUp = 9;
		break;
	}
	return BorderUp;
}

unsigned char ConvertDayToBit(unsigned char DayNumber) {
	switch (DayNumber) {
	case 1: return 0b00000001;
	case 2: return 0b00000010;
	case 3: return 0b00000100;
	case 4: return 0b00001000;
	case 5: return 0b00010000;
	case 6: return 0b00100000;
	case 7: return 0b01000000;
	default: return 0;
	}
}

unsigned char EditSchedule(unsigned int adress, unsigned int SourceOfRecord) {
	unsigned char TargetAdress;
	if (SourceOfRecord == 240) {
		TargetAdress = adress;
	} else {
		TargetAdress = SourceOfRecord;
	}
	unsigned long int Data = ReadFourBytesEE(TargetAdress);

	char BeginEditTimes = 0;
	if (Data == 0xFFFFFFFF) {
		Data = 0;
		BeginEditTimes = 1;
	}

	unsigned int TimeStart;
	unsigned int TimeStop;
	unsigned char Days;
	unsigned char Signals;
	ParseDataRecord(Data, &TimeStart, &TimeStop, &Days, &Signals);

	unsigned int Times[2];
	Times[1] = TimeStart;
	Times[0] = TimeStop;

	clrInd();
	char mode = 1;
	unsigned char key;
	while (1) {
		if (KeyCode == 43) { // Cancel
			KeyCode = 0;
			clrInd();
			return 0;
		} else if (KeyCode == 42) { // Enter
			KeyCode = 0;
			Data = (unsigned long int) Signals * 0x20000000 + (unsigned long int) Days * 0x400000 + (unsigned long int) Times[0]*0x800 + (unsigned long int) Times[1];
			WriteFourBytesEE(adress, Data);
			clrInd();
			return 1;
		}
		switch (mode) {
		case 1:
		{
			TimesToIndicator(Times);
			if (KeyCode == 45 || BeginEditTimes == 1) {
				KeyCode = 0;
				BeginEditTimes = 0;
				setBlink(0, 11, 1);
				char n = 11;
				unsigned char BorderUp = 2;
				while (1) {
					if (KeyCode == 43) { // Cancel
						KeyCode = 0;
						clrInd();
						break;
					} else if (KeyCode == 42) { // Enter
						KeyCode = 0;
						unsigned int T[2];
						for (char j = 0; j < 7; j += 6) {
							T[j / 6] = ((unsigned int) getDigit(0, j + 5))*600 + getDigit(0, j + 4)*60 + getDigit(0, j + 2)*10 + getDigit(0, j + 1);
						}
						if (T[0] > T[1]) {
							Times[0] = T[0];
							Times[1] = T[1];
							clrInd();
							break;
						} else {
							setBlink(0, n, 0);
							n = 5;
							BorderUp = getBorderUp(n);
							setBlink(0, n, 1);
							setDigit(0, 5, getDigit(0, 11));
							setDigit(0, 4, getDigit(0, 10));
							setDigit(0, 2, getDigit(0, 8));
							setDigit(0, 1, getDigit(0, 7));
						}
					} else if (KeyCode == 40) { // NextDig
						KeyCode = 0;
						if (n > 1) {
							setBlink(0, n, 0);
							n == 10 || n == 7 || n == 4 ? n -= 2 : n--;
							BorderUp = getBorderUp(n);
							setBlink(0, n, 1);
						}
					} else if (KeyCode == 41) { //PrevDig
						KeyCode = 0;
						if (n < 11) {
							setBlink(0, n, 0);
							n == 8 || n == 5 || n == 2 ? n += 2 : n++;
							BorderUp = getBorderUp(n);
							setBlink(0, n, 1);
						}
					}
					key = entNum(0, BorderUp);
					if (key != 255) {
						setDigit(0, n, key);
						if ((n == 11 || n == 5) && key == 2 && (getDigit(0, n - 1) > 3))setDigit(0, n - 1, 3);
						if (n > 1) {
							KeyCode = 40;
						} else {
							KeyCode = 42;
						}
					}
				}
			} else if (KeyCode == 40) {
				KeyCode = 0;
				clrInd();
				mode = 2;
			} else if (KeyCode == 41) {
				KeyCode = 0;
				clrInd();
				mode = 3;
			}
		}
			break;
		case 2:
		{
			ItemsToIndicator(Days, 8, 9);
			key = entNum(1, 7);
			if (key != 255) {
				Days ^= ConvertDayToBit(key);
			} else if (KeyCode == 30) {
				KeyCode = 0;
				Days = 0;
			} else if (KeyCode == 45) {
				KeyCode = 0;
				Days = 0b01111111;
			} else if (KeyCode == 40) {
				KeyCode = 0;
				clrInd();
				mode = 3;
			} else if (KeyCode == 41) {
				KeyCode = 0;
				clrInd();
				mode = 1;
			}
		}
			break;
		case 3:
		{
			ItemsToIndicator(Signals, 4, 8);
			key = entNum(1, 3);
			if (key != 255) {
				Signals ^= ConvertDayToBit(key);
			} else if (KeyCode == 30) {
				KeyCode = 0;
				Signals = 0;
			} else if (KeyCode == 45) {
				KeyCode = 0;
				Signals = 0b00000111;
			} else if (KeyCode == 40) {
				KeyCode = 0;
				clrInd();
				mode = 1;
			} else if (KeyCode == 41) {
				KeyCode = 0;
				clrInd();
				mode = 2;
			}
		}
		}
	}
	return 0;
}

void Scheduler(unsigned int StartFrom) {
	clrInd();
	volatile static unsigned int NumberOfCells = 0;
	if (StartFrom != 240) {
		NumberOfCells = StartFrom;
	}
	NumberOfCells = RefreshSchedulerIndicator(NumberOfCells, 0, 240);
	while (1) {
		if (NumberOfCells != 240 && KeyCode == 44) { // Delete
			KeyCode = 0;
			WriteFourBytesEE(NumberOfCells, 0xFFFFFFFF);
			NumberOfCells = RefreshSchedulerIndicator(NumberOfCells, 0, 240);
		} else if (KeyCode == 35) { // New
			KeyCode = 0;
			SchedulerNew(240);
			RefreshSchedulerIndicator(NumberOfCells, 0, 240);
		} else if (NumberOfCells != 240 && KeyCode == 39) { // New copy
			KeyCode = 0;
			SchedulerNew(NumberOfCells);
			RefreshSchedulerIndicator(NumberOfCells, 0, 240);
		} else if (NumberOfCells != 240 && KeyCode == 45) { // Edit
			KeyCode = 0;
			EditSchedule(NumberOfCells, 240);
			RefreshSchedulerIndicator(NumberOfCells, 0, 240);
		} else if (KeyCode == 40) { // Next
			KeyCode = 0;
			NumberOfCells = FindCell(NumberOfCells, 0, 0);
			RefreshSchedulerIndicator(NumberOfCells, 0, 240);
		} else if (KeyCode == 41) { // Prev
			KeyCode = 0;
			NumberOfCells = FindCell(NumberOfCells, 0, 1);
			RefreshSchedulerIndicator(NumberOfCells, 0, 240);
		} else if (KeyCode == 43) { // Exit
			KeyCode = 0;
			clrInd();
			return;
		} else if (KeyCode == 30) { // Adress = 0
			KeyCode = 0;
			NumberOfCells = 0;
			NumberOfCells = RefreshSchedulerIndicator(NumberOfCells, 0, 240);
		}
	}
}

void SchedulerNew(unsigned int SourceOfRecord) {
	clrInd();
	volatile static unsigned char NumberOfCells = 0;
	NumberOfCells = RefreshSchedulerIndicator(NumberOfCells, 1, SourceOfRecord);
	while (1) {
		if (NumberOfCells != 240 && KeyCode == 45) { // Edit
			KeyCode = 0;
			if (EditSchedule(NumberOfCells, SourceOfRecord) == 1) {
				NumberOfCells = FindCell(NumberOfCells, 1, 0);
			}
			RefreshSchedulerIndicator(NumberOfCells, 1, SourceOfRecord);
		} else if (KeyCode == 40) { // Next
			KeyCode = 0;
			NumberOfCells = FindCell(NumberOfCells, 1, 0);
			RefreshSchedulerIndicator(NumberOfCells, 1, SourceOfRecord);
		} else if (KeyCode == 41) { // Prev
			KeyCode = 0;
			NumberOfCells = FindCell(NumberOfCells, 1, 1);
			RefreshSchedulerIndicator(NumberOfCells, 1, SourceOfRecord);
		} else if (KeyCode == 43) { // Exit
			KeyCode = 0;
			clrInd();
			return;
		} else if (KeyCode == 30) { // Adress = 0
			KeyCode = 0;
			NumberOfCells = 0;
			NumberOfCells = RefreshSchedulerIndicator(NumberOfCells, 1, SourceOfRecord);
		}
	}
}

void ParseTime(long int TimeValue, unsigned int *Time, unsigned char *Day) {
	unsigned int DayTime = (unsigned int) (TimeValue / 6000);
	*Time = DayTime % 1440;
	*Day = DayTime / 1440 + 1;
}

void ParseDataRecord(unsigned long int Data, unsigned int *TimeStart, unsigned int *TimeStop, unsigned char *Days, unsigned char *Signals) {
	//	if(Data!=0xFFFFFFFF){
	*TimeStart = (unsigned int) (Data % (unsigned long int) 2048);
	Data /= 2048;
	*TimeStop = (unsigned int) (Data % (unsigned long int) 2048);
	Data /= 2048;
	*Days = (unsigned char) (Data % (unsigned long int) 128);
	Data /= 128;
	*Signals = (unsigned char) (Data % (unsigned long int) 8);
	//	}
}

unsigned int FindNextTimeStart(unsigned long int *TimeFrom) {

	unsigned char adress = 0;
	unsigned char adressStart = 240;
	unsigned char TimeIsFound = 0;
	unsigned long int LastFoundTimeStart;

	do {
		unsigned long int Data = ReadFourBytesEE(adress);

		adress += 4;
		if (Data == 0xFFFFFFFF) {
			continue;
		}
		unsigned int TimeStart;
		unsigned int TimeStop;
		unsigned char Days;
		unsigned char Signals;
		ParseDataRecord(Data, &TimeStart, &TimeStop, &Days, &Signals);

		if (Signals == 0) {
			continue;
		}
		for (char DayIndex = 0; DayIndex < 7; DayIndex++) {
			unsigned char DayOn = Days % 2;
			Days /= 2;
			if (DayOn != 1) {
				continue;
			}
			unsigned long int TimeOfSignals = ((unsigned long int) DayIndex * 86400 + (unsigned long int) TimeStart * 60) * 100;

			if ((TimeOfSignals < LastFoundTimeStart || TimeIsFound == 0) && TimeOfSignals > *TimeFrom) {
				TimeIsFound = 1;
				LastFoundTimeStart = TimeOfSignals;
				adressStart = adress - 4;
			}
		}
	} while (adress < 240);

	if (TimeIsFound == 1) {
		*TimeFrom = LastFoundTimeStart;
	} else {
		*TimeFrom = 0;
	}
	return adressStart;
}

void SignalsOnOff() {
	if (!flags.TimeIsRead) return;
	if (!flags.LockSignals && CurrentReadingCell < 240) {
		unsigned long int Data = ReadFourBytesEE(CurrentReadingCell);
		if (Data != 0xFFFFFFFF) {
			unsigned int TimeStart;
			unsigned int TimeStop;
			unsigned char Days;
			unsigned char Signals;
			TimeStart = Data % 2048;
			Data /= 2048;
			TimeStop = Data % 2048;
			Data /= 2048;
			Days = Data % 128;
			Data /= 128;
			Signals = Data % 8;
			if (Signals != 0 && Days != 0) {
				unsigned int ThisTime;
				unsigned char ThisDay;
				unsigned int DayTime = (unsigned int) (Clock / 6000);
				ThisTime = DayTime % 1440;
				ThisDay = DayTime / 1440 + 1;
				ThisDay = ConvertDayToBit(ThisDay);
				if (((ThisDay & Days) > 0) && (TimeStart <= ThisTime) && (TimeStop > ThisTime)) {
					SignalsOut = Signals | SignalsOut;
				}
			}
		}
		CurrentReadingCell += 4;
	} else {
		SignalsForInd = (flags.LockSignals ? 0 : SignalsOut);
		SignalsOut = 0;
		CurrentReadingCell = 0;
		unsigned char Data = (PORTB & 0b00011111) | ((CurrentSignals | SignalsForInd)*32);
		if ((Data & 0b11100000) != CurrentSignalsData) {
			EEWR(cMinutesAdress, cMinutes);
			EEWR(cHoursAdress, cHours);
			EEWR(cWeekDayAdress, cWeekDay);
			EEWR(cDaysAdress, cDays);
			EEWR(cMonthsAdress, cMonths);
			EEWR(cYearsAdress, cYears);
		}
		CurrentSignalsData = Data & 0b11100000;
		PORTB = Data;
	}
}

void FillMinutes() {
	Minutes = Clock - 3000;
}

void interrupt high_priority F_h() {
	ClrWdt();
}

void interrupt low_priority F_l() {
	ClrWdt();
	if (TMR1IF) {
		TMR1IF = 0;
		TMR1 += 51200;
		SignalsOnOff();
		char MinuteAgo = (Clock - Minutes > 6000);
		if (MinuteAgo > 0) {
			FillMinutes();
			flags.RelevanceOfNextStartCell = 0;
		}

		if (flags.LCD_Power_On && (GlobalBlinkCycleTime == 100 || flags.IsLCDModified)) {
			for (char line = 0; line < 2; line++) {
				for (int symbol = 0; symbol < 16; symbol++) {
					if (GlobalBlinkCycleTime == 100 || digitsAtrib[line][symbol].modifided > 0) {
						unsigned char digit = digits[line][symbol];
						lcd_send_byte((line * 0x40 + symbol) | 0b10000000); //SetDDRAM
						if (!(digitsAtrib[line][symbol].blink == 0 || digitsAtrib[line][symbol].blink == 1 && flags.GlobalBlink)) {
							digit = '_';
						}
						SendSymbolToLCD(getLcdCodeOfChar(digit));
						digitsAtrib[line][symbol].modifided = 0;
					}
				}
			}
			flags.IsLCDModified = 0;
		}

		if (GlobalBlinkCycleTime > 0) {
			GlobalBlinkCycleTime--;
		} else {
			flags.IsLCDModified = 1;
			GlobalBlinkCycleTime = 100;
			flags.GlobalBlink = !flags.GlobalBlink;

			// 	ReadTime();
		}

		static unsigned char ButtonPressTimeOut;

		static unsigned char KeyIndex;
		static unsigned char PressedKeyIndex = 0xFF;

		if (ButtonPressTimeOut > 0) {
			ButtonPressTimeOut--;
		} else {
			KeyCode = 0;
			if (RB7) {
				if (KeyIndex == 0x0F) {
					KeyIndex = 0;
				} else {
					KeyIndex++;
				}
				unsigned char data_temp = KeyIndex;
				unsigned char _KeyIndex = 0;
				for (int a = 4; a > 0; a--) {
					_KeyIndex *= 2;
					if (data_temp % 2 == 1) {
						_KeyIndex++;
					}
					data_temp /= 2;
				}

				PORTB = _KeyIndex * 2; //(PORTB & 0b0000000) | ()
				PressedKeyIndex = 0xFF;
			} else {
				if (PressedKeyIndex == 0xFF) {
					PressedKeyIndex = KeyIndex;
					KeyCode = ConvKeyNum(PressedKeyIndex);
					ButtonPressTimeOut = 30;
				}
			}
		}
		T0IF = 0;
	}
}

void main() {

	flags.TimeIsRead = 0;

	INTCON = 0;
	INTCON2 = 0;
	INTCON3 = 0;
	PIR1 = 0;
	PIR2 = 0;
	PIE1 = 0;
	PIE2 = 0;
	IPR1 = 0;
	IPR2 = 0;
	RCON = 0;

	clrInd();

	RCONbits.IPEN = 1; // ��������� ��������������� ����������

	INTCONbits.GIEH = 1;
	INTCONbits.GIEL = 1;

	T2CONbits.TMR2ON = 1;

	T1CONbits.RD16 = 1;
	TMR1 = 51200;
	T1CONbits.TMR1ON = 1;
	PIE1bits.TMR1IE = 1;

	ADCON1 = 0b00001111;
	TRISA = 0b00000000;
	TRISB = 0b10000000;
	TRISC = 0b00000000;

	Clock = 0;
	LATA = 0b00000000;
	LATB = 0b00000000;
	LATC = 0b00000000;
	T2CON = 0b00100100;

	flags.LockSignals = EERD(251);

	lcd_on();
	main2();
}

unsigned char getNumChar(unsigned char num) {
	switch (num) {
		case 0: return '0';
		case 1: return '1';
		case 2: return '2';
		case 3: return '3';
		case 4: return '4';
		case 5: return '5';
		case 6: return '6';
		case 7: return '7';
		case 8: return '8';
		case 9: return '9';
		default: return ' ';
	}
}

void outputString(unsigned char * stringData, unsigned char line, unsigned char position) {
	unsigned int i = 0;
	while(stringData[i] != '\0'){
		unsigned char l = line;
		for(line = 0; l < 2; l++) {
			unsigned char s = position;
			for (position = 0; (s < 16) && (stringData[i] != '\0'); s++) {
				setDigit(l, s, stringData[i]);
				i++;
			}
		}
	}
}

void drowText(unsigned char * stringData, int startNum, int direction){
	unsigned char srcLine, destLine;
	if(direction >= 0){
		srcLine = 1;
		destLine = 0;
	}else{
		srcLine = 0;
		destLine = 1;
	}
	
	for(unsigned char symbol = 0; symbol < 16; symbol++){
		digits[destLine][symbol] = digits[srcLine][symbol];
		digitsAtrib[destLine][symbol].blink = digitsAtrib[srcLine][symbol].blink;
		digitsAtrib[destLine][symbol].modifided = 1;
	}
	
	flags.IsLCDModified = 1;
	
	unsigned char cutedString[17] = 
	for(unsigned char symbol = 0; symbol < 16; symbol++) {
		unsigned char a = stringData[symbol + startNum];
		if(a == '\0'){
			break;
		}
		cutedString[symbol] = a;
	}
	
	outputString(cutedString, (direction >= 0 ? 1 : 0), 0);
}

void main2() {

	flags.isTimeSetting = 0;

	NearTimeStart = Clock;
	AdressOfNextStartCell = FindNextTimeStart(&NearTimeStart);
	
//	outputString("��������� ���� ~!@#$^%# AbCd()#�", 0, 0);
//	outputString("!@#$%^&*()_+=-/", 0, 0);
//	unsigned char data[] = "����� (�� ���. textus � ������; ���������, �����, �������, ���������) - ��������������� �� �����-���� ������������ �������� ������������ �����; � ����� ����� ������� � ������ ������������������ ��������.";
//	outputString(data, 0, 0);
//	drowText(data, 0, 0);
//	drowText(data, 16, 0);
//	
//	int position = 0;
//	int maxPosition = sizeof(data) - 2;
	
	while (1) {
		
//		if (KeyCode == 40) {
//			KeyCode = 0;
//			if(position < maxPosition){
//				drowText(data, position, 0);
//				position += 16;
//			}
//		}else if (KeyCode == 41) {
//			KeyCode = 0;
//			if (position - 16 >= 0) {
//				position -= 16;
//				if (position < 0){
//					position = 0;
//				}
//				drowText(data, position, -1);
//			}
//			
//		}

		if (flags.LCD_Power_On == 1 && LCD_ON_TIMEOUT == 0 && CurrentSignals == 0) {
			lcd_off();
		} else if (flags.LCD_Power_On == 0 && KeyCode != 0) {
			KeyCode = 0;
			lcd_on();
		}

		if (flags.LCD_Power_On == 0) {
			continue;
		}

		TimeToInd();

		if (KeyCode == 45 && !flags.DetailModeOfViewSheduler) {
			KeyCode = 0;
			TimeEdit();
		} else if (KeyCode == 40 || KeyCode == 41) {
			KeyCode = 0;
			Scheduler(240);
		} else if (KeyCode == 44) {
			KeyCode = 0;
			if (flags.LCD_Power_On) {
				flags.LCD_Light_On = !flags.LCD_Light_On;
			}
		} else if (KeyCode == 30) {
			KeyCode = 0;
			flags.LockSignals = 1;
			CurrentSignals = 0;
			EEWR(251, flags.LockSignals);
		} else if (KeyCode == 31 && !flags.DetailModeOfViewSheduler) {
			KeyCode = 0;
			clrInd();
			CurrentSignals = CurrentSignals^0b00000001;
		} else if (KeyCode == 32 && !flags.DetailModeOfViewSheduler) {
			KeyCode = 0;
			clrInd();
			CurrentSignals = CurrentSignals^0b00000010;
		} else if (KeyCode == 33 && !flags.DetailModeOfViewSheduler) {
			KeyCode = 0;
			clrInd();
			CurrentSignals = CurrentSignals^0b00000100;
		} else if (KeyCode == 34 && !flags.DetailModeOfViewSheduler) {
			KeyCode = 0;
			clrInd();
			CurrentSignals = 0;
		} else if (KeyCode == 35 && !flags.DetailModeOfViewSheduler) {
			KeyCode = 0;
			flags.LockSignals = 0;
			EEWR(251, flags.LockSignals);
			clrInd();
		} else if (KeyCode == 37) {
			KeyCode = 0;
			lcd_off();
		} else if (KeyCode == 43 || (KeyCode == 36 && (!flags.LockSignals || flags.DetailModeOfViewSheduler))) {
			if (KeyCode == 43) {
				NearTimeStart = Clock;
				if (flags.DetailModeOfViewSheduler) {
					flags.DetailModeOfViewSheduler = 0;
					clrInd();
				}
			}
			KeyCode = 0;
			FillMinutes();
			AdressOfNextStartCell = FindNextTimeStart(&NearTimeStart);
		}
	}
}

unsigned char BCD_to_DEC(unsigned char BCD) {
	return ((BCD & 0b11110000) >> 4) * 10 + (BCD & 0b00001111);
}

unsigned char DEC_to_BCD(unsigned char DEC) {
	if (DEC > 99) {
		DEC = 99;
	}
	return ((DEC / 10) << 4) +DEC % 10;
}

void ReadTime() {

	unsigned char Seconds = 0x00;

	I2CInit();
	I2CStart();
	I2CSend(0xD0);
	I2CSend(0x00);
	I2CRestart();
	I2CSend(0xD1);

	unsigned char I = I2CRead();

	if ((I & 0b10000000) == 0) {
		flags.TimeIsRead = 1;
		I2CAck();
		Seconds = I;
		cMinutes = I2CRead();
		I2CAck();
		cHours = I2CRead();
		I2CAck();
		cWeekDay = I2CRead();
		if (!flags.isTimeSetting) {
			I2CAck();
			cDays = I2CRead();
			I2CAck();
			cMonths = I2CRead();
			I2CAck();
			cYears = I2CRead();
		}
	} else {
		flags.TimeIsRead = 0;
		I2CNak();
		I2CStop();
		cDays = EERD(cDaysAdress);
		cMonths = EERD(cMonthsAdress);
		cYears = EERD(cYearsAdress);
		WriteTime(
			EERD(cMinutesAdress),
			EERD(cHoursAdress),
			EERD(cWeekDayAdress)
			);
		return;
	}
	I2CNak();

	I2CStop();

	Clock = ((long int) BCD_to_DEC(Seconds)
		+ (long int) BCD_to_DEC(cMinutes) * 60
		+ (long int) BCD_to_DEC(cHours) * 3600
		+ ((long int) cWeekDay - 1) * 86400) * 100;

}

void WriteTime(unsigned char Minutes, unsigned char Hours, unsigned char DayOfWeek) {

	volatile unsigned char INTCON_BUP = INTCON;
	INTCONbits.GIEH = 0;
	INTCONbits.GIEL = 0;
	ClrWdt();

	I2CInit();
	I2CStart();
	I2CSend(0xD0);
	I2CSend(0x00);

	I2CSend(0x00); // Seconds
	I2CSend(Minutes); // Minutes
	I2CSend(Hours); // Hours
	I2CSend(DayOfWeek); // DayOfWeek
	I2CSend(cDays); // Days
	I2CSend(cMonths); // Mouths
	I2CSend(cYears); // Years
	I2CSend(0b00010000); // Settings

	I2CStop();

	Clock = ((long int) BCD_to_DEC(Minutes) * 60
		+ (long int) BCD_to_DEC(Hours) * 3600
		+ ((long int) DayOfWeek - 1) * 86400) * 100;

	INTCON = INTCON_BUP;
}

void I2CInit(void) {
	TRISCbits.TRISC3 = 1; /* SDA and SCL as input pin */
	TRISCbits.TRISC4 = 1; /* these pins can be configured either i/p or o/p */
	SSPSTAT |= 0b10000000; /* Slew rate disabled */
	SSPADD = 0x18; /* 100Khz @ 40Mhz Fosc */
	SSPCON1 = 0b00101000; /* SSPEN = 1, I2C Master mode, clock = FOSC/(4 * (SSPADD + 1)) */
}

void I2CStart() {
	SSPCON2bits.SEN = 1; /* Start condition enabled */
	while (SSPCON2bits.SEN); /* automatically cleared by hardware */
	/* wait for start condition to finish */
}

void I2CStop() {
	SSPCON2bits.PEN = 1; /* Stop condition enabled */
	while (SSPCON2bits.PEN); /* Wait for stop condition to finish */
	/* PEN automatically cleared by hardware */
}

void I2CRestart() {
	SSPCON2bits.RSEN = 1; /* Repeated start enabled */
	while (SSPCON2bits.RSEN); /* wait for condition to finish */
}

void I2CAck() {
	SSPCON2bits.ACKDT = 0; /* Acknowledge data bit, 0 = ACK */
	SSPCON2bits.ACKEN = 1; /* Ack data enabled */
	while (SSPCON2bits.ACKEN); /* wait for ack data to send on bus */
}

void I2CNak() {
	SSPCON2bits.ACKDT = 1; /* Acknowledge data bit, 1 = NAK */
	SSPCON2bits.ACKEN = 1; /* Ack data enabled */
	while (SSPCON2bits.ACKEN); /* wait for ack data to send on bus */
}

void I2CSend(unsigned char dat) {
	SSPBUF = dat; /* Move data to SSPBUF */
	while (SSPSTATbits.BF); /* wait till complete data is sent from buffer */
	while ((SSPCON2 & 0b00011111) || (SSPSTAT & 0b00000100)); //I2CWait();       /* wait for any pending transfer */
}

unsigned char I2CRead(void) {
	unsigned char temp;
	/* Reception works if transfer is initiated in read mode */
	SSPCON2bits.RCEN = 1; /* Enable data reception */
	while (!SSPSTATbits.BF); /* wait for buffer full */
	temp = SSPBUF; /* Read serial buffer and store in temp register */
	while ((SSPCON2 & 0b00011111) || (SSPSTAT & 0b00000100)); //I2CWait();       /* wait to check any pending transfer */
	return temp; /* Return the read data from bus */
}
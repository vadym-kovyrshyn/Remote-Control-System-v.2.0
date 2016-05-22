
#include <xc.h>
#include <stdio.h>
#include <pic18f2620.h>


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

unsigned char digits[32];
unsigned char digits_system_message[32];

typedef struct {
	unsigned blink: 1;
	unsigned modifided: 1;
} digits_atributes;

#define CELL_CAPACITY 8
#define END_OF_CELLS (CELL_CAPACITY * 48)
#define LAST_CELL (END_OF_CELLS - CELL_CAPACITY)


digits_atributes digitsAtrib[32];
digits_atributes digitsAtrib_system_message[32];
unsigned long int _systemCounter = 0;
long int Clock;
long int Minutes = 0;
unsigned int LCD_ON_TIMEOUT = 60000; // 3 min
volatile unsigned char KeyCode;
unsigned int CorrectTimeConst;
unsigned int AdressOfNextStartCell = END_OF_CELLS;
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
	unsigned GSM_Connected: 1;
	unsigned ActiveCall: 1;
	unsigned UnreadSystemMessage: 1;
	unsigned UnprocessedIncommingUartData: 1;
	unsigned CurrentSignalsSetted: 1;
	unsigned RemoteControlIsEnabled: 1;
} flags;

unsigned char cMinutes = 0;
unsigned char cHours = 0;
unsigned char cWeekDay = 1;
unsigned char cDays = 1;
unsigned char cMonths = 1;
unsigned char cYears = 1;

unsigned char cMinutesAdress	= END_OF_CELLS;
unsigned char cHoursAdress		= END_OF_CELLS + 1;
unsigned char cWeekDayAdress	= END_OF_CELLS + 2;
unsigned char cDaysAdress		= END_OF_CELLS + 3;
unsigned char cMonthsAdress		= END_OF_CELLS + 4;
unsigned char cYearsAdress		= END_OF_CELLS + 5;

#define LOCK_SIGNALS_FLAG_CELL	(END_OF_CELLS + 6)

unsigned char CurrentSignals = 0;
unsigned char GlobalBlinkCycleTime = 70;

typedef struct {
	unsigned blink: 1;
	unsigned const_symbol: 1;
} symbol_properties;

enum InputType{
	aa,
	Aa,
	AA
};

enum InputLang{
	English,
	Russian,
	Numeric
};

enum SystemEvents{
	
};

typedef struct {
	unsigned Aa: 2;
	unsigned Language: 2;
} input_text_props;

typedef struct {
    symbol_properties props;
    unsigned char number;
    unsigned char symbol;
    unsigned char index;
} SymbolData;

typedef struct {
    SymbolData *Time [5];
    SymbolData *WeekDay [1];
    SymbolData *Date [10];
} TimeEditData;

typedef struct {
	unsigned char name[17];
	unsigned char phone [14];
	unsigned char blocked[1];
	unsigned char filled[1];
} Phone;

#define PHONEBOOK_START_ADRESS (END_OF_CELLS + 7)
#define PHONEBOOK_CAPACITY 16
#define END_OF_PHONEBOOK (PHONEBOOK_START_ADRESS + PHONEBOOK_CAPACITY * sizeof(Phone))
#define LAST_PHONEBOOK_CELL (END_OF_PHONEBOOK - sizeof(Phone))


#define USART_SPEED_NUMBER_CELL (END_OF_PHONEBOOK + 1)

#define _XTAL_FREQ 40000000

#define E	RC0
#define RS	RC2
//char LCD_Power_On_PIN = 0;
//#define LCD_Power_On_PIN	RC2
#define EMPTY_SYMBOL_VALUE ' '
#define UNDERSCORE_SYMBOL_VALUE '_' //_
#define EMPTY_STRING_16 "                "
#define BELL_SYMBOL '\x07'

#define BUFFER_STRING_LENGTH 256
#define MAX_INCOMMING_BUFF_INDEX 3
#define LAST_INCOMMING_DATA_INDEX (MAX_INCOMMING_BUFF_INDEX - 1)
#define INCOMMING_BUFF_LENGTH (MAX_INCOMMING_BUFF_INDEX + 1)
/*
unsigned char buff_string_0 [BUFFER_STRING_LENGTH];
unsigned char buff_string_1 [BUFFER_STRING_LENGTH];
unsigned char buff_string_2 [BUFFER_STRING_LENGTH];
unsigned char buff_string_3 [BUFFER_STRING_LENGTH];
unsigned char buff_string_4 [BUFFER_STRING_LENGTH];
unsigned char buff_string_5 [BUFFER_STRING_LENGTH];
unsigned char buff_string_6 [BUFFER_STRING_LENGTH];
unsigned char buff_string_7 [BUFFER_STRING_LENGTH];
*/
unsigned char buff_string_8 [BUFFER_STRING_LENGTH];
unsigned char buff_string_9 [BUFFER_STRING_LENGTH];
unsigned char buff_string_10 [BUFFER_STRING_LENGTH];
unsigned char buff_string_11 [BUFFER_STRING_LENGTH];

unsigned char *IncommingBuffer[INCOMMING_BUFF_LENGTH];

unsigned char *OutcommingBuffer = NULL;

const unsigned char StandardAnswer_OK [] = "OK";
//	+CCLK: "16/05/15,22:37:52+03"
const unsigned char StandardAnswer_CLOCK [] = "+CCLK: \"\x10\x10/\x10\x10/\x10\x10,\x10\x10:\x10\x10:\x10\x10\x10\x10\x10\"";
//	+CLIP: "+380957075762",145,"",,"",0
const unsigned char StandardAnswer_INCCALL [] = "+CLIP: \"\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10\",145,\"\",,\"\",0";
const unsigned char StandardAnswer_BUSY [] = "BUSY";
const unsigned char StandardAnswer_NO_ANSWER [] = "NO ANSWER";
const unsigned char StandardAnswer_NO_CARRIER [] = "NO CARRIER";
const unsigned char StandardAnswer_NO_DIALTONE [] = "NO DIALTONE";
const unsigned char StandardAnswer_DTMF [] = "+DTMF:\x10";
const unsigned char StandardAnswer_ERROR [] = "ERROR";
/*
const unsigned char StandardAnswer_ [] = "";
const unsigned char StandardAnswer_ [] = "";
const unsigned char StandardAnswer_ [] = "";
const unsigned char StandardAnswer_ [] = "";
*/	

unsigned char Clock_from_GSM[30];
unsigned char Incomming_Call_Data[36];
unsigned char Incomming_DTMF_Data[] = "       ";
unsigned char DTMF_Symbol;

Phone IncommingPhone;

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
unsigned char getNumChar(unsigned char num);
void I2CInit(void);
void I2CStart(void);
void I2CStop(void);
void I2CRestart(void);
void I2CAck(void);
void I2CNak(void);
void I2CWait(void);
void I2CSend(unsigned char dat);
void OutputString(unsigned char * stringData, unsigned char line, unsigned char position, unsigned char trasnsfer_line);
unsigned char I2CRead(void);
void ReadTime();
void WriteTime(unsigned long int lClock, unsigned char days, unsigned char months, unsigned char years);
unsigned char BCD_to_DEC(unsigned char BCD);
unsigned char DEC_to_BCD(unsigned char DEC);
void FillArrayFromEEPROM(unsigned char * container, unsigned int adress_start, unsigned int quantity);
void WriteArrayToEEPROM(unsigned char * container, unsigned int adress_start, unsigned int quantity);
void ReadDataOfCell(unsigned char *CellsData, unsigned long int *Data, unsigned int TargetAdress);
void CleanStringArray(unsigned char *myArray, unsigned int size, unsigned char settingData);
unsigned long int getSystemTimePoint(void);
unsigned char testTimePoint(unsigned long int point, unsigned long int value);
void PhonebookEdit(void);
unsigned char *InputText(unsigned char *text, input_text_props *_props, unsigned char capacity);
unsigned char Select_OK_NO(unsigned char *text);
unsigned char FindIncommingData(unsigned char *regexp, unsigned char *container, int history_index);
unsigned char _FindIncommingData_interrupt(unsigned char *regexp, unsigned char *container, int history_index);
char ShowMenu(unsigned char *menu[], unsigned char item);
void OutputSystemMessage(unsigned char *stringData);
void SendCommandToUSART(unsigned char *command, unsigned char wait);
unsigned char USART_Speed_Choise(void);
void Save_USART_SpeedNumber(unsigned char speed);
void Settings(void);
unsigned char Read_USART_SpeedNumber(void);
unsigned int GetStringLength(unsigned char *container);
unsigned int _GetStringLength_interr(unsigned char *container);

unsigned char getDigit(char line, char symbol) {
	return *(digits + 16 * line + symbol);
}

void setDigit(char line, char symbol, unsigned char data) {
	unsigned char value = data;
	unsigned char value0 = *(digits + 16 * line + symbol);
	(*(digitsAtrib + 16 * line + symbol)).modifided = (*(digitsAtrib + 16 * line + symbol)).modifided > 0 || (value != value0);
	flags.IsLCDModified = flags.IsLCDModified || (value != value0);
	*(digits + 16 * line + symbol) = value;
}

void setBlink(char line, char symbol, unsigned char value) {
	if (value != 0) {
		(*(digitsAtrib + 16 * line + symbol)).blink = 1;
	} else {
		(*(digitsAtrib + 16 * line + symbol)).blink = 0;
	}
	(*(digitsAtrib + 16 * line + symbol)).modifided = 1;
	flags.IsLCDModified = 1;
}

void clrInd() {
	for (char line = 0; line < 2; line++) {
		for (char symb = 0; symb < 16; symb++) {
			*(digits + line * 16 + symb) = ' ';
			(*(digitsAtrib + line * 16 + symb)).blink = 0;
			(*(digitsAtrib + line * 16 + symb)).modifided = 1;
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
	case '�': return 0x27;
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
    case '\x07': return 0xED; //bell

	default: return 0x3F; // '?'
	}
}

unsigned char CheckoutRegisterOfSymbol(unsigned char dig) {
	switch (dig) {
	case 'A': return 'a';
	case 'a': return 'A';
	case 'B': return 'b';
	case 'b': return 'B';
	case 'C': return 'c';
	case 'c': return 'C';
	case 'D': return 'd';
	case 'd': return 'D';
	case 'E': return 'e';
	case 'e': return 'E';
	case 'F': return 'f';
	case 'f': return 'F';
	case 'G': return 'g';
	case 'g': return 'G';
	case 'H': return 'h';
	case 'h': return 'H';
	case 'I': return 'i';
	case 'i': return 'I';
	case 'J': return 'j';
	case 'j': return 'J';
	case 'K': return 'k';
	case 'k': return 'K';
	case 'L': return 'l';
	case 'l': return 'L';
	case 'M': return 'm';
	case 'm': return 'M';
	case 'N': return 'n';
	case 'n': return 'N';
	case 'O': return 'o';
	case 'o': return 'O';
	case 'P': return 'p';
	case 'p': return 'P';
	case 'Q': return 'q';
	case 'q': return 'Q';
	case 'R': return 'r';
	case 'r': return 'R';
	case 'S': return 's';
	case 's': return 'S';
	case 'T': return 't';
	case 't': return 'T';
	case 'U': return 'u';
	case 'u': return 'U';
	case 'V': return 'v';
	case 'v': return 'V';
	case 'W': return 'w';
	case 'w': return 'W';
	case 'X': return 'x';
	case 'x': return 'X';
	case 'Y': return 'y';
	case 'y': return 'Y';
	case 'Z': return 'z';
	case 'z': return 'Z';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	case '�': return '�';
	default: return dig;
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

void NumericToString(unsigned long int n, unsigned char * mySring, unsigned char size) {
	if(mySring[size-1] == '\0'){
		size--;
	}
	unsigned char displacement = 1;
	do {
		mySring[size - displacement] = getNumChar(n % 10);
		n /= 10;
		displacement++;
	} while ((size - (displacement - 1) > 0) && (n > 0));
}

unsigned char *GetDayOfWeek(unsigned char day) {
	if (day == 1) {
        return "��";
	}
	else if (day == 2) {
        return "��";
	}
	else if (day == 3) {
        return "��";
	}
	else if (day == 4) {
        return "��";
	}
	else if (day == 5) {
        return "��";
	}
	else if (day == 6) {
        return "��";
	}
	else if (day == 7) {
        return "��";
	}
	return "??";
}

void TimeToInd() {
	if(flags.CurrentSignalsSetted){
		flags.CurrentSignalsSetted = 0;
		clrInd();
	}
	
	if (KeyCode == 32 && (AdressOfNextStartCell != END_OF_CELLS || flags.DetailModeOfViewSheduler)) {
		KeyCode = 0;
		clrInd();
		flags.DetailModeOfViewSheduler = !flags.DetailModeOfViewSheduler;
	}
	if (KeyCode == 45 && AdressOfNextStartCell != END_OF_CELLS && flags.DetailModeOfViewSheduler) {
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
	
	if (AdressOfNextStartCell == END_OF_CELLS || !flags.DetailModeOfViewSheduler) {
		unsigned char SignalsFinal = CurrentSignals | SignalsForInd;
		if (flags.ModeOfFirstLine != (SignalsFinal > 0)) {
			clrInd();
			flags.ModeOfFirstLine = SignalsFinal > 0;
		}
		if (SignalsFinal > 0) {

			unsigned char D [] = "   ";
			for (unsigned char i = 1; i < 4; i++) {
				unsigned char SignalOn = SignalsFinal % 2;
				SignalsFinal /= 2;
				if (SignalOn == 1) {
					D[0] = D[1];
					D[1] = D[2];
					D[2] = getNumChar(i);
				}
			}
			OutputString(D, 0, 13, 0);
		}
        
        unsigned char TimeData [] = "--:--:--"; // ��:��:��
		long int temp = Clock / 100;
        TimeData[7] = getNumChar(temp % 10);
		temp /= 10;
        TimeData[6] = getNumChar(temp % 6);
		temp /= 6;
        TimeData[4] = getNumChar(temp % 10);
		temp /= 10;
        TimeData[3] = getNumChar(temp % 6);
		temp /= 6;
		unsigned char day = temp / 24 + 1;
		temp %= 24;
        TimeData[1] = getNumChar(temp % 10);
        TimeData[0] = getNumChar(temp / 10);
        
        SignalsFinal = CurrentSignals | SignalsForInd;
        OutputString(TimeData, 0, (SignalsFinal == 7) ? 4 : 5, 0);
        OutputString(GetDayOfWeek(day), 0, (SignalsFinal == 7) ? 1 : 2, 0);
	}
    
	if (flags.LockSignals && !flags.DetailModeOfViewSheduler) {
		OutputString("���������� ����.", 1, 0, 0);
		return;
	}

	if (AdressOfNextStartCell != END_OF_CELLS) {

    	unsigned char DataArray[] = EMPTY_STRING_16;
		DataArray[0] = BELL_SYMBOL;

		char line = 1;
		if (flags.DetailModeOfViewSheduler) {
			line = 0;
            DataArray[5] = '�';
		}
        
		unsigned long int Data = 0;
		unsigned char CellsData [CELL_CAPACITY];
		ReadDataOfCell(&CellsData, &Data, AdressOfNextStartCell);

		unsigned int TimeStart;
		unsigned int TimeStop;
		unsigned char Days;
		unsigned char Signals;
		ParseDataRecord(Data, &TimeStart, &TimeStop, &Days, &Signals);
        
		unsigned int Time;
		unsigned char Day;
		ParseTime(NearTimeStart, &Time, &Day);

		DataArray[11] = getNumChar(Time % 10);
		Time /= 10;
		DataArray[10] = getNumChar(Time % 6);
		Time /= 6;
		DataArray[9] = ':';
		DataArray[8] = getNumChar(Time % 10);
		DataArray[7] = getNumChar(Time / 10);
        
		for (unsigned char i = 1; i < 4; i++) {
			unsigned char SignalOn = Signals % 2;
			Signals /= 2;
			if (SignalOn == 1) {
				DataArray[13] = DataArray[14];
				DataArray[14] = DataArray[15];
				DataArray[15] = getNumChar(i);
			}
		}
        
		unsigned char *DataArray2 = GetDayOfWeek(Day);
		DataArray[2] = DataArray2[0];
		DataArray[3] = DataArray2[1];

        OutputString(DataArray, line, 0, 0);

		if (flags.DetailModeOfViewSheduler) {
			unsigned char DataArrayOfLine2[] = "�   �� --:--"; // �12   �� 12:30
            int j2 = sizeof(DataArrayOfLine2)-1;
			DataArrayOfLine2[--j2] = getNumChar(TimeStop % 10);
			TimeStop /= 10;
			DataArrayOfLine2[--j2] = getNumChar(TimeStop % 6);
			TimeStop /= 6;
            j2--;
			DataArrayOfLine2[--j2] = getNumChar(TimeStop % 10);
			DataArrayOfLine2[--j2] = getNumChar(TimeStop / 10);

			unsigned char CellsNumber = AdressOfNextStartCell / CELL_CAPACITY + 1;
			do {
				char d = getNumChar(CellsNumber % 10);
				DataArrayOfLine2[2] = DataArrayOfLine2[1];
				DataArrayOfLine2[1] = d;
				CellsNumber /= 10;
			} while (CellsNumber > 0);

			OutputString(DataArrayOfLine2, 1, 0, 0);
		}
	} else {
		if (flags.DetailModeOfViewSheduler) {
			clrInd();
			flags.DetailModeOfViewSheduler = 0;
		}
        OutputString(EMPTY_STRING_16, 1, 0, 0);
	}
}

void EEWR(unsigned int adress, unsigned char data) {
	volatile unsigned char INTCON_BUP = INTCON;
	INTCONbits.GIEH = 0;
	INTCONbits.GIEL = 0;
	CLRWDT();
	EEADRH = adress >> 8;
	EEADR = adress % 256;
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
	EEADR = adress % 256;
	EECON1bits.EEPGD = 0;
	EECON1bits.CFGS = 0;
	EECON1bits.RD = 1;
	EEDATA_BUP = EEDATA;
	INTCON = INTCON_BUP;
	return EEDATA_BUP;
}

void OutputInteractiveData(SymbolData *data[], unsigned char length, unsigned char start_symbol){
	for(unsigned char i = 0; i < length; i++){
		unsigned char s = start_symbol + (*(data[i])).index;
		unsigned char line = s/16;
		if(line == 1){
			s-=16;
		}
		setDigit(line, s, (*(data[i])).symbol);
		setBlink(line, s, (*(data[i])).props.blink);
	}
}

void ReIndTimeEdit(TimeEditData *TimeData) {
	for(char i = 0; i < 10; i++){
		if((*((*TimeData).Date[i])).props.const_symbol == 0){
			(*((*TimeData).Date[i])).symbol = getNumChar((*((*TimeData).Date[i])).number);
		}
		(*((*TimeData).Date[i])).index = i;
	}
	for(char i = 0; i < 5; i++){
		if((*((*TimeData).Time[i])).props.const_symbol == 0){
			(*((*TimeData).Time[i])).symbol = getNumChar((*((*TimeData).Time[i])).number);
		}
		(*((*TimeData).Time[i])).index = i;
	}
	(*((*TimeData).WeekDay[0])).symbol = getNumChar((*((*TimeData).WeekDay[0])).number);
	(*((*TimeData).WeekDay[0])).index = 0;
	
	char n = 1;
	OutputInteractiveData((*TimeData).Date, 10, n);
	OutputInteractiveData((*TimeData).Time, 5, n+4+16);
	OutputInteractiveData((*TimeData).WeekDay, 1, n+1+16);
}

void InteractiveData_BlinkOFF(SymbolData * InteractiveData[], unsigned char leght){
	for(char i = 0; i < leght; i++){
		(*(*(InteractiveData + i))).props.blink = 0;
	}
}

void TimeData_BlinkOFF(TimeEditData * TimeData){
	InteractiveData_BlinkOFF((*TimeData).Date, 10);
	InteractiveData_BlinkOFF((*TimeData).Time, 5);
	InteractiveData_BlinkOFF((*TimeData).WeekDay, 1);
}

void TimeData_INIT(TimeEditData * TimeData){
	for(char i = 0; i < 10; i++){
		(*((*TimeData).Date[i])).props.blink = 0;
		(*((*TimeData).Date[i])).props.const_symbol = 0;
	}
	for(char i = 0; i < 5; i++){
		(*((*TimeData).Time[i])).props.blink = 0;
		(*((*TimeData).Time[i])).props.const_symbol = 0;
	}
	(*((*TimeData).WeekDay[0])).props.blink = 0;
	(*((*TimeData).WeekDay[0])).props.const_symbol = 0;

	(*((*TimeData).Time[2])).symbol = ':';
	(*((*TimeData).Time[2])).props.const_symbol = 1;
	(*((*TimeData).Date[5])).symbol = '/';
	(*((*TimeData).Date[5])).props.const_symbol = 1;
	(*((*TimeData).Date[2])).symbol = '/';
	(*((*TimeData).Date[2])).props.const_symbol = 1;
}

void TimeEdit() {
	
	clrInd();
	flags.isTimeSetting = 1;
    
	long int temp = Clock / 6000;
	long int n = temp;
    
    TimeEditData TimeData;
	SymbolData _time[5];
	SymbolData _weekday[1];
	SymbolData _date[10];
	
	for(unsigned char i = 0; i < 5; i++){
		TimeData.Time[i] = &(_time[i]);
	}
	for(unsigned char i = 0; i < 1; i++){
		TimeData.WeekDay[i] = &(_weekday[i]);
	}
	for(unsigned char i = 0; i < 10; i++){
		TimeData.Date[i] = &(_date[i]);
	}
	
	TimeData_INIT(&TimeData);
	
	
    (*(TimeData.Time[4])).number = temp % 10;
	temp /= 10;
	(*(TimeData.Time[3])).number = temp % 6;
	
	temp /= 6;
	temp %= 24;
	
	(*(TimeData.Time[1])).number = temp % 10;
	(*(TimeData.Time[0])).number = temp / 10;
	
	(*(TimeData.WeekDay[0])).number = 1 + n / 1440;

	(*(TimeData.Date[9])).number = cYears % 10;
	(*(TimeData.Date[8])).number = cYears / 10;
	(*(TimeData.Date[7])).symbol = '0';
	(*(TimeData.Date[7])).props.const_symbol = 1;
	(*(TimeData.Date[6])).symbol = '2';
	(*(TimeData.Date[6])).props.const_symbol = 1;
	
	(*(TimeData.Date[4])).number = cMonths % 10;
	(*(TimeData.Date[3])).number = cMonths / 10;
	
	(*(TimeData.Date[1])).number = cDays % 10;
	(*(TimeData.Date[0])).number = cDays / 10;
	
	
	SymbolData *symb_array [11];
	
	symb_array[0] = TimeData.Date[0];	// day_H
	symb_array[1] = TimeData.Date[1];	// day_L
	symb_array[2] = TimeData.Date[3];	// month_H
	symb_array[3] = TimeData.Date[4];	// month_L
	symb_array[4] = TimeData.Date[8];	// year_H
	symb_array[5] = TimeData.Date[9];	// year_L
	
	symb_array[6] = TimeData.WeekDay[0];// day_of_week
	
	symb_array[7] = TimeData.Time[0];	// hour_H
	symb_array[8] = TimeData.Time[1];	// hour_L
	symb_array[9] = TimeData.Time[3];	// minute_H
	symb_array[10] = TimeData.Time[4];	// minute_L	
	
	
	char blinking = 7;
	char input = 255;
	while(1){
		(*symb_array[blinking]).props.blink = 1;
		
		ReIndTimeEdit(&TimeData);
		
		input = entNum(0, 9);
		
		if(input != 255){
			(*symb_array[blinking]).number = input;
			TimeData_BlinkOFF(&TimeData);
			blinking == 10 ? blinking = 0 : blinking++;
		}
		
		if(KeyCode == 40){
			KeyCode = 0;
			TimeData_BlinkOFF(&TimeData);
			blinking == 10 ? blinking = 0 : blinking++;
		}else if(KeyCode == 41){
			KeyCode = 0;
			TimeData_BlinkOFF(&TimeData);
			blinking == 0 ? blinking = 10 : blinking--;
		}else if(KeyCode == 42){
			KeyCode = 0;
			unsigned char t;
			do{
				t = (*symb_array[2]).number * 10 + (*symb_array[3]).number;
				if(t > 12){
					TimeData_BlinkOFF(&TimeData);
					blinking = 2;
					break;
				}
				
				unsigned char month = t;
				
				unsigned char MonthDays [] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
				t = (*symb_array[4]).number * 10 + (*symb_array[5]).number;
				if(t % 4 == 0){
					MonthDays[1] = 29;
				}
				t = (*symb_array[0]).number * 10 + (*symb_array[1]).number;
				if(MonthDays[month-1] < t){
					(*symb_array[0]).number = MonthDays[month-1] / 10;
					(*symb_array[1]).number = MonthDays[month-1] % 10;
					TimeData_BlinkOFF(&TimeData);
					blinking = 0;
					break;
				}
				
				t = (*symb_array[6]).number;
				if(t > 7){
					TimeData_BlinkOFF(&TimeData);
					blinking = 6;
					break;
				}
				
				t = (*symb_array[7]).number * 10 + (*symb_array[8]).number;
				if(t > 23){
					TimeData_BlinkOFF(&TimeData);
					blinking = 7;
					break;
				}
				
				t = (*symb_array[9]).number * 10 + (*symb_array[10]).number;
				if(t > 59){
					TimeData_BlinkOFF(&TimeData);
					blinking = 9;
					break;
				}
				
				t = 0;
				
				Clock = ((long int)((*symb_array[6]).number - 1) * 86400
						+ (long int)((*symb_array[7]).number * 10 + (*symb_array[8]).number) * 3600
						+ (long int)((*symb_array[9]).number * 10 + (*symb_array[10]).number) * 60) * 100;

				cDays = (*symb_array[0]).number * 10 + (*symb_array[1]).number;
				cMonths = (*symb_array[2]).number * 10 + (*symb_array[3]).number;
				cYears = (*symb_array[4]).number * 10 + (*symb_array[5]).number;
				/*
				WriteTime(Clock, cDays, cMonths, cYears);
				*/
				
			}while(0);
			
			if(t == 0){
				clrInd();
				break;
			}
		}else if(KeyCode == 43){
			KeyCode = 0;
			clrInd();
			break;
		}
	}
}

unsigned long int ReadFourBytesEE(unsigned int adress) {
	unsigned long int x;
	for (int i = 3; i >= 0; i--) {
		x = x * 256 + EERD(adress + i);
	}
	return x;
}

void FillArrayFromEEPROM(unsigned char *container, unsigned int adress_start, unsigned int quantity) {
	for (unsigned char i = 0; i < quantity; i++) {
		container[i] = EERD(adress_start + i);
	}
}

void FillPhonebookCellFromEEPROM(Phone *container, unsigned int adress_start) {
	FillArrayFromEEPROM((*container).name, adress_start, sizeof((*container).name));
	adress_start += sizeof((*container).name);
	FillArrayFromEEPROM((*container).phone, adress_start, sizeof((*container).phone));
	adress_start += sizeof((*container).phone);
	FillArrayFromEEPROM((*container).blocked, adress_start, sizeof((*container).blocked));
	adress_start += sizeof((*container).blocked);
	FillArrayFromEEPROM((*container).filled, adress_start, sizeof((*container).filled));
}

void FillArrayFromEEPROM_interr(unsigned char * container, unsigned int adress_start, unsigned int quantity) {
	for (unsigned char i = 0; i < quantity; i++) {
		container[i] = EERD(adress_start + i);
	}
}

void WriteFourBytesEE(unsigned int adress, unsigned long int data) {
	for (int i = 0; i < 4; i++) {
		EEWR(adress + i, data % 256);
		data /= 256;
	}
}

void WriteArrayToEEPROM(unsigned char * container, unsigned int adress_start, unsigned int quantity) {
	for (unsigned char i = 0; i < quantity; i++) {
		EEWR(adress_start + i, container[i]);
	}
}

void WritePhonebookCellToEEPROM(Phone *container, unsigned int adress_start) {
	WriteArrayToEEPROM((*container).name, adress_start, sizeof((*container).name));
	adress_start += sizeof((*container).name);
	WriteArrayToEEPROM((*container).phone, adress_start, sizeof((*container).phone));
	adress_start += sizeof((*container).phone);
	WriteArrayToEEPROM((*container).blocked, adress_start, sizeof((*container).blocked));
	adress_start += sizeof((*container).blocked);
	WriteArrayToEEPROM((*container).filled, adress_start, sizeof((*container).filled));
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
	PORTA = (0b11110000 & Symb) / 8 | ((flags.LCD_Light_On == 1 || flags.ActiveCall == 1) ? 0b00100000 : 0b00000000);
	E = 1;
	delay(10, 1);
	E = 0;
	RS = 0;
	delay(25, 5);

	RS = 1;
	PORTA = (0b11110000 & (Symb * 16)) / 8 | ((flags.LCD_Light_On == 1 || flags.ActiveCall == 1) ? 0b00100000 : 0b00000000);
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
	PORTA = (0b00011110 & (data * 2)) | (flags.LCD_Light_On == 1 || flags.ActiveCall == 1 ? 0b00100000 : 0b00000000);
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
	__delay_ms(10);
	__delay_ms(10);
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
		case 0: return 39;
		case 1: return 43;
		case 2: return 42;
		case 3: return 41;
		case 4: return 40;
		case 5: return 45;
		case 6: return 33;
		case 7: return 36;
		case 8: return 31;
		case 9: return 32;
		case 10: return 35;
		case 11: return 34;
		case 12: return 38;
		case 13: return 30;
		case 14: return 44;
		case 15: return 37;
		default: return 0;
	}
}

unsigned int FindCell(unsigned int adressStart, char New, unsigned int previous) {
	
	if (adressStart == END_OF_CELLS){
		adressStart = LAST_CELL;
	}
	unsigned int adress = adressStart;
	unsigned int adressNew = END_OF_CELLS;
	do {

		if (!previous) {
			adress == LAST_CELL ? adress = 0 : adress += CELL_CAPACITY;
		} else {
			adress == 0 ? adress = LAST_CELL : adress -= CELL_CAPACITY;
		}
		
		unsigned long int Data = 0;
		unsigned char CellsData [CELL_CAPACITY];
		ReadDataOfCell(&CellsData, &Data, adress);
		unsigned char CellIsEmpty = 1;
		for(unsigned char i = 0; i < CELL_CAPACITY; i++){
			if(CellsData[i] != 0xFF) {
				CellIsEmpty = 0;
			}
		}

		if (New != 1 ? CellIsEmpty == 0 : CellIsEmpty == 1) {
			adressNew = adress;
			break;
		}
	} while (adress != adressStart);
	return adressNew;
}
 
unsigned int RefreshSchedulerIndicator(unsigned int adress, char New, unsigned int CopyFrom) {
    
    unsigned int cell = 0;
    
	unsigned long int Data = 0;
	unsigned char CellsData [CELL_CAPACITY];
	ReadDataOfCell(&CellsData, &Data, adress);
	unsigned char CellIsEmpty = 1;
	for(unsigned char i = 0; i < CELL_CAPACITY; i++){
		if(CellsData[i] != 0xFF) {
			CellIsEmpty = 0;
		}
	}
	
	if (adress != END_OF_CELLS && (New != 1 ? CellIsEmpty == 0 : CellIsEmpty == 1)) {
		cell = adress / CELL_CAPACITY + 1;
	} else {
		adress = FindCell(adress, New, 0);
		if (adress != END_OF_CELLS) {
        	cell = adress / CELL_CAPACITY + 1;
		}
	}
    
    if(cell != 0){
        unsigned char num [] = "  ";
        NumericToString(cell, num, sizeof(num));
        OutputString(num, 0, 14, 0);
    }else{
        OutputString("--", 0, 14, 0);
    }
    
	if (New == 1) {
		if (CopyFrom != END_OF_CELLS) {
            OutputString("���������� �", 1, 0, 0);
			unsigned char num [] = "  ";
			NumericToString(1 + CopyFrom / CELL_CAPACITY, num, sizeof(num));
			OutputString(num, 1, 14, 0);
		}
        OutputString("����� ", 0, 0, 0);
	}
    OutputString("������ ", 0, 6, 0);
    
	return adress;
}

void TimesToTimeData(unsigned int * Times, unsigned char * TimeData) {
	unsigned int temp;
	for (char j = 0; j < 2; j++) {
		temp = Times[j];
        char d = 4 * j;
		TimeData[3 + d] = temp % 10;
		temp /= 10;
		TimeData[2 + d] = temp % 6;
		temp = temp / 6;
		TimeData[1 + d] = temp % 10;
		TimeData[0 + d] = temp / 10;
	}
}

void TimeDataToString(unsigned char * TimeData, unsigned char * string) {
    unsigned char j = 0;
	for (unsigned char i = 0; i < 12; i++) {
        if(string[i] == '*'){
            string[i] = getNumChar(TimeData[j]);
            j++;
        }
	}
}

void ItemsToIndicator(unsigned char items, unsigned char max_i, unsigned char firstDigit) {
	for (char i = 1; i < max_i; i++) {
		items % 2 == 1 ? setDigit(0, 16 -(firstDigit - i), getNumChar(i)) : setDigit(0, 16 - (firstDigit - i), '-');
		items /= 2;
	}
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

void ReadDataOfCell(unsigned char *CellsData, unsigned long int *Data, unsigned int TargetAdress){
	FillArrayFromEEPROM(CellsData, TargetAdress, CELL_CAPACITY);
	for(int i = 3; i >= 0; i--){
		*Data = *Data * 256 + CellsData[i];
	}
}

void ReadDataOfCell_interr(unsigned char *CellsData, unsigned long int *Data, unsigned int TargetAdress){
	FillArrayFromEEPROM_interr(CellsData, TargetAdress, CELL_CAPACITY);
	for(int i = 3; i >= 0; i--){
		*Data = *Data * 256 + CellsData[i];
	}
}

void WriteDataOfCell(unsigned char *CellsData, unsigned long int *Data, unsigned int TargetAdress){
	for(unsigned char i = 0; i < 4; i++){
		CellsData[i] = (unsigned char)(*Data % 256);
		*Data /= 256;
	}
	WriteArrayToEEPROM(CellsData, TargetAdress, CELL_CAPACITY);
}

void CleanStringArray(unsigned char *myArray, unsigned int size, unsigned char settingData){
	for(unsigned int i = 0; i < size; i++){
		*(myArray + i) = settingData;
	}
}

void _CleanStringArray_interrupt(unsigned char *myArray, unsigned int size, unsigned char settingData){
	for(unsigned int i = 0; i < size; i++){
		*(myArray + i) = settingData;
	}
}

void DeleteDataOfCell(unsigned int TargetAdress){
	unsigned char CellsData [CELL_CAPACITY];
	CleanStringArray(CellsData, sizeof(CellsData), 0xFF);
	WriteArrayToEEPROM(CellsData, TargetAdress, CELL_CAPACITY);
}

unsigned char EditSchedule(unsigned int adress, unsigned int SourceOfRecord) {
	/*
	unsigned int TargetAdress;
	if (SourceOfRecord == END_OF_CELLS) {
		TargetAdress = adress;
	} else {
		TargetAdress = SourceOfRecord;
	}
	
	unsigned long int Data = 0;
	unsigned char CellsData [CELL_CAPACITY];
	ReadDataOfCell(&CellsData, &Data, TargetAdress);
	
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
    
	clrInd();
    
	char mode = 1;
	unsigned char key;
    
	SymbolData _times[11];
	_times[]
	
	SymbolData *Times[8];
	
    while (1) {
        if (KeyCode == 43) { // Cancel
            KeyCode = 0;
            clrInd();
            return 0;
        } else if (KeyCode == 42) { // Enter
            KeyCode = 0;
            Data = (unsigned long int) Signals * 0x20000000 + (unsigned long int) Days * 0x400000 + (unsigned long int) Times[1]*0x800 + (unsigned long int) Times[0];
			WriteDataOfCell(&CellsData, &Data, adress);
            clrInd();
            return 1;
        }    
        
        switch (mode) {
            case 1:
            {
                unsigned char string [] = "**:**-**:**";
                TimeDataToString(TimeData, string);
                OutputString(string, 0, 5, 0);
                
                if (KeyCode == 45 || BeginEditTimes == 1) {
                    KeyCode = 0;
                    BeginEditTimes = 0;
                    char n = 5;
                    setBlink(0, n, 1);
                    unsigned char BorderUp = 2;
                    while (1) {
                        if (KeyCode == 43) { // Cancel
                            KeyCode = 0;
                            clrInd();
                            break;
                        } else if (KeyCode == 42) { // Enter
                            KeyCode = 0;
                            unsigned int T[2];
                            for (char j = 0; j < 2; j++) {
                                char f = j * 4;
                                T[j] = TimeData[f + 0] * 600
                                        + TimeData[f + 1] * 60
                                        + TimeData[f + 2] * 10
                                        + TimeData[f + 3];
                            }
                            Times[0] = T[0];
                            if (T[0] < T[1]) {
                                Times[1] = T[1];
                                clrInd();
                                break;
                            } else {
                                setBlink(0, n, 0);
                                n = 11;
                                BorderUp = getBorderUp(n);
                                Times[1] = T[0];
                                TimesToTimeData(Times, TimeData);
                                setBlink(0, n, 1);
                            }
                        } else if (KeyCode == 41) { // NextDig
                            KeyCode = 0;
                            if (n > 5) {
                                setBlink(0, n, 0);
                                (n == 8 || n == 11 || n == 14) ? n -= 2 : n--;
                                
                                setBlink(0, n, 1);
                            }
                        } else if (KeyCode == 40) { //PrevDig
                            KeyCode = 0;
                            if (n < 15) {
                                setBlink(0, n, 0);
                                n == 6 || n == 9 || n == 12 ? n += 2 : n++;
                                BorderUp = getBorderUp(n);
                                setBlink(0, n, 1);
                            }
                        }
                        
                        unsigned char string2 [] = "**:**-**:**";
                        TimeDataToString(TimeData, string2);
                        OutputString(string2, 0, 5, 0);
                        
                        char d = 0;
                        switch(n){
                            case 5: d = 0; break;
                            case 6: d = 1; break;
                            case 8: d = 2; break;
                            case 9: d = 3; break;
                            case 11: d = 4; break;
                            case 12: d = 5; break;
                            case 14: d = 6; break;
                            case 15: d = 7; break;
                        }
                        
                        key = entNum(0, BorderUp);
                        if (key != 255) {
                            TimeData[d] = key;
                            if ((d == 0 || d == 4) && key == 2 && (TimeData[d+1] > 3)){
                                TimeData[d+1] = 3;
                            }
                            if (n > 1) {
                                KeyCode = 40;
                            } else {
                                KeyCode = 42;
                            }
                        }
                    }
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
                }
            }
        }       
        if (KeyCode == 40) {
            KeyCode = 0;
            clrInd();
            if(mode == 3){
                mode = 1;
            }else{
                mode++;
            }
        } else if (KeyCode == 41) {
            KeyCode = 0;
            clrInd();
            if(mode == 1){
                mode = 3;
            }else{
                mode--;
            }
        }
    }
    return 0;*/
}

void Scheduler(unsigned int StartFrom) {
	clrInd();
	volatile static unsigned int NumberOfCells = 0;
	if (StartFrom != END_OF_CELLS) {
		NumberOfCells = StartFrom;
	}
	
	while (1) {
		NumberOfCells = RefreshSchedulerIndicator(NumberOfCells, 0, END_OF_CELLS);
		
		if (NumberOfCells != END_OF_CELLS && KeyCode == 44  && Select_OK_NO("������� ������?")) { // Delete
			KeyCode = 0;
			DeleteDataOfCell(NumberOfCells);
		} else if (KeyCode == 35) { // New
			KeyCode = 0;
			SchedulerNew(END_OF_CELLS);
		} else if (NumberOfCells != END_OF_CELLS && KeyCode == 33) { // New copy
			KeyCode = 0;
			SchedulerNew(NumberOfCells);
		} else if (NumberOfCells != END_OF_CELLS && KeyCode == 45) { // Edit
			KeyCode = 0;
			EditSchedule(NumberOfCells, END_OF_CELLS);
		} else if (KeyCode == 40) { // Next
			KeyCode = 0;
			NumberOfCells = FindCell(NumberOfCells, 0, 0);
		} else if (KeyCode == 41) { // Prev
			KeyCode = 0;
			NumberOfCells = FindCell(NumberOfCells, 0, 1);
		} else if (KeyCode == 43) { // Exit
			KeyCode = 0;
			clrInd();
			return;
		} else if (KeyCode == 30) { // Adress = 0
			KeyCode = 0;
			NumberOfCells = 0;
		}
	}
}

void SchedulerNew(unsigned int SourceOfRecord) {
	clrInd();
	volatile static unsigned int NumberOfCells = 0;
	while (1) {
		NumberOfCells = RefreshSchedulerIndicator(NumberOfCells, 1, SourceOfRecord);
		if (NumberOfCells != END_OF_CELLS && KeyCode == 45) { // Edit
			KeyCode = 0;
			if (EditSchedule(NumberOfCells, SourceOfRecord) == 1) {
				NumberOfCells = FindCell(NumberOfCells, 1, 0);
			}
		} else if (KeyCode == 40) { // Next
			KeyCode = 0;
			NumberOfCells = FindCell(NumberOfCells, 1, 0);
		} else if (KeyCode == 41) { // Prev
			KeyCode = 0;
			NumberOfCells = FindCell(NumberOfCells, 1, 1);
		} else if (KeyCode == 43) { // Exit
			KeyCode = 0;
			clrInd();
			return;
		} else if (KeyCode == 30) { // Adress = 0
			KeyCode = 0;
			NumberOfCells = 0;
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
	//return 0;
	unsigned int adress = 0;
	unsigned int adressStart = END_OF_CELLS;
	unsigned char TimeIsFound = 0;
	unsigned long int LastFoundTimeStart;

	do {
		unsigned long int Data = 0;
		unsigned char CellsData [CELL_CAPACITY];
		ReadDataOfCell(&CellsData, &Data, adress);

		adress += CELL_CAPACITY;
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
				adressStart = adress - CELL_CAPACITY;
			}
		}
	} while (adress < END_OF_CELLS);

	if (TimeIsFound == 1) {
		*TimeFrom = LastFoundTimeStart;
	} else {
		*TimeFrom = 0;
	}
	return adressStart;
}

void SignalsOnOff() {
	if (!flags.TimeIsRead) return;
	if (!flags.LockSignals && CurrentReadingCell < END_OF_CELLS) {
		
		unsigned long int Data = 0;
		unsigned char CellsData [CELL_CAPACITY];
		ReadDataOfCell_interr(&CellsData, &Data, CurrentReadingCell);

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
				if (((ThisDay & Days) > 0) && (TimeStart <= ThisTime) && (TimeStop >= ThisTime)) {
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

Phone *FindPhoneContactByNumber(unsigned char *number){
	Phone container;

	unsigned char found = 0;
	
	for(unsigned char i = 0; i < PHONEBOOK_CAPACITY; i++){
		FillPhonebookCellFromEEPROM(&container, PHONEBOOK_START_ADRESS + sizeof(container) * i);
		if(container.filled[0] != 1){
			continue;
		}
		
		unsigned char end_of_phone = 0;
		while(container.phone[end_of_phone + 1] != '\0'){
			end_of_phone++;
		}
		
		for(unsigned char j = 0; j < 10 && (end_of_phone - j + 1) > 0; j++){
			if(container.phone[end_of_phone - j] != *(number + 9 - j)){
				found = 0;
				break;
			}else{
				found = 1;
			}
		}
		
		if(found){
			break;
		}
	}
	
	if(found){
		return &container;
	}else{
		return NULL;
	}
}

unsigned char *_getContainer_Clock_from_GSM(){
	CleanStringArray(Clock_from_GSM, sizeof(Clock_from_GSM), '\0');
	return Clock_from_GSM;
}

unsigned char *_getContainer_Incomming_Call_Data(){
	CleanStringArray(Incomming_Call_Data, sizeof(Incomming_Call_Data), '\0');
	return Incomming_Call_Data;
}

unsigned char *_getContainer_Incomming_DTMF_Data(){
	CleanStringArray(Incomming_DTMF_Data, sizeof(Incomming_DTMF_Data), '\0');
	return Incomming_DTMF_Data;
}

void ProcessIncommingUartData(){
	
	if(flags.UnprocessedIncommingUartData){
		flags.UnprocessedIncommingUartData = 0;
		
		if(_FindIncommingData_interrupt(StandardAnswer_OK, NULL, LAST_INCOMMING_DATA_INDEX)){
			// do nothing
		}else if(_FindIncommingData_interrupt("RING", NULL, LAST_INCOMMING_DATA_INDEX)){
			// do nothing
		}else if(_FindIncommingData_interrupt(StandardAnswer_CLOCK, _getContainer_Clock_from_GSM(), LAST_INCOMMING_DATA_INDEX)){
			
		}else if(_FindIncommingData_interrupt(StandardAnswer_INCCALL, _getContainer_Incomming_Call_Data(), LAST_INCOMMING_DATA_INDEX)){
			unsigned char number [11];
			CleanStringArray(number, sizeof(number), '\0');

			for(unsigned char i = 0; i < sizeof(number)-1; i++){
				number[i] = Incomming_Call_Data[i+11];
			}
			Phone *contact = FindPhoneContactByNumber(number);
			if(contact == NULL){
				SendCommandToUSART("ATH", 0);
				flags.ActiveCall = 0;
			}else{
		//		OutputSystemMessage((*contact).name);
				SendCommandToUSART("ATA", 0);
				flags.ActiveCall = 1;
			}
		}else if(_FindIncommingData_interrupt(StandardAnswer_DTMF, _getContainer_Incomming_DTMF_Data(), LAST_INCOMMING_DATA_INDEX)){
			
			DTMF_Symbol = Incomming_DTMF_Data[sizeof(Incomming_DTMF_Data)-2];
			
			if(flags.RemoteControlIsEnabled){
				if (DTMF_Symbol == '4') {
					CurrentSignals = 0;
					flags.CurrentSignalsSetted = 1;
				} else if (DTMF_Symbol == '7') {
					CurrentSignals = CurrentSignals | 0b00000001;
					flags.CurrentSignalsSetted = 1;
				} else if (DTMF_Symbol == '8') {
					CurrentSignals = CurrentSignals | 0b00000010;
					flags.CurrentSignalsSetted = 1;
				} else if (DTMF_Symbol == '9') {
					CurrentSignals = CurrentSignals | 0b00000100;
					flags.CurrentSignalsSetted = 1;
				} else if (DTMF_Symbol == '0') {
					flags.LockSignals = 1;
					CurrentSignals = 0;
					EEWR(LOCK_SIGNALS_FLAG_CELL, flags.LockSignals);
					flags.CurrentSignalsSetted = 1;
				} else if (DTMF_Symbol == '5') {
					flags.LockSignals = 0;
					flags.CurrentSignalsSetted = 1;
					EEWR(LOCK_SIGNALS_FLAG_CELL, flags.LockSignals);
				}
			}else if(DTMF_Symbol == '*'){
				flags.RemoteControlIsEnabled = 1;
			}
			
			flags.ActiveCall = 1;
		}else if(  _FindIncommingData_interrupt(StandardAnswer_NO_CARRIER	, NULL, LAST_INCOMMING_DATA_INDEX)
				|| _FindIncommingData_interrupt(StandardAnswer_BUSY			, NULL, LAST_INCOMMING_DATA_INDEX)
				|| _FindIncommingData_interrupt(StandardAnswer_NO_ANSWER	, NULL, LAST_INCOMMING_DATA_INDEX)
				|| _FindIncommingData_interrupt(StandardAnswer_NO_DIALTONE	, NULL, LAST_INCOMMING_DATA_INDEX))
		{
			flags.ActiveCall = 0;
			flags.RemoteControlIsEnabled = 1;
		
		}else if(_FindIncommingData_interrupt(StandardAnswer_ERROR, NULL, LAST_INCOMMING_DATA_INDEX)){
			OutputSystemMessage("������ GSM");
		}

	}
}

void AddByteToUSARTbuff(unsigned char byte){
	
	static struct{
		unsigned is_r: 1;
		unsigned is_n: 1;
		unsigned is_rn: 1;
	} lastdata;
	
	if(byte == '\r'){
		if(!lastdata.is_rn){
			lastdata.is_r = 1;
		}
	}else if(byte == '\n' && lastdata.is_r){
		if(!lastdata.is_rn){
			lastdata.is_n = 1;
		}
	}else{
		if(lastdata.is_r || lastdata.is_n){
			lastdata.is_r = 0;
			lastdata.is_n = 0;
		}
		if(byte != '\n'){
			lastdata.is_rn = 0;
			for(unsigned int i = 0; i < BUFFER_STRING_LENGTH-2; i++){
				*(*(IncommingBuffer + MAX_INCOMMING_BUFF_INDEX) + i) = *(*(IncommingBuffer + MAX_INCOMMING_BUFF_INDEX) + i + 1);
			}
			*(*(IncommingBuffer + MAX_INCOMMING_BUFF_INDEX) + BUFFER_STRING_LENGTH-2) = byte;
		}
	}
	
	if(lastdata.is_r && lastdata.is_n){
		lastdata.is_r = 0;
		lastdata.is_n = 0;
		
		lastdata.is_rn = 1;
		
		unsigned char *buff = IncommingBuffer[0];
		for(unsigned char i = 0; i < MAX_INCOMMING_BUFF_INDEX; i++){
			IncommingBuffer[i] = IncommingBuffer[i+1];
		}
		IncommingBuffer[MAX_INCOMMING_BUFF_INDEX] = buff;
		_CleanStringArray_interrupt(IncommingBuffer[MAX_INCOMMING_BUFF_INDEX], BUFFER_STRING_LENGTH, '\0');
		
		flags.UnprocessedIncommingUartData = 1;
	}
}

void _load_TXREG(){
	if(OutcommingBuffer == NULL && TXSTAbits.TRMT){
		TXSTAbits.TXEN = 0;
		return;
	}
	if((*OutcommingBuffer) == '\0'){
		while(!TXSTAbits.TRMT);
		TXREG = 0x0D;
		TXREG = 0x0A;
		OutcommingBuffer = NULL;
	}else if(OutcommingBuffer != NULL){
		TXREG = *OutcommingBuffer++;
	}
}

void interrupt high_priority F_h() {
	ClrWdt();
	if(PIR1bits.RCIF == 1){
		AddByteToUSARTbuff(RCREG);
	}else if(PIR1bits.TXIF == 1){
		_load_TXREG();
	}
}

unsigned long int getSystemTimePoint(){
	return _systemCounter;
}

unsigned char testTimePoint(unsigned long int point, unsigned long int value){
	unsigned long int _point = getSystemTimePoint();
	if(_point < point){
		_point = _point + (0xFFFFFFF - point);
	}else{
		_point = _point - point;
	}
	return _point >= value;
}

void system_BlinkReset(unsigned char val){
	flags.IsLCDModified = 1;
	GlobalBlinkCycleTime = 100;
	flags.GlobalBlink = val;
}

void ProcessSystemMessageShowing(){
	if(flags.UnreadSystemMessage){
		KeyCode = 0;
		flags.UnreadSystemMessage = 0;
	}
}

void interrupt low_priority F_l() {
	ClrWdt();
	if (TMR1IF) {
		_systemCounter++;
		TMR1IF = 0;
		TMR1 += 51200;
		SignalsOnOff();
		char MinuteAgo = (Clock - Minutes > 6000);
		if (MinuteAgo > 0) {
			FillMinutes();
			flags.RelevanceOfNextStartCell = 0;
		}
		
		unsigned char *digs;
		digits_atributes *digsAtrib;
		
		if(flags.UnreadSystemMessage){
			digs = digits_system_message;
			digsAtrib = digitsAtrib_system_message;
		}else{
			digs = digits;
			digsAtrib = digitsAtrib;
		}
		
		if (flags.LCD_Power_On && (GlobalBlinkCycleTime == 100 || flags.IsLCDModified)) {
			for (char line = 0; line < 2; line++) {
				for (int symbol = 0; symbol < 16; symbol++) {
					if (GlobalBlinkCycleTime == 100 || (*(digsAtrib + 16 * line + symbol)).modifided > 0) {
						unsigned char digit = *(digs + 16 * line + symbol);
						lcd_send_byte((line * 0x40 + symbol) | 0b10000000); //SetDDRAM
						if (!((*(digsAtrib + 16 * line + symbol)).blink == 0 || (*(digsAtrib + 16 * line + symbol)).blink == 1 && flags.GlobalBlink)) {
							digit = '_';
						}
						SendSymbolToLCD(getLcdCodeOfChar(digit));
						(*(digsAtrib + 16 * line + symbol)).modifided = 0;
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
					ButtonPressTimeOut = 10;
					
					ProcessSystemMessageShowing();
				}
			}
		}
		
		ProcessIncommingUartData();
	}
	if(T0IF) {
		T0IF = 0;
	}
}

void Init_USART(unsigned char SpeedNumber){
	
 	unsigned int _value = 0;
	if(SpeedNumber == 1){			// 1200
		_value = 8332;
	}else if(SpeedNumber == 2){		// 2400
		_value = 4165;
	}else if(SpeedNumber == 4){		// 19200
		_value = 520;
	}else if(SpeedNumber == 5){		// 57600
		_value = 172;
	}else{							// 9600
		_value = 1040;
	}
	
	TRISCbits.RC6 = 1; //TX pin set as output
    TRISCbits.RC7 = 1; //RX pin set as input
	BAUDCONbits.BRG16 = 1;
	TXSTAbits.BRGH = 1;
	SPBRG = (unsigned char)(_value%256);
	SPBRGH = (unsigned char)(_value/256);
	
	IPR1bits.RCIP = 1; // ������� ��������� ���������� �� ��������� USART
	IPR1bits.TXIP = 1; // ������� ��������� ���������� �� ����������� USART
	
	TXSTAbits.SYNC = 0; // ����������� �����
	RCSTAbits.SPEN = 1; // uart on
	PIE1bits.RCIE = 1;  // ���������� �� ������ ��������
	PIE1bits.TXIE = 1;  // ���������� �� �������� ��������
	
	RCSTAbits.CREN = 1; // receiving enabled
	
}

void main() {

	flags.TimeIsRead = 0;
	flags.GSM_Connected = 0;

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

	CurrentSignals = 0;
	
	clrInd();

	RCONbits.IPEN = 1; // ��������� ��������������� ����������
	INTCONbits.PEIE = 1; // ������������ ���������� ���������
	
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

	Clock = ((long int) cMinutes * 60
		+ (long int) cHours * 3600
		+ ((long int) cWeekDay - 1) * 86400) * 100;
	
	LATA = 0b00000000;
	LATB = 0b00000000;
	LATC = 0b00000000;
	T2CON = 0b00100100;
	
	flags.LockSignals = EERD(LOCK_SIGNALS_FLAG_CELL);
	
	unsigned char i = 0;
	/*
	IncommingBuffer[i++] = buff_string_0;
	IncommingBuffer[i++] = buff_string_1;
	IncommingBuffer[i++] = buff_string_2;
	IncommingBuffer[i++] = buff_string_3;
	IncommingBuffer[i++] = buff_string_4;
	IncommingBuffer[i++] = buff_string_5;
	IncommingBuffer[i++] = buff_string_6;
	IncommingBuffer[i++] = buff_string_7;
	*/
	IncommingBuffer[i++] = buff_string_8;
	IncommingBuffer[i++] = buff_string_9;
	IncommingBuffer[i++] = buff_string_10;
	IncommingBuffer[i++] = buff_string_11;
	
	for(unsigned char i = 0; i < INCOMMING_BUFF_LENGTH; i++){
		CleanStringArray(IncommingBuffer[i], BUFFER_STRING_LENGTH, '\0');
	}

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
		case 10: return 'A';
		case 11: return 'B';
		case 12: return 'C';
		case 13: return 'D';
		case 14: return 'E';
		case 15: return 'F';
		default: return '?';
	}
}

void OutputString(unsigned char *stringData, unsigned char line, unsigned char position, unsigned char transfer_line) {
	unsigned int i = 0;
	while(*(stringData + i) != '\0'){
		unsigned char l = line;
		for(line = 0; l < 2; l++) {
			unsigned char s = position;
			for (position = 0; (s < 16) && (*(stringData + i) != '\0'); s++) {
				setDigit(l, s, *(stringData + i));
				i++;
			}
			if(!transfer_line){
				return;
			}
		}
	}
}

void OutputSystemMessage(unsigned char *stringData) {
	for (char line = 0; line < 2; line++) {
		for (char symb = 0; symb < 16; symb++) {
			*(digits_system_message + line * 16 + symb) = EMPTY_SYMBOL_VALUE;
			(*(digitsAtrib_system_message + line * 16 + symb)).blink = 0;
			(*(digitsAtrib_system_message + line * 16 + symb)).modifided = 1;
		}
	}
	unsigned int i = 0;
	while(*(stringData + i) != '\0'){
		for(unsigned char l = 0; l < 2; l++) {
			unsigned char s = 0;
			for (unsigned char position = 0; (s < 16) && (*(stringData + i) != '\0'); s++) {
				(*(digitsAtrib_system_message + 16 * l + s)).modifided = 1;
				*(digits_system_message + 16 * l + s) = *(stringData + i);
				i++;
			}
		}
	}
	flags.UnreadSystemMessage = 1;
	flags.IsLCDModified = 1;
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
		*(digits + destLine * 16 + symbol) = *(digits + srcLine * 16 + symbol);
		(*(digitsAtrib + destLine * 16 + symbol)).blink = (*(digitsAtrib + srcLine * 16 + symbol)).blink;
		(*(digitsAtrib + destLine * 16 + symbol)).modifided = 1;
	}
	
	flags.IsLCDModified = 1;
	
	unsigned char cutedString[17] = EMPTY_STRING_16;
	for(unsigned char symbol = 0; symbol < 16; symbol++) {
		unsigned char a = stringData[symbol + startNum];
		if(a == '\0'){
			break;
		}
		cutedString[symbol] = a;
	}
	
	OutputString(cutedString, (direction >= 0 ? 1 : 0), 0, 1);
}

unsigned char FindIncommingData(unsigned char *regexp, unsigned char *container, int history_index){
	
	unsigned char *pointer;
	/*
	unsigned char *end_of_regexp = regexp;
	while(*(end_of_regexp+1) != '\0'){
		end_of_regexp++;
	}
	*/
	unsigned char *end_of_regexp = GetStringLength(regexp) - 1 + regexp;
	
	unsigned int start_index, finish_index;
	if(history_index >= 0){
		start_index = history_index;
		finish_index = history_index;
	}else{
		start_index = 0;
		finish_index = MAX_INCOMMING_BUFF_INDEX-1;
	}
	
	for(unsigned int i = start_index; i <= finish_index; i++){
		pointer = IncommingBuffer[i];
		unsigned int last_s, not_found = 0;
		for(unsigned int j = 0; end_of_regexp - j >= regexp && j < BUFFER_STRING_LENGTH-1; j++){
			unsigned int s0 = *(end_of_regexp - j);
			last_s = BUFFER_STRING_LENGTH - 2 - j;
			unsigned int s1 = *(pointer + last_s);
			if(s0 != s1 && s0 != '\x10'){
				not_found = 1;
				break;
			}
		}
		if(!not_found){ // if regexp is found copy data to container
			if(container != NULL){
				for(unsigned int a = 0; last_s < BUFFER_STRING_LENGTH - 1;){
					*(container + a) = *(pointer + last_s);
					 a++;
					 last_s++;
				}
			}
			CleanStringArray(pointer, BUFFER_STRING_LENGTH, '\0');
			return 1;
		}
	}
	return 0;
}

unsigned int GetStringLength(unsigned char *container){
	unsigned int i = 0;
	while(*(container) != '\0'){
		i++;
		container++;
	}
	return i;
}

unsigned int _GetStringLength_interr(unsigned char *container){
	unsigned int i = 0;
	while(*(container) != '\0'){
		i++;
		container++;
	}
	return i;
}

unsigned char _FindIncommingData_interrupt(unsigned char *regexp, unsigned char *container, int history_index){
	
	unsigned char *pointer;
	
	unsigned char *end_of_regexp = _GetStringLength_interr(regexp) - 1 + regexp;
	
	unsigned int start_index, finish_index;
	if(history_index >= 0){
		start_index = history_index;
		finish_index = history_index;
	}else{
		start_index = 0;
		finish_index = LAST_INCOMMING_DATA_INDEX;
	}
	
	for(unsigned int i = start_index; i <= finish_index; i++){
		pointer = IncommingBuffer[i];
		unsigned int last_s, not_found = 0;
		for(unsigned int j = 0; end_of_regexp - j >= regexp && j < BUFFER_STRING_LENGTH-1; j++){
			unsigned int s0 = *(end_of_regexp - j);
			last_s = BUFFER_STRING_LENGTH - 2 - j;
			unsigned int s1 = *(pointer + last_s);
			if(s0 != s1 && s0 != '\x10'){
				not_found = 1;
				break;
			}
		}
		if(!not_found){ // if regexp is found copy data to container
			if(container != NULL){
				for(unsigned int a = 0; last_s < BUFFER_STRING_LENGTH - 1;){
					*(container + a) = *(pointer + last_s);
					 a++;
					 last_s++;
				}
				CleanStringArray(pointer, BUFFER_STRING_LENGTH, '\0');
			}
			return 1;
		}
	}
	return 0;
}

void _setInputTextProps(input_text_props *_props){
	clrInd();
	
	input_text_props props;
	props.Aa = (*_props).Aa;
	props.Language = (*_props).Language;
	
	const unsigned char source_A [] = " �� �� ��";
	const unsigned char source_L [] = "English�������    123";
	unsigned char Aa [] = "   ";
	unsigned char Lang [] = "       ";
	while(1){
		
		for(unsigned char i = 0; i < 3; i++){
			Aa[i] = source_A[i + 3 * props.Aa];
		}
		
		for(unsigned char i = 0; i < 7; i++){
			Lang[i] = source_L[i + 7 * props.Language];
		}
		
		OutputString(Aa, 0, 2, 0);
		OutputString(Lang, 0, 7, 0);
		
		if (KeyCode == 35){
			KeyCode = 0;
			props.Aa < 2 ? props.Aa++ : props.Aa = 0;
		}else if (KeyCode == 36){
			KeyCode = 0;
			props.Language < 2 ? props.Language++ : props.Language = 0;
		}else if (KeyCode == 38){
			KeyCode = 0;
			props.Aa > 0 ? props.Aa-- : props.Aa = 2;
		}else if (KeyCode == 39){
			KeyCode = 0;
			props.Language > 0 ? props.Language-- : props.Language = 2;
		}else if (KeyCode == 42){ // enter
			KeyCode = 0;
			(*_props).Aa = props.Aa;
			(*_props).Language = props.Language;
			clrInd();
			break;
		}else if (KeyCode == 43){ // cancel
			KeyCode = 0;
			clrInd();
			break;
		}		
	}
}

unsigned char *InputText(unsigned char *text, input_text_props *_props, unsigned char capacity){
	clrInd();
	
	input_text_props props;
	props.Aa = AA;
	props.Language = English;
	
	if(_props != NULL){
		props.Aa = (*_props).Aa;
		props.Language = (*_props).Language;
	}
	
	unsigned char *symbols [30];
	
	unsigned char s_0 [] = " 0";			symbols[0] = s_0;
	unsigned char s_1 [] = "-=?+;:().,*!<>~/\"";
											symbols[1] = s_1;
	unsigned char s_2 [] = "abc2";			symbols[2] = s_2;
	unsigned char s_3 [] = "def3";			symbols[3] = s_3;
	unsigned char s_4 [] = "ghi4";			symbols[4] = s_4;
	unsigned char s_5 [] = "jkl5";			symbols[5] = s_5;
	unsigned char s_6 [] = "mno6";			symbols[6] = s_6;
	unsigned char s_7 [] = "pqrs7";			symbols[7] = s_7;
	unsigned char s_8 [] = "tuv8";			symbols[8] = s_8;
	unsigned char s_9 [] = "wxyz9";			symbols[9] = s_9;
		
											symbols[10] = s_0;
											symbols[11] = s_1;
	unsigned char s_12 [] = "����2";		symbols[12] = s_12;
	unsigned char s_13 [] = "����3";		symbols[13] = s_13;
	unsigned char s_14 [] = "����4";		symbols[14] = s_14;
	unsigned char s_15 [] = "����5";		symbols[15] = s_15;
	unsigned char s_16 [] = "����6";		symbols[16] = s_16;
	unsigned char s_17 [] = "����7";		symbols[17] = s_17;
	unsigned char s_18 [] = "����8";		symbols[18] = s_18;
	unsigned char s_19 [] = "����9";		symbols[19] = s_19;
	
	unsigned char s_20 [] = "0+";			symbols[20] = s_20;
	unsigned char s_21 [] = "1";			symbols[21] = s_21;
	unsigned char s_22 [] = "2";			symbols[22] = s_22;
	unsigned char s_23 [] = "3";			symbols[23] = s_23;
	unsigned char s_24 [] = "4";			symbols[24] = s_24;
	unsigned char s_25 [] = "5";			symbols[25] = s_25;
	unsigned char s_26 [] = "6";			symbols[26] = s_26;
	unsigned char s_27 [] = "7";			symbols[27] = s_27;
	unsigned char s_28 [] = "8";			symbols[28] = s_28;
	unsigned char s_29 [] = "9";			symbols[29] = s_29;
	
	if(capacity == 0){
		capacity = 31;
	}else{
		capacity--;
	}
	
	SymbolData _input_data [33];
	SymbolData *InputData [33];
	for(unsigned char i = 0; i < 33; i++){
		_input_data[i].index = i;
		_input_data[i].props.blink = 0;
		_input_data[i].props.const_symbol = 1;
		_input_data[i].number = '\0';
		_input_data[i].symbol = ' ';
		InputData[i] = &(_input_data[i]);
	}
	_input_data[32].symbol = '\0';
	
	unsigned char active_cell = 0;
	unsigned char lastKeyCode = 0;
	unsigned long int time_point = 0;
	unsigned char *cur_char;
	
	if(text != NULL){
		unsigned char i = 0;
		for(; *(text + i) != '\0'; i++){
			_input_data[i].symbol = *(text + i);
			_input_data[i].number = 1;
			active_cell = i;
		}
		if(active_cell < capacity){
			active_cell++;
		}
	}
	
	while(1){
		if(lastKeyCode != 0 && testTimePoint(time_point, 300)){
			lastKeyCode = 0;
			if(active_cell < capacity && (*InputData[capacity]).number == '\0'){
				active_cell++;
			}
		}
		
		if(KeyCode >= 30 && KeyCode <= 39){
			
			if(lastKeyCode == KeyCode){
				if(*(cur_char+1) == '\0'){
					cur_char = symbols[((KeyCode-30) + 10 * props.Language)];
				}else{
					cur_char++;
				}
			}else{
				cur_char = symbols[((KeyCode-30) + 10 * props.Language)];
			}
			
			SymbolData *temp;
			if(lastKeyCode != KeyCode && (*InputData[capacity]).number == '\0'){
				if(active_cell < capacity && lastKeyCode != 0){
					active_cell++;
				}
				temp = InputData[capacity];
				for(unsigned char i = capacity; i > active_cell; i--){
					InputData[i] = InputData[i-1];
					(*InputData[i]).index++;
				}
				InputData[active_cell] = temp;
			}else{
				temp = InputData[active_cell];
			}
			
			(*temp).symbol = *cur_char;
			if(props.Aa == 1 && active_cell == 0 || props.Aa == 2){
				(*temp).symbol = CheckoutRegisterOfSymbol((*temp).symbol);
			}
			(*temp).index = active_cell;
			(*temp).number = 1;
			
			lastKeyCode = KeyCode;
			KeyCode = 0;
			system_BlinkReset(1);
			
			time_point = getSystemTimePoint();
		}
		InteractiveData_BlinkOFF(InputData, 32);
		(*InputData[active_cell]).props.blink = 1;
		
		OutputInteractiveData(InputData, 32, 0);
		
		if (KeyCode == 40){ // next
			KeyCode = 0;
			if(active_cell < capacity && (*InputData[active_cell]).number != '\0'){
				active_cell++;
				system_BlinkReset(0);
				
				lastKeyCode = 0;
			}
		}else if (KeyCode == 41){ // prev
			KeyCode = 0;
			if(active_cell > 0){
				active_cell--;
				system_BlinkReset(0);
				
				lastKeyCode = 0;
			}
		}else if (KeyCode == 42){ // enter
			KeyCode = 0;
			unsigned char ret_string [33];
			unsigned char i = 0;
			for(; i < 33; i++){
				if((*InputData[i]).number == '\0'){
					break;
				}
				ret_string[i] = (*InputData[i]).symbol;
			}
			ret_string[i] = '\0';
			clrInd();
			return ret_string;
		}else if (KeyCode == 43){ // cancel
			KeyCode = 0;
			clrInd();
			return NULL;
		}else if (KeyCode == 44){ // backspace
			KeyCode = 0;
			if(lastKeyCode != 0){
				active_cell++;
			}
			if(active_cell > 0){
				SymbolData *temp = InputData[active_cell-1];
				(*temp).number = '\0';
				(*temp).symbol = ' ';
				(*temp).index = 31;
				for(unsigned char i = active_cell-1; i < 32; i++){
					InputData[i] = InputData[i+1];
					(*InputData[i]).index--;
				}
				InputData[31] = temp;
				active_cell--;
				
				lastKeyCode = 0;
			}
		}else if (KeyCode == 45){ // properties
			KeyCode = 0;
			_setInputTextProps(&props);
		}
	}
}

void SendCommandToUSART(unsigned char *command, unsigned char wait){
	
	if(command != NULL){
		OutcommingBuffer = command;
		TXSTAbits.TXEN = 1;
		if(wait){
			while(OutcommingBuffer != NULL);
		}
	}
	
}

void WorkingWithGSM(){
	clrInd();

	unsigned char myOutputString [33];
	myOutputString[32] = '\0';
	static unsigned char line = MAX_INCOMMING_BUFF_INDEX - 1;
	
	while(1){

		unsigned char *p = IncommingBuffer[line] + BUFFER_STRING_LENGTH - 33;
		for(unsigned char i = 0; i < 32; i++){
			*(myOutputString + i) = *p++;
			if(*(myOutputString + i) == '\0'){
				*(myOutputString + i) = EMPTY_SYMBOL_VALUE;
			}
		}
		
		OutputString(myOutputString, 0, 0, 1);

		if (KeyCode == 33){
			KeyCode = 0;
			unsigned char speed = USART_Speed_Choise();
			if(speed != 0){
				Save_USART_SpeedNumber(speed);
				Init_USART(speed);
			}
		}else if (KeyCode == 45){
			KeyCode = 0;
			unsigned char *command = InputText(NULL, NULL, 0);
			SendCommandToUSART(command, 0);
		}else if (KeyCode == 40){
			KeyCode = 0;
			if(line > 0){
				line--;
			}
		}else if (KeyCode == 41){
			KeyCode = 0;
			if(line < MAX_INCOMMING_BUFF_INDEX){
				line++;
			}
		}else if (KeyCode == 43){
			KeyCode = 0;
			clrInd();
			break;
		}
	}
}

void testSystemTimer(){
	clrInd();
	
	unsigned char string [] = EMPTY_STRING_16;
	while(1){
		
		unsigned long int a = getSystemTimePoint();
		for(unsigned char i = 0; i < sizeof(string)-1; i++){
			string[i] = ' ';
		}
		NumericToString(a, string, sizeof(string));
		OutputString(string, 1, 0, 0);
		if (KeyCode == 43) {
			KeyCode = 0;
			clrInd();
			break;
		}
	}
}

void UserDelay(unsigned int value){
	unsigned long int timestamp = getSystemTimePoint();
	while(!testTimePoint(timestamp, value));
}

unsigned char Init_GSM(unsigned char show){
	
	do{
		flags.GSM_Connected = 0;
		
		Init_USART(Read_USART_SpeedNumber());
		
		if(show){
			clrInd();
			OutputSystemMessage("����� GSM-      ������...");
		}
		SendCommandToUSART("AT", 1);
		UserDelay(20);
		if(!FindIncommingData(StandardAnswer_OK, NULL, LAST_INCOMMING_DATA_INDEX)){
			if(show){
				UserDelay(200);
				clrInd();
				OutputSystemMessage("GSM-������ ��   ������");
				UserDelay(200);
			}
			break;
		}else{
			flags.GSM_Connected = 1;
		}
		//SendCommandToUSART("AT+IPR=9600", 1);
		//while(!FindIncommingData(StandardAnswer_OK, NULL, LAST_INCOMMING_DATA_INDEX));
		//Init_USART(19200);
		if(show){
			clrInd();
			OutputSystemMessage("��������� ������...");
		}
		SendCommandToUSART("ATE0", 1);
		while(!FindIncommingData(StandardAnswer_OK, NULL, LAST_INCOMMING_DATA_INDEX));
		OutputSystemMessage("��������� ������... OK");
		SendCommandToUSART("AT+DDET=1", 1);
		while(!FindIncommingData(StandardAnswer_OK, NULL, LAST_INCOMMING_DATA_INDEX));
		OutputSystemMessage("��������� ������... OK  OK");
		
		if(show){
			UserDelay(120);
			clrInd();
		}
		
		return 1;
		
	}while(0);
	
	if(show){
		clrInd();
	}
	return 0;
}

void TelephoneCall(unsigned char *phone_number){
	if(phone_number == NULL){
		return;
	}
	
	volatile unsigned char command[] = "ATD              ";
	unsigned char i;
	for(i = 0; *(phone_number + i) != '\0'; i++){
		command[i+3] = *(phone_number + i);
	}
	command[i+3] = ';';
	command[i+4] = '\0';
	SendCommandToUSART(command, 1);
	
}

unsigned char Select_OK_NO(unsigned char *text){
	
	clrInd();
	
	while (1) {
		
		OutputString(text, 0, 0, 1);
		
		if (KeyCode == 42) { // Enter
			KeyCode = 0;
			clrInd();
			return 1;
		} else if (KeyCode == 43) { // Cancel
			KeyCode = 0;
			clrInd();
			return 0;
		}
	}
}

unsigned char ShowMenu(unsigned char *menu[], unsigned char item){
	
	clrInd();
	
	static unsigned char line = 0;
	if(item > 0){
		item--;
	}
	if(item == 0){
		line = 0;
	}
	
	unsigned char result = 0;
	
	unsigned char item_max = 0;
	while(menu[item_max] != NULL){
		item_max++;		
	}
	
	item_max--;
	
	while (1) {
		
		if(line == 0){
			OutputString(menu[item], 0, 1, 0);
			if(item_max > item){
				OutputString(menu[item+1], 1, 1, 0);
			}
		}else if(line == 1){
			if(item > 0){
				OutputString(menu[item-1], 0, 1, 0);
			}
			OutputString(menu[item], 1, 1, 0);
		}
		
		OutputString("�", line, 0, 0);
	
		if (KeyCode == 42) { // Enter
			KeyCode = 0;
			result = item + 1;
			break;
		} else if (KeyCode == 43) { // Cancel
			KeyCode = 0;
			result = 0;
			break;
		} else if (KeyCode == 40) { // Up
			KeyCode = 0;
			if(item > 0){
				item--;
				if(line == 1){
					line = 0;
				}
			}
			clrInd();
		} else if (KeyCode == 41) { // Down
			KeyCode = 0;
			if(item < item_max){
				item++;
				if(line == 0){
					line = 1;
				}
			}
			clrInd();
		}
	}
	
	clrInd();
	return result;
}

void DialDTMF(){
	
}

void PhonebookEdit(){
	clrInd();
	
	Phone Contact;
	
	unsigned int cell = PHONEBOOK_START_ADRESS;
	
	unsigned char read = 0;
	
	while (1) {
		
		if(!read){
			FillPhonebookCellFromEEPROM(&Contact, cell);
			Contact.name[sizeof(Contact.name)-1] = '\0';
			Contact.phone[sizeof(Contact.phone)-1] = '\0';
			read = 1;
			if(Contact.filled[0] != 1){
				Contact.name[0] = '\0';
				Contact.name[sizeof(Contact.name)-1] = '\0';
				Contact.phone[0] = '\0';
				Contact.phone[sizeof(Contact.phone)-1] = '\0';
			}
		}
		
		unsigned char num_cell[] = "  ";
		NumericToString((cell - PHONEBOOK_START_ADRESS)/ sizeof(Contact) + 1, num_cell, sizeof(num_cell));
		OutputString(num_cell, 1, 14, 0);
		if(Contact.filled[0] == 1){
			OutputString(Contact.name, 0, 0, 0);
			OutputString(Contact.phone, 1, 0, 0);
		}else{
			OutputString("     <�����>    ", 0, 0, 0);
			OutputString("             ", 1, 0, 0);
		}
		
		if (KeyCode == 45) { // Menu
			KeyCode = 0;
			unsigned char *menu[] = {"��������", "������� �����", "�������", NULL};
			char result = ShowMenu(menu, 0);
			
			if(result == 1){ // ��������
				clrInd();

				unsigned char *field;
				input_text_props input_props;
				input_props.Aa = Aa;
				input_props.Language = Russian;
				if((field = InputText(
						(Contact.filled[0] == 1)
						?
							Contact.name
						:
							NULL,
						&input_props, sizeof(Contact.name)-1)) != NULL){

					unsigned char i;
					for(i = 0; (*(field + i) != '\0'); i++){
						Contact.name[i] = *(field + i);
					}
					Contact.name[i] = '\0';
					input_props.Language = Numeric;

					if((field = InputText(
						(Contact.filled[0] == 1)
						?
							Contact.phone
						:
							NULL,
						&input_props, sizeof(Contact.phone)-1)) != NULL){

						for(i = 0; (*(field + i) != '\0'); i++){
							Contact.phone[i] = *(field + i);
						}
						Contact.phone[i] = '\0';

						Contact.filled[0] = 1;
						WritePhonebookCellToEEPROM(&Contact, cell);
					}else{
						read = 0;
					}
				}
			}else if(result == 2){ // ������� �����
				
				input_text_props input_props;
				input_props.Language = Numeric;
				unsigned char *number = InputText(NULL, &input_props, 13);
				if(number != NULL){
					TelephoneCall(number);
					flags.ActiveCall = 1;
				}
			
			}else if(result == 3){ // �������
				
				if(Contact.filled[0] == 1 && Select_OK_NO("������� �����?")){
					Phone aaa;
					WritePhonebookCellToEEPROM(&aaa, cell);
					read = 0;
					clrInd();
				}
			}
			
		} else if (KeyCode == 31){
			KeyCode = 0;
			if(flags.ActiveCall){
				SendCommandToUSART("ATA", 0); // ������� �����
			}else if(Select_OK_NO("��������� �����?")){
				if(Contact.filled[0] == 1){
					TelephoneCall(Contact.phone);
					flags.ActiveCall = 1;
				}
			}
		} else if (KeyCode == 33) { // ��������� �����
			KeyCode = 0;
			SendCommandToUSART("ATH", 0);
			flags.ActiveCall = 0;
			flags.RemoteControlIsEnabled = 0;
		} else if (KeyCode == 40) { // Next
			KeyCode = 0;
			if(cell < LAST_PHONEBOOK_CELL){
				cell = cell + sizeof(Contact);
			}else{
				cell = PHONEBOOK_START_ADRESS;
			}
			read = 0;
			clrInd();
		} else if (KeyCode == 41) { // Prev
			KeyCode = 0;
			if(cell > PHONEBOOK_START_ADRESS){
				cell = cell - sizeof(Contact);
			}else{
				cell = LAST_PHONEBOOK_CELL;
			}
			read = 0;
			clrInd();
		} else if (KeyCode == 43) { // Exit
			KeyCode = 0;
			clrInd();
			return;
		} else if (KeyCode == 30) { // Adress = 0
			KeyCode = 0;
			cell = PHONEBOOK_START_ADRESS;
			read = 0;
			clrInd();
		}
	}
}

unsigned char Read_USART_SpeedNumber(){
	unsigned char value = EERD(USART_SPEED_NUMBER_CELL);
	if(value == 0 || value == 0xFF){
		value = 3;
	}
	return value;
}

void Save_USART_SpeedNumber(unsigned char speed){
	if(speed == 0 || speed == 0xFF){
		speed == 1;
	}
	EEWR(USART_SPEED_NUMBER_CELL, speed);
}

unsigned char USART_Speed_Choise(){
	unsigned char *menu[] = {
		" 1200",
		" 2400",
		" 9600",
		" 19200",
		//" 57600",
		NULL
	};
	return ShowMenu(menu, Read_USART_SpeedNumber());
}

void Settings(){
	KeyCode = 0;
	char item = 0;
	unsigned char *menu[] = {
		"�������. �����",
		"�������� USART",
		"Init GSM",
		NULL
	};
	do{

		item = ShowMenu(menu, item);

		if(item == 1){ // 
			TimeEdit();
		}else if(item == 2){ // 
			unsigned char speed = USART_Speed_Choise();
			if(speed != 0){
				Save_USART_SpeedNumber(speed);
			}
		}else if(item == 3){ // 
			Init_GSM(1);
		}
	}while(item > 0);
}

void main2() {
	
	//+CCLK: "16/05/15,22:37:52+03"
	//+CLIP: "+380957075762",145,"",,"",0
	//flags.UnprocessedIncommingUartData = 1;
	//ProcessIncommingUartData();
	//unsigned char container[50];
	
	//FindIncommingData(StandardAnswer_INCCALL, container, LAST_INCOMMING_DATA_INDEX);
	
	Init_GSM(1);
	
	flags.isTimeSetting = 0;

	NearTimeStart = Clock;
	AdressOfNextStartCell = FindNextTimeStart(&NearTimeStart);
		
	while (1) {
		
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
			char item = 0;
			unsigned char *menu[] = {
				"�����������",
				"���. �����",
				"���������",
				NULL
			};
			do{
				
				item = ShowMenu(menu, item);
				
				if(item == 1){ // 
					Scheduler(END_OF_CELLS);
				}else if(item == 2){ // 
					PhonebookEdit();
				}else if(item == 3){ // ���������
					Settings();
				}
			}while(item > 0);
			clrInd();
		} else if (KeyCode == 40) {
			KeyCode = 0;
			Scheduler(END_OF_CELLS);
		} else if (KeyCode == 41) {
			KeyCode = 0;
			PhonebookEdit();
		} else if (KeyCode == 44) {
			KeyCode = 0;
			if (flags.LCD_Power_On) {
				flags.LCD_Light_On = !flags.LCD_Light_On;
			}
		} else if (KeyCode == 30) {
			KeyCode = 0;
			flags.LockSignals = 1;
			CurrentSignals = 0;
			EEWR(LOCK_SIGNALS_FLAG_CELL, flags.LockSignals);
		} else if (KeyCode == 37 && !flags.DetailModeOfViewSheduler) {
			KeyCode = 0;
			clrInd();
			CurrentSignals = CurrentSignals^0b00000001;
		} else if (KeyCode == 38 && !flags.DetailModeOfViewSheduler) {
			KeyCode = 0;
			clrInd();
			CurrentSignals = CurrentSignals^0b00000010;
		} else if (KeyCode == 39 && !flags.DetailModeOfViewSheduler) {
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
			EEWR(LOCK_SIGNALS_FLAG_CELL, flags.LockSignals);
			clrInd();
		} else if (KeyCode == 31) {
			KeyCode = 0;
			lcd_off();
		} else if (KeyCode == 33) {
			KeyCode = 0;
			WorkingWithGSM();
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
		Seconds = BCD_to_DEC(I);
		cMinutes = BCD_to_DEC(I2CRead());
		I2CAck();
		cHours = BCD_to_DEC(I2CRead());
		I2CAck();
		cWeekDay = BCD_to_DEC(I2CRead());
		if (!flags.isTimeSetting) {
			I2CAck();
			cDays = BCD_to_DEC(I2CRead());
			I2CAck();
			cMonths = BCD_to_DEC(I2CRead());
			I2CAck();
			cYears = BCD_to_DEC(I2CRead());
		}
	} else {
		flags.TimeIsRead = 0;
		I2CNak();
		I2CStop();
		cDays = EERD(cDaysAdress);
		cMonths = EERD(cMonthsAdress);
		cYears = EERD(cYearsAdress);
		
		return;
	}
	I2CNak();

	I2CStop();

	Clock = ((long int) Seconds
		+ (long int) cMinutes * 60
		+ (long int) cHours * 3600
		+ ((long int) cWeekDay - 1) * 86400) * 100;

}

void WriteTime(unsigned long int lClock, unsigned char days, unsigned char months, unsigned char years) {

	volatile unsigned char INTCON_BUP = INTCON;
	INTCONbits.GIEH = 0;
	INTCONbits.GIEL = 0;
	ClrWdt();

	unsigned char week_day;
	unsigned char hours;
	unsigned char minutes;
	
	lClock/=6000;
	minutes = DEC_to_BCD(lClock % 60);
	lClock /= 60;
	hours = DEC_to_BCD(lClock % 24);
	lClock /= 24;
	week_day = DEC_to_BCD(lClock + 1);
	
	I2CInit();
	I2CStart();
	I2CSend(0xD0);
	I2CSend(0x00);

	I2CSend(0x00); // Seconds
	I2CSend(minutes); // Minutes
	I2CSend(hours); // Hours
	I2CSend(week_day); // DayOfWeek
	I2CSend(DEC_to_BCD(days)); // Days
	I2CSend(DEC_to_BCD(months)); // Mouths
	I2CSend(DEC_to_BCD(years)); // Years
	I2CSend(0b00010000); // Settings

	I2CStop();

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

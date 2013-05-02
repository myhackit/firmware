#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "usb_keyboard_debug.h"
#include "print.h"
//#include "sdcard.h"
//#include "macro.h"
#include "analog.h"

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define EMPTY_ROW       {0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe}
#define ZERO_ROW        {0,    0,    0,    0,    0,    0,    0}
#define JUST_SWITCHED   0x80
#define STILL_ON        0x00
#define DEBUG           1
#define ON              1
#define OFF             0
#define ROWS            10
#define COLS            7
#define MAP_SIZE        139

typedef struct{
	char name[30];
	int  id;
} pair;

pair str_map[MAP_SIZE] = {
	//2 Misc (Fail early)
	{"NA",0},    {"TOGGLE",KEY_TOGGLE},
	//25 Alphabetic
	{"A",KEY_A}, {"B",KEY_B}, {"C",KEY_C}, {"D",KEY_D}, {"E",KEY_E}, 
	{"F",KEY_F}, {"G",KEY_G}, {"H",KEY_H}, {"I",KEY_I}, {"J",KEY_J}, 
	{"K",KEY_K}, {"L",KEY_L}, {"M",KEY_M}, {"N",KEY_N}, {"O",KEY_O}, 
	{"P",KEY_P}, {"Q",KEY_Q}, {"R",KEY_R}, {"S",KEY_S}, {"T",KEY_T}, 
	{"U",KEY_U}, {"V",KEY_V}, {"W",KEY_W}, {"X",KEY_X}, {"Y",KEY_Y},
	{"Z",KEY_Z}, 
	//10 Numeric Keys
	{"1",KEY_1}, {"2",KEY_2}, {"3",KEY_3}, {"4",KEY_4}, {"5",KEY_5}, 
	{"6",KEY_6}, {"7",KEY_7}, {"8",KEY_8}, {"9",KEY_9}, {"0",KEY_0}, 
	//8 Macros
	{"MACRO_1",MACRO_1}, {"MACRO_2",MACRO_2}, {"MACRO_3",MACRO_3},
	{"MACRO_4",MACRO_4}, {"MACRO_5",MACRO_5}, {"MACRO_6",MACRO_6},
	{"MACRO_7",MACRO_7}, {"MACRO_8",MACRO_8},
	//18 Command
	{"ENTER",KEY_ENTER},   {"ESC",KEY_ESC},     {"BACKSPACE",KEY_BACKSPACE},
	{"TAB",KEY_TAB},       {"SPACE",KEY_SPACE}, {"MINUS",KEY_MINUS},
	{"EQUAL",KEY_EQUAL},   {"LEFT_BRACE",KEY_LEFT_BRACE}, 
	{"RIGHT_BRACE",KEY_RIGHT_BRACE},            {"BACKSLASH",KEY_BACKSLASH}, 
	{"NUMBER",KEY_NUMBER}, {"SEMICOLON",KEY_SEMICOLON},
	{"QUOTE",KEY_QUOTE},   {"TILDE",KEY_TILDE}, {"COMMA",KEY_COMMA}, 
	{"PERIOD",KEY_PERIOD}, {"SLASH",KEY_SLASH}, {"CAPS_LOCK",KEY_CAPS_LOCK},
	//12 Function Keys
	{"F1",KEY_F1}, {"F2",KEY_F2},   {"F3",KEY_F3},   {"F4",KEY_F4},
	{"F5",KEY_F5}, {"F6",KEY_F6},   {"F7",KEY_F7},   {"F8",KEY_F8}, 
	{"F9",KEY_F9}, {"F10",KEY_F10}, {"F11",KEY_F11}, {"F12",KEY_F12},
	//14
	{"PRINTSCREEN",KEY_PRINTSCREEN}, {"SCROLL_LOCK",KEY_SCROLL_LOCK},
	{"PAUSE",KEY_PAUSE},         {"INSERT",KEY_INSERT}, {"HOME",KEY_HOME},
	{"PAGE_UP",KEY_PAGE_UP},     {"DELETE",KEY_DELETE}, {"END",KEY_END},
	{"PAGE_DOWN",KEY_PAGE_DOWN}, {"RIGHT",KEY_RIGHT},   {"LEFT",KEY_LEFT}, 
	{"DOWN",KEY_DOWN}, {"UP",KEY_UP}, {"NUM_LOCK",KEY_NUM_LOCK},
	//16 Keypad keys
	{"KEYPAD_SLASH",KEYPAD_SLASH}, {"KEYPAD_ASTERIX",KEYPAD_ASTERIX}, 
	{"KEYPAD_MINUS",KEYPAD_MINUS}, {"KEYPAD_PLUS",KEYPAD_PLUS}, 
	{"KEYPAD_ENTER",KEYPAD_ENTER}, {"KEYPAD_PERIOD",KEYPAD_PERIOD},
	{"KEYPAD_1",KEYPAD_1}, {"KEYPAD_2",KEYPAD_2}, {"KEYPAD_3",KEYPAD_3},
	{"KEYPAD_4",KEYPAD_4}, {"KEYPAD_5",KEYPAD_5}, {"KEYPAD_6",KEYPAD_6},
	{"KEYPAD_7",KEYPAD_7}, {"KEYPAD_8",KEYPAD_8}, {"KEYPAD_9",KEYPAD_9},
	{"KEYPAD_0",KEYPAD_0},
	//12 Meta Keys
	{"CTRL",KEY_CTRL}, {"SHIFT",KEY_SHIFT}, {"ALT",KEY_ALT}, {"GUI",KEY_GUI},
	{"LEFT_CTRL",KEY_LEFT_CTRL},   {"LEFT_SHIFT",KEY_LEFT_SHIFT},
	{"LEFT_ALT",KEY_LEFT_ALT},     {"LEFT_GUI",KEY_LEFT_GUI},
	{"RIGHT_CTRL",KEY_RIGHT_CTRL}, {"RIGHT_SHIFT",KEY_RIGHT_SHIFT},
	{"RIGHT_ALT",KEY_RIGHT_ALT},   {"RIGHT_GUI",KEY_RIGHT_GUI},
	//8 Mouse Directions
	{"MOUSE_N",MOUSE_N},   {"MOUSE_E",MOUSE_E},   {"MOUSE_S",MOUSE_S},   {"MOUSE_W",MOUSE_W},
	//3 Mouse Buttons
	{"MOUSE_BTN_LEFT",MOUSE_BTN_LEFT}, {"MOUSE_BTN_MIDDLE",MOUSE_BTN_MIDDLE}, {"MOUSE_BTN_RIGHT",MOUSE_BTN_RIGHT},
	//4 Scroll Functions
	{"SCROLL_N",SCROLL_N}, {"SCROLL_E",SCROLL_E}, {"SCROLL_S",SCROLL_S}, {"SCROLL_W",SCROLL_W},
};

char maps[2][ROWS][COLS] = {
	{
		//LEFT SIDE
		{ KEY_ESC        , KEY_1          , KEY_2          , KEY_3          , KEY_4          , KEY_5          , /*RightH*/MOUSE_H    }, 
		{ KEY_TAB        , KEY_Q          , KEY_W          , KEY_E          , KEY_R          , KEY_T          , /*RightV*/MOUSE_V    },
		{ KEY_CAPS_LOCK  , KEY_A          , KEY_S          , KEY_D          , KEY_F          , KEY_G          , /*LeftH*/SCROLL_H     },
		{ KEY_LEFT_SHIFT , KEY_Z          , KEY_X          , KEY_C          , KEY_V          , KEY_B          , /*LeftV*/SCROLL_V     },
		{ KEY_LEFT_CTRL  , KEY_LEFT_ALT   , KEY_LEFT_GUI   , /*PED1*/KEY_TOGGLE, KEY_SPACE      , MOUSE_BTN_LEFT , 0           },
		//RIGHT SIDE
		{ KEY_6          , KEY_7          , KEY_8          , KEY_9          , KEY_0          , KEY_MINUS      , KEY_BACKSPACE  },
		{ KEY_Y          , KEY_U          , KEY_I          , KEY_O          , KEY_P          , KEY_LEFT_BRACE , KEY_RIGHT_BRACE},
		{ KEY_H          , KEY_J          , KEY_K          , KEY_L          , KEY_SEMICOLON  , KEY_QUOTE      , KEY_ENTER      },
		{ KEY_N          , KEY_M          , KEY_COMMA      , KEY_PERIOD     , KEY_BACKSLASH  , KEY_SLASH      , KEY_RIGHT_SHIFT},
		{ MOUSE_BTN_RIGHT, KEY_SPACE      , /*PED2*/MOUSE_BTN_LEFT, KEY_RIGHT_GUI  , KEY_RIGHT_ALT  , KEY_RIGHT_CTRL , KEY_TOGGLE     },
	},
	{
		//LEFT SIDE
		{ KEY_TILDE     , KEY_F1         , KEY_F2         , KEY_F3         , KEY_F4         , KEY_F5         , 0              }, 
		{ KEY_TAB       , 0              , MACRO_1        , MOUSE_N        , MACRO_2        , SCROLL_N       , 0              },
		{ KEY_CAPS_LOCK , 0              , MOUSE_W        , MOUSE_S        , MOUSE_E        , SCROLL_S       , 0              },
		{ KEY_LEFT_SHIFT, 0              , 0              , 0              , 0              , 0              , 0              },
		{ KEY_LEFT_CTRL , KEY_LEFT_ALT   , KEY_LEFT_GUI   , 0              , KEY_BACKSPACE  , KEY_TOGGLE     , 0              },
		//RIGHT SIDE
		{ KEY_F6        , KEY_F7         , KEY_F8         , KEY_F9         , KEY_F10        , KEY_F11        , KEY_EQUAL      },
		{ 0             , MOUSE_BTN_LEFT , MOUSE_BTN_MIDDLE, MOUSE_BTN_RIGHT, KEY_PAGE_UP    , KEY_INSERT     , 0              },
		{ KEY_LEFT      , KEY_DOWN       , KEY_UP         , KEY_RIGHT      , KEY_PAGE_DOWN  , KEY_DELETE     , KEY_ENTER      },
		{ 0             , KEY_HOME       , 0              , KEY_END        , 0              , 0              , KEY_RIGHT_SHIFT},
		{ KEY_TOGGLE    , KEY_BACKSPACE  , 0              , KEY_RIGHT_GUI  , KEY_RIGHT_ALT  , KEY_RIGHT_CTRL , KEY_TOGGLE     },
	}
};
char key_history[ROWS][COLS] = {EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW};
uint16_t key_ticks[ROWS][COLS] = {ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW};

static uint8_t aref = (1<<REFS0); // default to AREF = Vcc
int16_t adc_read(uint8_t mux){
	uint8_t low;
	ADCSRA = (1<<ADEN) | ADC_PRESCALER;             // enable ADC
	ADCSRB = (1<<ADHSM) | (mux & 0x20);             // high speed mode
	ADMUX = aref | (mux & 0x1F);                    // configure mux input
	ADCSRA = (1<<ADEN) | ADC_PRESCALER | (1<<ADSC); // start the conversion
	while (ADCSRA & (1<<ADSC)) ;                    // wait for result
	low = ADCL;                                     // must read LSB first
	return (ADCH << 8) | low;                       // must read MSB only once!
} //adc_read

int check_col(int c, int r) {
	int out = 0;
	if(r==4 && c==3) c=10; // Pedal 1
	if(r==9 && c==2) c=11; // Pedal 2
	switch (c) {
		case 0:  out = (PIND & (1<<5)); break;
		case 1:  out = (PINE & (1<<0)); break;
		case 2:  out = (PINC & (1<<7)); break;
		case 3:  out = (PINE & (1<<1)); break;
		case 4:  out = (PIND & (1<<7)); break;
		case 5:  out = (PINC & (1<<0)); break;
		case 6:  out = (PINC & (1<<1)); break;
		case 10: out = (PINE & (1<<6)); break; // Pedal 1
		case 11: out = (PINE & (1<<7)); break; // Pedal 2
	}
	out = (out ^ 0); // Inverting for pullup resistors
#if DEBUG == 2
	print(out ? " " : "*");
#endif
	return out;
} // check_col

void turn_row(int i, int on) {
	int out = 0;
	switch (i) {
		case 0: out = on ? (PORTD &= ~(1<<2)) : (PORTD |= (1<<2)); break;
		case 1: out = on ? (PORTD &= ~(1<<0)) : (PORTD |= (1<<0)); break;
		case 2: out = on ? (PORTD &= ~(1<<3)) : (PORTD |= (1<<3)); break;
		case 3: out = on ? (PORTD &= ~(1<<1)) : (PORTD |= (1<<1)); break;
		case 4: out = on ? (PORTD &= ~(1<<4)) : (PORTD |= (1<<4)); break;
		case 5: out = on ? (PORTC &= ~(1<<4)) : (PORTC |= (1<<4)); break;
		case 6: out = on ? (PORTC &= ~(1<<2)) : (PORTC |= (1<<2)); break;
		case 7: out = on ? (PORTC &= ~(1<<5)) : (PORTC |= (1<<5)); break;
		case 8: out = on ? (PORTC &= ~(1<<3)) : (PORTC |= (1<<3)); break;
		case 9: out = on ? (PORTC &= ~(1<<6)) : (PORTC |= (1<<6)); break;
	}
} // turn_row

int main(void) {

	int8_t mouse_h, mouse_v, scroll_h, scroll_v, mouse_btn_left, mouse_btn_middle, mouse_btn_right;
	uint8_t row, col, idx, key, delta, which_map;
	uint8_t mouse_btn_left_prev   = 0;
	uint8_t mouse_btn_middle_prev = 0;
	uint8_t mouse_btn_right_prev  = 0;
	uint16_t ticks;

	// set for 16 MHz clock
	CPU_PRESCALE(0);

	// Turn off unneeded analog pins
	DIDR0 = 0b00001111;

	//0=In 1=Out
	//     0b76543210;
	DDRC = 0b01111100;
	DDRD = 0b00011111;
	DDRE = 0b11000000;
	DDRF = 0b00000000;

	//if DDR=0, 0=Normal, 1=Pullup Resistor
	//if DDR=1, 0=Low Output, 1=High Output
	//      0b76543210;
	PORTC = 0b11111111;
	PORTD = 0b11111111;
	PORTE = 0b11111111;
	PORTF = 0b11111111;

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);

	print("Starting myhackit\n");

	analog_autocalibrate_center();

	print("Autocalibrate complete\n");

	//TODO PARSE SD CARD

	while (1) { // Main logic loop

		// Re-initialize buffers
		for(idx=0; idx<sizeof(keyboard_keys); idx++) keyboard_keys[idx] = 0; //TODO Seems like this could be reset with bit flipping?
		keyboard_modifier_keys = idx = 0;
		mouse_btn_left = mouse_btn_middle = mouse_btn_right = 0;
		mouse_h = mouse_v = scroll_h = scroll_v = 0;

		// TODO Map this out into the dead spots in the grid somehow?
		analog_update(0);
		mouse_h = analogs[0].moving;
		mouse_v = analogs[1].moving;
		scroll_h = analogs[2].moving;
		scroll_v = analogs[3].moving;

		// Read in the current key state
		which_map = 0;
		for(row=0; row<ROWS; row++){
			turn_row(row, ON);
			for(col=0; col<COLS; col++){
				if (check_col(col, row)){
					key_history[row][col] |= 1;
				}
				key_history[row][col] <<= 1;

				switch (key_history[row][col]){
					case JUST_SWITCHED:
						key_ticks[row][col] = 1;
					break;
					case STILL_ON:
						key_ticks[row][col]  += 1;
						if(maps[0][row][col] == KEY_TOGGLE){
							which_map = 1;  // Use the alternate map
						}
					break;
					default:
						key_ticks[row][col] = 0;
					break;
				}
			}
			turn_row(row, OFF);
		}

		// Determine which keys are pressed
		for(col=0; col<COLS; col++){
			for(row=0; row<ROWS; row++){
				key = maps[which_map][row][col];
				if(key_history[row][col] == STILL_ON){
					if (key == KEY_TOGGLE){
						continue; // Skip toggle buttons
					} else if ( key >= KEY_CTRL && key <= KEY_RIGHT_GUI ){
						keyboard_modifier_keys |= 1<<(key-224); // Could also change USB report to do this
					} else if (key >= MOUSE_N && key < KEY_TOGGLE){ // Digital pointer concerns
						ticks = key_ticks[row][col];
						if      (ticks > 120 ) delta = 4;
						else if (ticks > 80 )  delta = 3;
						else if (ticks > 40  ) delta = 2;
						else                   delta = 1;

						if (key == MOUSE_N) mouse_v -= delta;
						if (key == MOUSE_S) mouse_v += delta;
						if (key == MOUSE_E) mouse_h += delta;
						if (key == MOUSE_W) mouse_h -= delta;
						if (key == MOUSE_BTN_LEFT)   mouse_btn_left = 1;
						if (key == MOUSE_BTN_MIDDLE) mouse_btn_middle = 1;
						if (key == MOUSE_BTN_RIGHT)  mouse_btn_right = 1;
						if (key == SCROLL_N && ticks % ANALOG_SPEED == 1) scroll_v = delta;
						if (key == SCROLL_E && ticks % ANALOG_SPEED == 1) scroll_h = -delta;
						if (key == SCROLL_S && ticks % ANALOG_SPEED == 1) scroll_v = -delta;
						if (key == SCROLL_W && ticks % ANALOG_SPEED == 1) scroll_h = delta;
					} else if (key >= MACRO_1 && key <= MACRO_8){ //Macro Concerns
						start_macro(MACRO_1 - key);
					} else { //  Not a special case, just boring old keypress
						if (idx<7){ //  Throwing away more than 6 at a time, damn usb :(
							keyboard_keys[idx++] = key;
						}
					}
				}
			}
		}

		// run_macros(keyboard_keys, idx); //TODO

		// Send digital keyboard signals
		usb_keyboard_send(); // Send the buffered commands

		// Send alaog (mouse+wheel) signals
		if (mouse_h || mouse_v || scroll_v || scroll_h){
			usb_mouse_move(mouse_h, mouse_v, scroll_v, scroll_h);
		}
		if ( mouse_btn_left   != mouse_btn_left_prev || 
		     mouse_btn_middle != mouse_btn_middle_prev || 
		     mouse_btn_right  != mouse_btn_right_prev
		) {
			usb_mouse_buttons(mouse_btn_left, mouse_btn_middle, mouse_btn_right);
		}

		mouse_btn_left_prev   = mouse_btn_left;
		mouse_btn_middle_prev = mouse_btn_middle;
		mouse_btn_right_prev  = mouse_btn_right;

	} // while
} // main

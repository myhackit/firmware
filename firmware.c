#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_keyboard_debug.h"
#include "print.h"

#define LED_CONFIG	(DDRD |= (1<<6))
#define LED_ON		(PORTD &= ~(1<<6))
#define LED_OFF		(PORTD |= (1<<6))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

#define ROWS 10
#define COLS 7

char map_default[ROWS][COLS] = {
	//LEFT SIDE
	{ KEY_ESC       , KEY_1          , KEY_2          , KEY_3          , KEY_4          , KEY_5          , 0              }, 
	{ KEY_TAB       , KEY_Q          , KEY_W          , KEY_E          , KEY_R          , KEY_T          , 0              },
	{ KEY_TOGGLE    , KEY_A          , KEY_S          , KEY_D          , KEY_F          , KEY_G          , 0              },
	{ KEY_LEFT_SHIFT, KEY_Z          , KEY_X          , KEY_C          , KEY_V          , KEY_B          , 0              },
	{ KEY_LEFT_CTRL , KEY_LEFT_ALT   , KEY_LEFT_GUI   , 0              , KEY_SPACE      , 0              , 0              },
	//RIGHT SIDE
	{ KEY_6         , KEY_7          , KEY_8          , KEY_9          , KEY_0          , KEY_MINUS      , KEY_BACKSPACE  },
	{ KEY_Y         , KEY_U          , KEY_I          , KEY_O          , KEY_P          , KEY_LEFT_BRACE , KEY_RIGHT_BRACE},
	{ KEY_H         , KEY_J          , KEY_K          , KEY_L          , KEY_SEMICOLON  , KEY_QUOTE      , KEY_ENTER      },
	{ KEY_N         , KEY_M          , KEY_COMMA      , KEY_PERIOD     , KEY_BACKSLASH  , KEY_SLASH      , KEY_RIGHT_SHIFT},
	{ 0             , KEY_SPACE      , 0              , KEY_RIGHT_GUI  , KEY_RIGHT_ALT  , KEY_RIGHT_CTRL , KEY_TOGGLE     },
};
char map_toggle[ROWS][COLS] = {
	//LEFT SIDE
	{ KEY_TILDE     , KEY_F1         , KEY_F2         , KEY_F3         , KEY_F4         , KEY_F5         , 0              }, 
	{ KEY_TAB       , 0              , 0              , MOUSE_N        , 0              , MOUSE_SCROLL_UP, 0              },
	{ KEY_TOGGLE    , 0              , MOUSE_W        , MOUSE_S        , MOUSE_E        , MOUSE_SCROLL_DN, 0              },
	{ KEY_LEFT_SHIFT, 0              , 0              , 0              , 0              , 0              , 0              },
	{ KEY_LEFT_CTRL , KEY_LEFT_ALT   , KEY_LEFT_GUI   , 0              , KEY_BACKSPACE  , 0              , 0              },
	//RIGHT SIDE
	{ KEY_F6        , KEY_F7         , KEY_F8         , KEY_F9         , KEY_F10        , KEY_F11        , KEY_EQUAL      },
	{ 0             , KEY_HOME       , KEY_UP         , KEY_END        , KEY_PAGE_UP    , KEY_INSERT     , 0              },
	{ 0             , KEY_LEFT       , KEY_DOWN       , KEY_RIGHT      , KEY_PAGE_DOWN  , KEY_DELETE     , KEY_ENTER      },
	{ 0             , MOUSE_BTNL     , MOUSE_BTNM     , MOUSE_BTNR     , 0              , 0              , KEY_RIGHT_SHIFT},
	{ 0             , KEY_BACKSPACE  , 0              , KEY_RIGHT_GUI  , KEY_RIGHT_ALT  , KEY_RIGHT_CTRL , KEY_TOGGLE     },
};
#define EMPTY_ROW {1,1,1,1,1,1,1}
#define ZERO_ROW {0,0,0,0,0,0,0}
#define MOUSE_SPEED 50
#define JUST_SWITCHED   0x80
#define STILL_ON        0x00
#define ASSERTED        1
//#define DEBUG           0

char key_history[ROWS][COLS] = {EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW};
uint16_t key_ticks[ROWS][COLS] = {ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW};

int main(void)
{
	uint8_t row, col, i, key, toggle_mode, delta;
	uint8_t mouse_left_prev, mouse_middle_prev, mouse_right_prev;
	uint16_t ticks;
	int8_t x, y, wheel, mouse_left, mouse_middle, mouse_right;

	mouse_left_prev = mouse_middle_prev = mouse_right_prev = 0;

	// set for 16 MHz clock
	CPU_PRESCALE(0);

	// RIGHT ROWS  = B0-B4 = 0x1f
	// ALL COLUMNS = C0-C6 = 0x7f
	// LEFT ROWS   = D1-D5 = 0x3e

	//0=Input 1=Output
	// Columns Input, Rows Output
	DDRB = 0x00;
	DDRC = 0x7f;
	DDRD = 0x00;

	//if DDR=1, 0=Low Output, 1=High Output
	//if DDR=0, 0=Normal, 1=Pullup Resistor
	PORTB = 0xff;
	PORTC = 0xff;
	PORTD = 0xff;

	LED_CONFIG;
	LED_OFF;

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);

	print("Starting myhackit\n");

	while (1) {
		//Read in the current key state
		toggle_mode = 0;
		for(col=0;col<COLS;col++){
			PORTC &= ~(1<<col); //Clear bit to 0
			for(row=0;row<ROWS;row++){
				if (row<5){ //Left Side
					if ((PIND & (1<<(row+1))) ^ (!ASSERTED)) key_history[row][col] |= 1;
				} else { //Right Side
					if ((PINB & (1<<(row-5))) ^ (!ASSERTED)) key_history[row][col] |= 1;
				}
				key_history[row][col] <<= 1;

				if (key_history[row][col] == JUST_SWITCHED){
					key_ticks[row][col] = 1;
				} else if (key_history[row][col] == STILL_ON) {
					key_ticks[row][col]  += 1;
					if(map_default[row][col] == KEY_TOGGLE) toggle_mode = 1;  //Use the alternate map
				} else {
					key_ticks[row][col] = 0;
				}
			}
			PORTC |= (1<<col); //Set bit back to 1
		}


		// Re-initialize buffers
		for(i=0;i<7;i++) keyboard_keys[i] = 0;
		keyboard_modifier_keys = i = 0;
		mouse_left = mouse_middle = mouse_right = 0;
		x = y = wheel = 0;

		// Determine which keys are pressed
		for(col=0;col<COLS;col++){
			for(row=0;row<ROWS;row++){
				ticks = key_ticks[row][col];
				key = (toggle_mode) ? map_toggle[row][col] : map_default[row][col];
#ifdef DEBUG
				phex(key_history[row][col]);
#endif
				if(key_history[row][col] == STILL_ON){
					if (key == KEY_TOGGLE) continue; //Skip toggle buttons
					if( key > 223 && key < 232 ){
						keyboard_modifier_keys |= 1<<(key-224); //Could also change USB report to do this
					} else if (key > 234 && key < 255){ //Mouse Concerns
						if      (ticks > 180 ) delta = 4;
						else if (ticks > 120 ) delta = 3;
						else if (ticks > 60  ) delta = 3;
						else                   delta = 2;

						if (key == MOUSE_N) y -= delta;
						if (key == MOUSE_S) y += delta;
						if (key == MOUSE_E) x += delta;
						if (key == MOUSE_W) x -= delta;
						if (key == MOUSE_SCROLL_UP && ticks % MOUSE_SPEED == 1) wheel = delta;
						if (key == MOUSE_SCROLL_DN && ticks % MOUSE_SPEED == 1) wheel = -delta;
						if (key == MOUSE_BTNL) mouse_left = 1;
						if (key == MOUSE_BTNM) mouse_middle = 1;
						if (key == MOUSE_BTNR) mouse_right = 1;
					} else { //Not a special case, just boring old keypress
						if (i<7){ //Throwing away more than 6, damn usb :(
							keyboard_keys[i++] = key;
						}
					}
				}
			}
#ifdef DEBUG
			print("\n");
#endif
		}
#ifdef DEBUG
		print("\n");
		print("\n");
		_delay_ms(100);
#endif
		usb_keyboard_send();

		if (x || y || wheel) usb_mouse_move(x,y,wheel);
		if ( mouse_left != mouse_left_prev || 
		     mouse_middle != mouse_middle_prev || 
		     mouse_right != mouse_right_prev
		) usb_mouse_buttons(mouse_left, mouse_middle, mouse_right);

		mouse_left_prev = mouse_left;
		mouse_middle_prev = mouse_middle;
		mouse_right_prev = mouse_right;

		continue;
	}
}

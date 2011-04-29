#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_keyboard_debug.h"
#include "print.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sd-reader/fat.h>
#include <sd-reader/fat_config.h>
#include <sd-reader/partition.h>
#include <sd-reader/sd_raw.h>
#include <sd-reader/sd_raw_config.h>

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))
#define EMPTY_ROW {0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe}
#define ZERO_ROW {0,0,0,0,0,0,0}
#define MOUSE_SPEED 50
#define JUST_SWITCHED   0x80
#define STILL_ON        0x00
#define ASSERTED        1
#define DEBUG           1
#define ROWS 10
#define COLS 7


uint16_t lookup_key(const char* key){
	if      (!strcmp(key,"0")) return 0;
	else if (!strcmp(key,"A")) return 4;
	else if (!strcmp(key,"B")) return 5;
	else if (!strcmp(key,"C")) return 6;
	else if (!strcmp(key,"D")) return 7;
	else if (!strcmp(key,"E")) return 8;
	else if (!strcmp(key,"F")) return 9;
	else if (!strcmp(key,"G")) return 10;
	else if (!strcmp(key,"H")) return 11;
	else if (!strcmp(key,"I")) return 12;
	else if (!strcmp(key,"J")) return 13;
	else if (!strcmp(key,"K")) return 14;
	else if (!strcmp(key,"L")) return 15;
	else if (!strcmp(key,"M")) return 16;
	else if (!strcmp(key,"N")) return 17;
	else if (!strcmp(key,"O")) return 18;
	else if (!strcmp(key,"P")) return 19;
	else if (!strcmp(key,"Q")) return 20;
	else if (!strcmp(key,"R")) return 21;
	else if (!strcmp(key,"S")) return 22;
	else if (!strcmp(key,"T")) return 23;
	else if (!strcmp(key,"U")) return 24;
	else if (!strcmp(key,"V")) return 25;
	else if (!strcmp(key,"W")) return 26;
	else if (!strcmp(key,"X")) return 27;
	else if (!strcmp(key,"Y")) return 28;
	else if (!strcmp(key,"Z")) return 29;
	else if (!strcmp(key,"1")) return 30;
	else if (!strcmp(key,"2")) return 31;
	else if (!strcmp(key,"3")) return 32;
	else if (!strcmp(key,"4")) return 33;
	else if (!strcmp(key,"5")) return 34;
	else if (!strcmp(key,"6")) return 35;
	else if (!strcmp(key,"7")) return 36;
	else if (!strcmp(key,"8")) return 37;
	else if (!strcmp(key,"9")) return 38;
	else if (!strcmp(key,"0")) return 39;
	else if (!strcmp(key,"ENTER")) return 40;
	else if (!strcmp(key,"ESC")) return 41;
	else if (!strcmp(key,"BACKSPACE")) return 42;
	else if (!strcmp(key,"TAB")) return 43;
	else if (!strcmp(key,"SPACE")) return 44;
	else if (!strcmp(key,"MINUS")) return 45;
	else if (!strcmp(key,"EQUAL")) return 46;
	else if (!strcmp(key,"LEFT_BRACE")) return 47;
	else if (!strcmp(key,"RIGHT_BRACE")) return 48;
	else if (!strcmp(key,"BACKSLASH")) return 49;
	else if (!strcmp(key,"NUMBER")) return 50;
	else if (!strcmp(key,"SEMICOLON")) return 51;
	else if (!strcmp(key,"QUOTE")) return 52;
	else if (!strcmp(key,"TILDE")) return 53;
	else if (!strcmp(key,"COMMA")) return 54;
	else if (!strcmp(key,"PERIOD")) return 55;
	else if (!strcmp(key,"SLASH")) return 56;
	else if (!strcmp(key,"CAPS_LOCK")) return 57;
	else if (!strcmp(key,"F1")) return 58;
	else if (!strcmp(key,"F2")) return 59;
	else if (!strcmp(key,"F3")) return 60;
	else if (!strcmp(key,"F4")) return 61;
	else if (!strcmp(key,"F5")) return 62;
	else if (!strcmp(key,"F6")) return 63;
	else if (!strcmp(key,"F7")) return 64;
	else if (!strcmp(key,"F8")) return 65;
	else if (!strcmp(key,"F9")) return 66;
	else if (!strcmp(key,"F10")) return 67;
	else if (!strcmp(key,"F11")) return 68;
	else if (!strcmp(key,"F12")) return 69;
	else if (!strcmp(key,"PRINTSCREEN")) return 70;
	else if (!strcmp(key,"SCROLL_LOCK")) return 71;
	else if (!strcmp(key,"PAUSE")) return 72;
	else if (!strcmp(key,"INSERT")) return 73;
	else if (!strcmp(key,"HOME")) return 74;
	else if (!strcmp(key,"PAGE_UP")) return 75;
	else if (!strcmp(key,"DELETE")) return 76;
	else if (!strcmp(key,"END")) return 77;
	else if (!strcmp(key,"PAGE_DOWN")) return 78;
	else if (!strcmp(key,"RIGHT")) return 79;
	else if (!strcmp(key,"LEFT")) return 80;
	else if (!strcmp(key,"DOWN")) return 81;
	else if (!strcmp(key,"UP")) return 82;
	else if (!strcmp(key,"NUM_LOCK")) return 83;
	else if (!strcmp(key,"KEYPAD_SLASH")) return 84;
	else if (!strcmp(key,"KEYPAD_ASTERIX")) return 85;
	else if (!strcmp(key,"KEYPAD_MINUS")) return 86;
	else if (!strcmp(key,"KEYPAD_PLUS")) return 87;
	else if (!strcmp(key,"KEYPAD_ENTER")) return 88;
	else if (!strcmp(key,"KEYPAD_1")) return 89;
	else if (!strcmp(key,"KEYPAD_2")) return 90;
	else if (!strcmp(key,"KEYPAD_3")) return 91;
	else if (!strcmp(key,"KEYPAD_4")) return 92;
	else if (!strcmp(key,"KEYPAD_5")) return 93;
	else if (!strcmp(key,"KEYPAD_6")) return 94;
	else if (!strcmp(key,"KEYPAD_7")) return 95;
	else if (!strcmp(key,"KEYPAD_8")) return 96;
	else if (!strcmp(key,"KEYPAD_9")) return 97;
	else if (!strcmp(key,"KEYPAD_0")) return 98;
	else if (!strcmp(key,"KEYPAD_PERIOD")) return 99;
	else if (!strcmp(key,"CTRL")) return 224;
	else if (!strcmp(key,"SHIFT")) return 225;
	else if (!strcmp(key,"ALT")) return 226;
	else if (!strcmp(key,"GUI")) return 227;
	else if (!strcmp(key,"LEFT_CTRL")) return 224;
	else if (!strcmp(key,"LEFT_SHIFT")) return 225;
	else if (!strcmp(key,"LEFT_ALT")) return 226;
	else if (!strcmp(key,"LEFT_GUI")) return 227;
	else if (!strcmp(key,"RIGHT_CTRL")) return 228;
	else if (!strcmp(key,"RIGHT_SHIFT")) return 229;
	else if (!strcmp(key,"RIGHT_ALT")) return 230;
	else if (!strcmp(key,"RIGHT_GUI")) return 231;

	else if (!strcmp(key,"MACRO_1")) return 232;
	else if (!strcmp(key,"MACRO_2")) return 233;
	else if (!strcmp(key,"MACRO_3")) return 234;
	else if (!strcmp(key,"MACRO_4")) return 235;
	else if (!strcmp(key,"MACRO_5")) return 236;
	else if (!strcmp(key,"MACRO_6")) return 237;
	else if (!strcmp(key,"MACRO_7")) return 238;
	else if (!strcmp(key,"MACRO_8")) return 239;

	else if (!strcmp(key,"MOUSE_N")) return 240;
	else if (!strcmp(key,"MOUSE_NE")) return 241;
	else if (!strcmp(key,"MOUSE_E")) return 242;
	else if (!strcmp(key,"MOUSE_SE")) return 243;
	else if (!strcmp(key,"MOUSE_S")) return 244;
	else if (!strcmp(key,"MOUSE_SW")) return 245;
	else if (!strcmp(key,"MOUSE_W")) return 246;
	else if (!strcmp(key,"MOUSE_NW")) return 247;
	else if (!strcmp(key,"MOUSE_BTNL")) return 248;
	else if (!strcmp(key,"MOUSE_BTNM")) return 249;
	else if (!strcmp(key,"MOUSE_BTNR")) return 250;
	else if (!strcmp(key,"MOUSE_SCROLL_UP")) return 251;
	else if (!strcmp(key,"MOUSE_SCROLL_DN")) return 252;
	else if (!strcmp(key,"TOGGLE")) return 255;
	return 0;
}

char map_default[ROWS][COLS]
= {
	//LEFT SIDE
	{ KEY_ESC       , KEY_1          , KEY_2          , KEY_3          , KEY_4          , KEY_5          , 0              }, 
	{ KEY_TAB       , KEY_Q          , KEY_W          , KEY_E          , KEY_R          , KEY_T          , 0              },
	{ KEY_CAPS_LOCK , KEY_A          , KEY_S          , KEY_D          , KEY_F          , KEY_G          , 0              },
	{ KEY_LEFT_SHIFT, KEY_Z          , KEY_X          , KEY_C          , KEY_V          , KEY_B          , 0              },
	{ KEY_LEFT_CTRL , KEY_LEFT_ALT   , KEY_LEFT_GUI   , 0              , KEY_SPACE      , KEY_TOGGLE     , 0              },
	//RIGHT SIDE
	{ KEY_6         , KEY_7          , KEY_8          , KEY_9          , KEY_0          , KEY_MINUS      , KEY_BACKSPACE  },
	{ KEY_Y         , KEY_U          , KEY_I          , KEY_O          , KEY_P          , KEY_LEFT_BRACE , KEY_RIGHT_BRACE},
	{ KEY_H         , KEY_J          , KEY_K          , KEY_L          , KEY_SEMICOLON  , KEY_QUOTE      , KEY_ENTER      },
	{ KEY_N         , KEY_M          , KEY_COMMA      , KEY_PERIOD     , KEY_BACKSLASH  , KEY_SLASH      , KEY_RIGHT_SHIFT},
	{ KEY_TOGGLE    , KEY_SPACE      , 0              , KEY_RIGHT_GUI  , KEY_RIGHT_ALT  , KEY_RIGHT_CTRL , KEY_TOGGLE     },
};
char map_toggle[ROWS][COLS]
= {
	//LEFT SIDE
	{ KEY_TILDE     , KEY_F1         , KEY_F2         , KEY_F3         , KEY_F4         , KEY_F5         , 0              }, 
	{ KEY_TAB       , 0              , MACRO_2        , MOUSE_N        , MACRO_1        , MOUSE_SCROLL_UP, 0              },
	{ KEY_CAPS_LOCK , 0              , MOUSE_W        , MOUSE_S        , MOUSE_E        , MOUSE_SCROLL_DN, 0              },
	{ KEY_LEFT_SHIFT, 0              , 0              , 0              , 0              , 0              , 0              },
	{ KEY_LEFT_CTRL , KEY_LEFT_ALT   , KEY_LEFT_GUI   , 0              , KEY_BACKSPACE  , KEY_TOGGLE     , 0              },
	//RIGHT SIDE
	{ KEY_F6        , KEY_F7         , KEY_F8         , KEY_F9         , KEY_F10        , KEY_F11        , KEY_EQUAL      },
	{ 0             , KEY_HOME       , KEY_UP         , KEY_END        , KEY_PAGE_UP    , KEY_INSERT     , 0              },
	{ 0             , KEY_LEFT       , KEY_DOWN       , KEY_RIGHT      , KEY_PAGE_DOWN  , KEY_DELETE     , KEY_ENTER      },
	{ 0             , MOUSE_BTNL     , MOUSE_BTNM     , MOUSE_BTNR     , 0              , 0              , KEY_RIGHT_SHIFT},
	{ KEY_TOGGLE    , KEY_BACKSPACE  , 0              , KEY_RIGHT_GUI  , KEY_RIGHT_ALT  , KEY_RIGHT_CTRL , KEY_TOGGLE     },
};
char key_history[ROWS][COLS] = {EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW,EMPTY_ROW};
uint16_t key_ticks[ROWS][COLS] = {ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW,ZERO_ROW};

uint16_t macro_1_ticks;
char macro_1_keys[5] = { KEY_H,     KEY_E,     KEY_L,   KEY_L,KEY_O};
uint16_t macro_1_time[11] = { 30,100, 30,30, 30,100, 30,100,30,100,30 };


//static uint8_t read_line(char* buffer, uint8_t buffer_length);
//static uint32_t strtolong(const char* str);
static uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry);
static struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name); 
//static uint8_t print_disk_info(const struct fat_fs_struct* fs);

int main(void) {

	uint8_t row, col, i, j, key, toggle_mode, delta;
	uint8_t mouse_left_prev, mouse_middle_prev, mouse_right_prev;
	uint16_t ticks;
	int8_t x, y, wheel, mouse_left, mouse_middle, mouse_right;

	mouse_left_prev = mouse_middle_prev = mouse_right_prev = 0;

	macro_1_ticks = 0;

	// set for 16 MHz clock
	CPU_PRESCALE(0);

	//0=Input 1=Output
	DDRC = 0x00;
	DDRD = 0x7f;
	DDRF = 0x7f;

	//if DDR=1, 0=Low Output, 1=High Output
	//if DDR=0, 0=Normal, 1=Pullup Resistor
	PORTC = 0xff;
	PORTD = 0xff;
	PORTF = 0xff;

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);


	//16000000/s  
	for(i=0;i<11;i++){
		macro_1_time[i] = macro_1_time[i]+ ((i==0)?0:macro_1_time[i-1]);
	}

	print("Starting myhackit\n");

	/* setup sd card slot */
#if DEBUG
	if(!sd_raw_init()) {
		print("MMC/SD initialization failed\n");
	}
#endif

	/* open first partition */
	struct partition_struct* partition = partition_open(sd_raw_read, sd_raw_read_interval, sd_raw_write, sd_raw_write_interval, 0);
	/* If the partition did not open, assume the storage device
	* is a "superfloppy", i.e. has no MBR.
	*/
	if(!partition) {
		partition = partition_open(sd_raw_read, sd_raw_read_interval, sd_raw_write, sd_raw_write_interval, -1);
#if DEBUG
		if(!partition) {
			print("opening partition failed\n");
		}
#endif
	}

	/* open file system */
	struct fat_fs_struct* fs = fat_open(partition);
#if DEBUG
	if(!fs){
		print("opening filesystem failed\n");
	}
#endif

	/* open root directory */
	struct fat_dir_entry_struct directory;
	fat_get_dir_entry_of_path(fs, "/layouts/", &directory);

	struct fat_dir_struct* dd = fat_open_dir(fs, &directory);
#if DEBUG
	if(!dd){
		print("opening root directory failed\n");
	}
#endif

	/* search file in current directory and open it */
	struct fat_file_struct* fd = open_file_in_dir(fs, dd, "myhackit");
#if DEBUG
	if(!fd){
		print("error opening file\n");
	}
#endif

	/* print file contents */
	uint8_t buffer[8];
	/*char* line = malloc(300);
	strcpy(line, "hello, ");
	strcat(line,"good afternoon");
	for(j=0;j<strlen(line);j++){
		usb_debug_putchar(line[j]);
	}*/
	uint32_t offset = 0;
	uint8_t map_toggle2 = 0;
	uint8_t side = 0;
	uint8_t length = 0;
	uint8_t map_row = 0;
	uint8_t map_col = 0;
	uint8_t ignore = 0;
	uint8_t which_key[30];
	while(fat_read_file(fd, buffer, sizeof(buffer)) > 0){
		for(uint8_t i = 0; i < sizeof(buffer); ++i){
			if(offset == 0){
				switch (buffer[i]) {
					case 'L': side = 0; break;
					case 'R': side = 1; break;
					case 'M': map_toggle2 = 0; ignore = 1; break;
					case 'T': map_toggle2 = 1; ignore = 1; break;
				}
			} else {
				if(!ignore){
					if(offset == 1){
						map_row = buffer[i]-49;
					} else if(offset > 3){
						switch (buffer[i]) {
							case ' ':
							break;
							case '\n':
							case ',':
								which_key[length] = '\0';
								if(map_toggle2){
									//map_toggle[side*5+map_row][map_col] = lookup_key(which_key);
								} else {
									//map_default[side*5+map_row][map_col] = lookup_key(which_key);
								}
								for(uint8_t j=0;j<length;j++){
									usb_debug_putchar(which_key[j]);
								}
								print(", ");
								length = 0;
								if (buffer[i]=='\n') {
									map_col = 0;
								} else {
									map_col++;
								}
							break;
							default:
								which_key[length++]=buffer[i];
							break;
						
						}
					}
				}
			}
			//usb_debug_putchar(buffer[i]);
			offset += 1;
			if(buffer[i] == '\n'){
				offset = 0;
				ignore = 0;
			} 
		}
	}
	fat_close_file(fd);



	while (1) {
		//Read in the current key state
		toggle_mode = 0;
		for(row=0;row<10;row++){
			if(row<5){
				PORTD &= ~(1<<row);
			} else {
				PORTF &= ~(1<<(row-5));
			}

			for(col=0;col<COLS;col++){
				if ((PINC & (1<<col)) ^ (!ASSERTED)) key_history[row][col] |= 1;
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
			//pbin(PINC); print(" "); pbin(PIND); print(" "); pbin(PINF); print("\n");
			if(row<5){
				PORTD |= (1<<row);
			} else {
				PORTF |= (1<<(row-5));
			}
		}

		// Re-initialize buffers
		for(i=0;i<7;i++) keyboard_keys[i] = 0;
		keyboard_modifier_keys = i = 0;
		mouse_left = mouse_middle = mouse_right = 0;
		x = y = wheel = 0;


		if(macro_1_ticks){
			for(j=0;j<11;j++){
				if (macro_1_ticks <= macro_1_time[j]){
					break;
				}
			}
			if(j==11){
				macro_1_ticks = 0;
			} else {
				if(j%2 == 0){
					keyboard_keys[i++] = macro_1_keys[j/2];
				}
				macro_1_ticks++;
			}
		}

		// Determine which keys are pressed
		for(col=0;col<COLS;col++){
			for(row=0;row<ROWS;row++){
				ticks = key_ticks[row][col];
				key = (toggle_mode) ? map_toggle[row][col] : map_default[row][col];
#if DEBUG == 2
				phex(key_history[row][col]);
#endif
				if(key_history[row][col] == STILL_ON){
					if (key == KEY_TOGGLE) continue; //Skip toggle buttons
					if( key > 223 && key < 232 ){
						keyboard_modifier_keys |= 1<<(key-224); //Could also change USB report to do this
					} else if (key >= 232 && key < 255){ //Mouse Concerns
						if      (ticks > 120 ) delta = 4;
						else if (ticks > 80 ) delta = 3;
						else if (ticks > 40  ) delta = 2;
						else                   delta = 1;

						if (key == MOUSE_N) y -= delta;
						if (key == MOUSE_S) y += delta;
						if (key == MOUSE_E) x += delta;
						if (key == MOUSE_W) x -= delta;
						if (key == MOUSE_SCROLL_UP && ticks % MOUSE_SPEED == 1) wheel = delta;
						if (key == MOUSE_SCROLL_DN && ticks % MOUSE_SPEED == 1) wheel = -delta;
						if (key == MOUSE_BTNL) mouse_left = 1;
						if (key == MOUSE_BTNM) mouse_middle = 1;
						if (key == MOUSE_BTNR) mouse_right = 1;

						if (key == MACRO_1 && macro_1_ticks == 0){
							macro_1_ticks = 1;
						}
					} else { //Not a special case, just boring old keypress
						if (i<7){ //Throwing away more than 6, damn usb :(
							keyboard_keys[i++] = key;
						}
					}
				}
			}
#if DEBUG == 2
			print("\n");
#endif
		}
#if DEBUG == 2
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

struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name) {
    struct fat_dir_entry_struct file_entry;
    if(!find_file_in_dir(fs, dd, name, &file_entry)){
        return 0;
    }

    return fat_open_file(fs, &file_entry);
}

uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry) {
    while(fat_read_dir(dd, dir_entry)) {
        if(strcmp(dir_entry->long_name, name) == 0) {
            fat_reset_dir(dd);
            return 1;
        }
    }

    return 0;
}

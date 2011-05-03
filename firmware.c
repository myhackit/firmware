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

#define MAP_SIZE 137

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
	{"MOUSE_N",MOUSE_N},   {"MOUSE_NE",MOUSE_NE}, {"MOUSE_E",MOUSE_E},
	{"MOUSE_SE",MOUSE_SE}, {"MOUSE_S",MOUSE_S},   {"MOUSE_SW",MOUSE_SW},
	{"MOUSE_W",MOUSE_W},   {"MOUSE_NW",MOUSE_NW},
	//5 Mouse Controls
	{"MOUSE_BTNL",MOUSE_BTNL}, {"MOUSE_BTNM",MOUSE_BTNM}, {"MOUSE_BTNR",MOUSE_BTNR},
	{"MOUSE_SCROLL_UP",MOUSE_SCROLL_UP}, {"MOUSE_SCROLL_DN",MOUSE_SCROLL_DN},
};

uint16_t lookup_id(const char* id){
	for(int i=0;i<MAP_SIZE;i++) if(!strcmp(id,str_map[i].name)) return str_map[i].id;
	return 0;
}




char map_def[ROWS][COLS] = {
	//LEFT SIDE
	{ KEY_ESC       , KEY_5          , KEY_2          , KEY_3          , KEY_4          , KEY_5          , 0              }, 
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
char map_tog[ROWS][COLS] = {
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
//
//


void prrint(char* line){
	for(uint8_t i=0;i<strlen(line);++i){
		usb_debug_putchar(line[i]);
	}
}

char *tok;
uint8_t which_map, map_col, map_row = 0;
void parse_layout(char* line){
	if(strcmp(line,"[MAIN]")==0){
		which_map = 0;
		map_row = 0;
	} else if(strcmp(line,"[TOGGLE]")==0){
		which_map = 1;
		map_row = 0;
	} else { //Have full line, let's toke
		map_col = 0;
		tok = strtok(line,",");
		while(tok != NULL){
			if(which_map == 0) map_def[map_row][map_col] = lookup_id(tok);
			if(which_map == 1) map_tog[map_row][map_col] = lookup_id(tok);
			tok = strtok(NULL,",");
			map_col++;
		}
		map_row++;
	}
}

uint8_t macro_idx = 0;
void parse_macro(char* line){
	if(memcmp(line,"[MACRO_",7)==0){
		macro_idx = line[7] - '0' - 1;
	} else { //Have full line, let's toke
		tok = strtok(line,",");
		while(tok != NULL){
			prrint(tok);
			tok = strtok(NULL,",");
		}
	}
}

void lineize(struct fat_file_struct* fd, void (*process_line)(char*) ){
	uint8_t read_buffer[32];
	uint8_t line[300];
	uint8_t pos = 0;

	while(fat_read_file(fd, read_buffer, sizeof(read_buffer)) > 0){
		for(uint8_t i=0;i<sizeof(read_buffer);++i){
			switch (read_buffer[i]) {
				case '\r': case '\t': case ' ': //Ignore white space
				continue;
				case '\n': //How to detect missing newline before EOF?
					if(pos == 0) continue; //Ignore blank lines
					line[pos] = '\0';
					process_line(line);
					pos = 0;
				continue;
			}
			line[pos++] = read_buffer[i];
		}
	}
}

int main(void) {

	uint8_t row, col, i, j, key, delta;
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

	struct fat_file_struct* fd = open_file_in_dir(fs, dd, "myhackit");
#if DEBUG
	if(!fd){
		print("error opening file\n");
	}
#endif

	lineize(fd, parse_layout);
	fat_close_file(fd);

	fd = open_file_in_dir(fs, dd, "macros");
	lineize(fd, parse_macro);
	fat_close_file(fd);

	while (1) {
		//Read in the current key state
		which_map = 0;
		for(row=0;row<10;row++){
			if(row<5){
				PORTD &= ~(1<<row);
			} else {
				PORTF &= ~(1<<(row-5));
			}

			for(col=0;col<COLS;col++){
				if ((PINC & (1<<col)) ^ (!ASSERTED)) key_history[row][col] |= 1;
				key_history[row][col] <<= 1;

				switch (key_history[row][col]){
					case JUST_SWITCHED:
						key_ticks[row][col] = 1;
					break;
					case STILL_ON:
						key_ticks[row][col]  += 1;
						if(map_def[row][col] == KEY_TOGGLE){
							which_map = 1;  //Use the alternate map
						}
					break;
					default:
						key_ticks[row][col] = 0;
					break;
				}
			}
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
				key = (which_map) ? map_tog[row][col] : map_def[row][col];
#if DEBUG == 2
				phex(key_history[row][col]);
#endif
				if(key_history[row][col] == STILL_ON){
					if (key == KEY_TOGGLE) continue; //Skip toggle buttons
					if( key >= KEY_CTRL && key <= KEY_RIGHT_GUI ){
						keyboard_modifier_keys |= 1<<(key-224); //Could also change USB report to do this
					} else if (key >= MOUSE_N && key <= MOUSE_SCROLL_DN){ //Mouse Concerns
						if      (ticks > 120 ) delta = 4;
						else if (ticks > 80 )  delta = 3;
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
					} else if (key >= MACRO_1 && key <= MACRO_8){ //Macro Concerns
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

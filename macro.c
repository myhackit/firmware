#include <avr/io.h>
#include "macro.h"
#include <string.h>
#include <stdlib.h>
#include "print.h"

uint16_t macro_idx = 0;
uint16_t macro_ticks[8] = {0,0,0,0,0,0,0,0};
macro_state macro_current[8][3] = {
	{{0,0,0}}, //idx,key,left
	{{0,0,0}},
	{{0,0,0}},
	{{0,0,0}},
	{{0,0,0}},
	{{0,0,0}},
	{{0,0,0}},
	{{0,0,0}},
};
	
uint16_t macro_sizes[8] = {0,0,0,0,0,0,0,0};
uint16_t macro_steps[8][20];
char *rest, *token, *ptr;
char *rest2, *token2, *ptr2;

void parse_macro(char* line){
	uint8_t action, length, i, j = 0;
	if(memcmp(line,"[MACRO_",7)==0){
		macro_idx = line[7] - '0' - 1;
	} else { 
		ptr = line;
		j=0;
		while((token = strtok_r(ptr, ",", &rest)) != NULL) {
			ptr2 = token;
			i = length = action = 0;
			while((token2 = strtok_r(ptr2, ":", &rest2)) != NULL) {
				if(i==0){
					length = atoi(token2);
				} else if(i == 1){
					action = lookup_id(token2);
				}
				i++;
				ptr2 = rest2;
			}
			phex(action); phex(length); print("\n");
			macro_steps[macro_idx][(j*2)] = action;
			macro_steps[macro_idx][(j*2)+1] = length;
			ptr = rest;
			j++;
		}
		macro_sizes[macro_idx] = j;
		//for(j=0;j<macro_sizes[macro_idx];){
		//	phex(j);phex(macro_steps[macro_idx][j++]); phex(macro_steps[macro_idx][j++]);print(" ");
		//}
	}
}
void run_macros(uint8_t keyboard_keys[], uint8_t idx){
		//increment macro tick counter
		for(macro_idx=0; macro_idx<8; macro_idx++){
			if(!macro_current[macro_idx]->ticks_left) continue; // Macro is not running, ignore

			if(--macro_current[macro_idx]->ticks_left <= 0){ //Just now shifted step
				if((++macro_current[macro_idx]->step) > (macro_sizes[macro_idx]-1) ){ //We are done
					macro_current[macro_idx]->step       = 0;
					macro_current[macro_idx]->action     = 0;
					macro_current[macro_idx]->ticks_left = 0;
					continue; 
				} else { //Go get the new step
					macro_current[macro_idx]->action     = macro_steps[macro_idx][(macro_current[macro_idx]->step*2)];
					macro_current[macro_idx]->ticks_left = macro_steps[macro_idx][(macro_current[macro_idx]->step*2)+1];
				}
			}
			if(macro_current[macro_idx]->action){
				keyboard_keys[idx++] = macro_current[macro_idx]->action;
			}
		}
}
void start_macro(int macro_idx){
	if (macro_current[macro_idx]->ticks_left == 0){
		macro_current[macro_idx]->step       = 0; // index
		macro_current[macro_idx]->action     = macro_steps[macro_idx][0]; // key
		macro_current[macro_idx]->ticks_left = macro_steps[macro_idx][1]; // time_left
	}
}

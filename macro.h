#ifndef macro_h__
#define macro_h__

uint16_t macro_idx;
typedef struct{
	int step;  // index + 1
	int action;  // what to do
	int ticks_left; // how long is left in this step
} macro_state;
macro_state macro_current[8][3];
uint16_t macro_sizes[8];
uint16_t macro_steps[8][20];
void run_macros(uint8_t[], uint8_t); //TODO: Pointer/deref etc
void start_macro(int);

#endif //macro_h__

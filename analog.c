#include <avr/io.h>
#include "print.h"
#include "analog.h"

AnalogData analogs[NUM_ANALOGS] = {
	{0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0},
	{2,0,0,0,0,0,0},
	{3,0,0,0,0,0,0},
};

void analog_update(int do_dead){
	for(int i=0; i<NUM_ANALOGS; i++){
		analogs[i].latest = adc_read(analogs[i].pin);
		if(analogs[i].latest < analogs[i].min){
			analogs[i].min = analogs[i].latest;
		}
		if(analogs[i].latest > analogs[i].max){
			analogs[i].max = analogs[i].latest;
		}

		if(do_dead){
			if (analogs[i].latest <= analogs[i].dead_min){
				analogs[i].dead_min = analogs[i].latest - DEADZONE;
			}
			if (analogs[i].latest >= analogs[i].dead_max){
				analogs[i].dead_min = analogs[i].latest + DEADZONE;
			}
		}

		if(analogs[i].latest <= analogs[i].dead_min){
			analogs[i].moving = (analogs[i].dead_min - analogs[i].latest)/ANALOG_SENSITIVITY;
		}
		else if(analogs[i].latest >= analogs[i].dead_max){
			analogs[i].moving = (analogs[i].dead_max - analogs[i].latest)/ANALOG_SENSITIVITY;
		}
		else {
			analogs[i].moving = 0;
		}
	}
}
void analog_autocalibrate_center(){
	analog_update(0);
	// Presume that the deadzone is the latest reading
	for(int i=0; i<NUM_ANALOGS; i++){ 
		analogs[i].dead_min = analogs[i].dead_max = analogs[i].latest;
	}

	// Now lets take a few samples to see how much the resting position moves
	for(int i=0;i<32;i++){
		analog_update(1);
	}
}

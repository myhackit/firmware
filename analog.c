#include <avr/io.h>
#include "print.h"
#include "analog.h"

AnalogData analogs[NUM_ANALOGS] = {
	{0,0,0,0,0,0,0}, //R2X
	{1,0,0,0,0,0,0}, //R2Y
	{3,0,0,0,0,0,0}, //L1X
	{2,0,0,0,0,0,0}, //L2Y
};

void analog_update(int do_dead){
	for(int i=0; i<NUM_ANALOGS; i++){
		analogs[i].latest = adc_read(analogs[i].pin);

		// min/max are not used yet, but might be useful when tuning as we will have an idea what "100%" is
		if(analogs[i].latest < analogs[i].min){
			analogs[i].min = analogs[i].latest;
		}
		if(analogs[i].latest > analogs[i].max){
			analogs[i].max = analogs[i].latest;
		}

		// How the 'deadzone' works
		// MIN<----------(Min/DEADZONE/Max)+++++++++++>MAX
		if(do_dead){
			if (analogs[i].latest <= analogs[i].dead_min){
				analogs[i].dead_min = analogs[i].latest - DEADZONE_RADIUS;
			}
			if (analogs[i].latest >= analogs[i].dead_max){
				analogs[i].dead_min = analogs[i].latest + DEADZONE_RADIUS;
			}
		} else {
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
} // analog_update

void analog_autocalibrate_center(){
	analog_update(0);
	// Presume that the deadzone _is_ the latest reading
	for(int i=0; i<NUM_ANALOGS; i++){ 
		analogs[i].dead_min = analogs[i].dead_max = analogs[i].latest;
	}

	// Now lets take a few samples to see how much the resting position moves
	for(int i=0;i<32;i++){
		analog_update(1);
	}
} // analog_autocalibrate_center

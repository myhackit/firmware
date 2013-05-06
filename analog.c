#include <avr/io.h>
#include "print.h"
#include "analog.h"

AnalogData analogs[NUM_ANALOGS] = {
	{0,0,450,590,160,860}, //LY
	{1,0,560,650,290,900}, //LX
	{3,0,470,580,180,800}, //RY
	{2,0,560,610,260,880}, //RX
};
AnalogData *curr;
uint16_t latest;

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

void analog_update(int do_calibrate){
	for(int i=0; i<NUM_ANALOGS; i++){
		curr = &analogs[i];
		latest = adc_read(curr->pin);

		if(latest < curr->min) curr->min = latest;
		if(latest > curr->max) curr->max = latest;

		// How the 'deadzone' works
		// MIN<----------(Min/DEADZONE/Max)+++++++++++>MAX
		if(do_calibrate){
			if (latest <= curr->dead_min){
				curr->dead_min = (latest - DEADZONE_RADIUS);
			}
			if (latest >= curr->dead_max){
				curr->dead_max = (latest + DEADZONE_RADIUS);
			}
		} else {
			if(latest < curr->dead_min){
				curr->moving = (int) (curr->dead_min - latest);
			}
			else if(latest > curr->dead_max){
				curr->moving = (int) -(latest - curr->dead_max);
			}
			else {
				curr->moving = 0;
			}
		}
		/*
		print("Latest:");   (latest<0)           ? print("-") : print(" "); phex16(latest);
		print(" Delta:");   (curr->moving < 0)   ? print("-") : print(" "); phex16(curr->moving);
		print(" Limits:("); (curr->min < 0)      ? print("-") : print(" "); phex16(curr->min);
		print(",");         (curr->dead_min < 0) ? print("-") : print(" "); phex16(curr->dead_min);
		print(",");         (curr->dead_max < 0) ? print("-") : print(" "); phex16(curr->dead_max);
		print(",");         (curr->max < 0)      ? print("-") : print(" "); phex16(curr->max);
		print(")\n");
		*/
	}
} // analog_update

void analog_autocalibrate_center(){
	// Lets take a few samples to see how much the resting position moves
	for(int i=0; i<100; i++){
		analog_update(1);
	}
} // analog_autocalibrate_center

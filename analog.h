#ifndef analog_h__
#define analog_h__

#define NUM_ANALOGS         4
#define DEADZONE_RADIUS     20
#define ANALOG_SPEED        50
#define ANALOG_SENSITIVITY  100

typedef struct{
	int pin;
	uint16_t latest; //TODO: FLOATS?
	uint16_t moving; 
	uint16_t max;
	uint16_t dead_max;
	uint16_t dead_min;
	uint16_t min;
} AnalogData;
AnalogData analogs[NUM_ANALOGS];

void analog_update(int do_dead);
void analog_autocalibrate_center(void);

#endif

#ifndef analog_h__
#define analog_h__

#define NUM_ANALOGS         4
#define DEADZONE_RADIUS     3
#define ANALOG_SPEED        50

typedef struct{
	int pin;
	int16_t moving;
	int16_t dead_min;
	int16_t dead_max;
	int16_t min;
	int16_t max;
} AnalogData;
AnalogData analogs[NUM_ANALOGS];

void analog_update(int do_dead);
void analog_autocalibrate_center(void);

#endif

#include "demokit.h"

#define timers2ms(tlow,thigh)(tlow | (thigh<<16)) >> 5

int ticksSpeed;
unsigned int lastTime;
unsigned int timeCounted;
int clockStopped;

void demoInit(void)
{
	reStartRealTicks();
	reStartTicks();
}

void reStartRealTicks(void)
{
	TIMER2_DATA=0;
	TIMER3_DATA=0;
	TIMER2_CR=TIMER_ENABLE | TIMER_DIV_1024;
	TIMER3_CR=TIMER_ENABLE | TIMER_CASCADE;
}

unsigned int getRealTicks(void)
{
	return timers2ms(TIMER2_DATA, TIMER3_DATA);
}

void reStartTicks(void)
{
	ticksSpeed = 100;
	clockStopped = 0;
	setTicksTo(0);
}

void startTicks(void)
{
	if (clockStopped) {
		clockStopped = 0;
		lastTime = getRealTicks();
	}
}

void stopTicks(void)
{
	if (!clockStopped) {
		clockStopped = 1;
		getTicks();
	}
}

void setTicksTo(unsigned int time)
{
	timeCounted = time;
	lastTime = getRealTicks();
}

unsigned int getTicks(void)
{
	unsigned int t = ((getRealTicks() - lastTime)*ticksSpeed)/100;
	if ((t > 0) || (-t < timeCounted)) {
		timeCounted += t;
	} else {
		timeCounted = 0;
	}
	lastTime = getRealTicks();
	return timeCounted;
}

void setTicksSpeed(int percentage)
{
	getTicks();
	ticksSpeed = percentage;
}

int getTicksSpeed(void)
{
	return ticksSpeed;
}

void delay(unsigned int d) {
	unsigned int start = getTicks();
	while (getTicks() <= start+d);
}


int my_rand(void)
{
	static int seed = 2701;
	return seed = ((seed * 1103515245) + 12345) & 0x7fffffff;
}


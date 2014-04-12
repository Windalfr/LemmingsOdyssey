#ifndef _DEMOKIT_H_
#define _DEMOKIT_H_

#include <nds.h>

void demoInit(void);
void reStartRealTicks(void);
unsigned int getRealTicks(void);

void reStartTicks(void);
void startTicks(void);
void stopTicks(void);
void setTicksTo(unsigned int time);
unsigned int getTicks(void);
void setTicksSpeed(int percentage);
int getTicksSpeed(void);
void delay(unsigned int d);

int my_rand(void);

#endif

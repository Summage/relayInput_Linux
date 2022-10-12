#ifndef _INPUT_H_
#define _INPUT_H_

#include <bits/pthreadtypes.h>
#define KEYBOARD_DEV_PATH "/dev/input/event3"
#define MOUCE_DEV_PATH "/dev/input/mice"

pthread_t getInputRegister(void * obj, void (*callback)(void *, int, int), char * logFilePath);
pthread_t relayInputRegister(char * logFilePath);
#endif
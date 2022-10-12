#ifndef _INPUT_H_
#define _INPUT_H_

#define KEYBOARD_DEV_PATH "/dev/input/event3"
#define MOUCE_DEV_PATH "/dev/input/mice"

int keyRegister(void * obj, void (*callback)(void *, int, int), char * logFilePath);
int mouceRegister(void *obj, void (*callback)(void *, int, int));
#endif
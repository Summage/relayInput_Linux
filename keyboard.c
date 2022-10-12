#include <bits/types/struct_timeval.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/types.h>
#include <fcntl.h>
#include "getInput.h"

#define KEYARRAY_LEN 20
#define LOGFILEPATH "./tmp_key.log"
#define LOGFMT "%13ld-%3d-%1d\n"//"%13lld-%3d-%c"
#define LOGLEN 21 // 21

#define isKeyPressed(stat, code) (stat[code/32] & (1u << (code % 32)))
#define setKeyPressed(stat, code) (stat[code/32] |= (1u << (code % 32)))
#define setKeyReleased(stat, code) (stat[code/32] &= ~(1u << (code % 32)))

typedef struct input_event KeyEvent;

typedef struct{
    int fd;
    int logFd;
    void * obj;
    void (*callback)(void *, int, int);
} KeyFile;

typedef struct{
    int fd;
    int len;
    KeyEvent * keyArray;
} KeyLog;

static void creadThread_Detached(void * obj, void (*callback)(void *)){
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, (void*)callback, (void *)obj);
    pthread_attr_destroy(&attr);
}

static void logKeyArray(void * argv){
    if(argv == NULL)
        return;
    KeyLog * keyLog = (KeyLog *) argv;
    char buf[LOGLEN] = {0};
    if(keyLog->fd > 0 && keyLog->len > 0 && keyLog->keyArray != NULL){
        for(int i = 0; i < keyLog->fd; ++i){
            sprintf(buf, LOGFMT, keyLog->keyArray[i].time.tv_sec*1000+keyLog->keyArray[i].time.tv_usec, keyLog->keyArray[i].code, keyLog->keyArray[i].value);
            write(keyLog->fd, buf, LOGLEN-1);
        }
        free(keyLog->keyArray);
    }
    free(keyLog);
}

static KeyEvent * flushKeyArray(KeyLog * keyLog, char reuse){
    if(keyLog != NULL)
        creadThread_Detached((void *)keyLog, &logKeyArray);
    if(reuse == 0)
        return NULL;
    KeyEvent * keyArray = (KeyEvent *) calloc(KEYARRAY_LEN, sizeof(KeyEvent));
    if(!keyArray){
        printf("flushKeyArray: failed to alloc space for key event array\r\n");
        pthread_exit((void *)1);
    }
    return keyArray;
}

// F1 59 
static void keyThread(void * argv){
    KeyFile * keyFile = (KeyFile *) argv;
    KeyEvent * keyArray = NULL;
    KeyLog * keyLog = NULL;
    int index = 0, keyStat[8] = {0};
    while(1){
        keyArray = flushKeyArray(keyLog, 1);
        if(keyArray == NULL){
            printf("keyThread: failed to obtain key array\r\n");
            pthread_exit((void *)1);
        }
        for(index = 0; index < KEYARRAY_LEN;){
            if(read(keyFile->fd, keyArray+index, sizeof(KeyEvent)) > 0 && keyArray[index].type == EV_KEY){
                // check and update key`s status
                if(!keyArray[index].value){
                    if(isKeyPressed(keyStat, keyArray[index].code))
                        continue;
                    setKeyPressed(keyStat, keyArray[index].code);
                }else{
                    setKeyReleased(keyStat, keyArray[index].code);
                }

                // log key 
                keyFile->callback(keyFile->obj, keyArray[index].code, keyArray[index].value);
                ++index;
            }
        }
        keyLog = (KeyLog *)calloc(1, sizeof(KeyLog));
        keyLog->fd = keyFile->logFd;
        keyLog->len = index;
        keyLog->keyArray = keyArray;
    }

    // break
    if(keyArray){
        keyLog = (KeyLog *)calloc(1, sizeof(KeyLog));
        keyLog->fd = keyFile->logFd;
        keyLog->len = index;
        keyLog->keyArray = keyArray;
        flushKeyArray(keyLog, 0);
    }
    free(keyFile);
}

int keyRegister(void *obj, void (*callback)(void *, int, int), char * logFilePath){
    if(!callback)
        return -1;
    int fd = open(KEYBOARD_DEV_PATH, O_RDONLY);
    if(fd < 1){
        printf("keyRegister: fialed to open %s \r\n", KEYBOARD_DEV_PATH);
        return -1;
    }
    if(logFilePath == NULL)
        logFilePath = LOGFILEPATH;
    int logFd = open(logFilePath, O_WRONLY | O_CREAT);
    if(logFd < 1){
        printf("keyRegister: fialed to open %s \r\n", logFilePath);
        return -1;
    }

    KeyFile * keyFile = (KeyFile *)calloc(1, sizeof(KeyFile));
    keyFile->fd = fd;
    keyFile->logFd = logFd;
    keyFile->obj = obj;
    keyFile->callback = callback;
    creadThread_Detached(keyFile, &keyThread);
}
#include <bits/types/struct_timeval.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "getInput.h"

#define KEYARRAY_LEN 20
#define LOGFILEPATH "./tmp_key.log"
#define LOGFMT "%13ld-%3d-%1d\n"//"%13lld-%3d-%c"
#define LOGLEN 21 // 21

#define isKeyPressed(stat, code) ((stat[code/32]>>(code%32)) & 1u)
#define setKeyPressed(stat, code) (stat[code/32] |= (1u << (code % 32)))
#define setKeyReleased(stat, code) (stat[code/32] &= ~(1u << (code % 32)))

#define MEMERROR 1
#define INPUTERROR 2

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

static char * logFilePath = NULL;

static void creadThread_Detached(void * obj, void (*callback)(void *)){
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, (void*)callback, (void *)obj);
    pthread_attr_destroy(&attr);
}

// store the keyArray into fd and destroy keyArray and keyLog
static void depositKeyLog(void * argv){
    KeyLog * keyLog = (KeyLog *)argv;
    if(keyLog == NULL)
        return;
    if(keyLog->fd > 0 && keyLog->len > 0 && keyLog->keyArray != NULL){
        char buf[LOGLEN] = {0};
        for(int i = 0; i < keyLog->fd; ++i){
            sprintf(buf, LOGFMT, keyLog->keyArray[i].time.tv_sec*1000+keyLog->keyArray[i].time.tv_usec, keyLog->keyArray[i].code, keyLog->keyArray[i].value);
            write(keyLog->fd, buf, LOGLEN-1);
        }
        free(keyLog->keyArray);
    }
    free(keyLog);
}

//deposit the given keyLog (if not NULL) and return an initialized one
static KeyLog * flushKeyLog(KeyLog * keyLog, int fd){
    if(keyLog != NULL)
        creadThread_Detached((void *)keyLog, &depositKeyLog);
    keyLog = (KeyLog *)calloc(1, sizeof(KeyLog));
    if(!keyLog){
        printf("flushKeyArray: failed to alloc space for key log\n");
        pthread_exit((void *)MEMERROR);
    }
    keyLog->keyArray = (KeyEvent *) calloc(KEYARRAY_LEN, sizeof(KeyEvent));
    if(!keyLog->keyArray){
        printf("flushKeyArray: failed to alloc space for key event array\n");
        pthread_exit((void *)MEMERROR);
    }
    keyLog->len = 0;
    keyLog->fd = fd;
    return keyLog;
}

// F1 59 
static void keyThread(void * argv){
    if(argv == NULL){
        printf("keyThread: param keyFile is empty!\n");
        pthread_exit((void *)INPUTERROR);
    }

    KeyFile * keyFile = (KeyFile *) argv;
    KeyLog * keyLog = NULL;
    int keyStat[8] = {0};
    while(1){
        keyLog = flushKeyLog(keyLog, keyFile->logFd);
        for(keyLog->len = 0; keyLog->len < KEYARRAY_LEN; ){
            if(read(keyFile->fd, keyLog->keyArray+keyLog->len, sizeof(KeyEvent)) > 0 && keyLog->keyArray[keyLog->len].type == EV_KEY){
                // check and update key`s status
                if(keyLog->keyArray[keyLog->len].value > 0){
                    if(isKeyPressed(keyStat, keyLog->keyArray[keyLog->len].code))
                        continue;
                    setKeyPressed(keyStat, keyLog->keyArray[keyLog->len].code);
                }else{
                    setKeyReleased(keyStat, keyLog->keyArray[keyLog->len].code);
                }
                
                // log key 
                keyFile->callback(keyFile->obj, keyLog->keyArray[keyLog->len].code, keyLog->keyArray[keyLog->len].value);
                ++keyLog->len;
            }
        }
    }

}


pthread_t  keyRegister(void *obj, void (*callback)(void *, int, int), char * logFilePath){
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
    chmod(logFilePath, (7ull<<6)+(7ull<<3)+7);

    KeyFile * keyFile = (KeyFile *)calloc(1, sizeof(KeyFile));
    keyFile->fd = fd;
    keyFile->logFd = logFd;
    keyFile->obj = obj;
    keyFile->callback = callback;
    pthread_t thread;
    pthread_create(&thread, NULL, (void *)&keyThread, (void *)keyFile);
    return thread;
}
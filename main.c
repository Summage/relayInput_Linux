//  #include <stdio.h>
//  #include <stdlib.h>
//  #include <getopt.h>
 
//  int main(int argc, char ** argv){
//    const struct option table[] = {
//     {"batch"    , no_argument      , NULL, 'b'},
//     {"log"      , required_argument, NULL, 'l'},
//     {"diff"     , required_argument, NULL, 'd'},
//     {"port"     , required_argument, NULL, 'p'},
//     {"help"     , no_argument      , NULL, 'h'},
//     {"ftrace"   , required_argument, NULL, 'f'},
//     {0          , 0                , NULL,  0 },
//    };
 
//   int o;
//   while ( (o = getopt_long(argc, argv, "-bhl:d:p:", table, NULL)) != -1) {
//     switch (o) {
//       case 'b': printf("b:%s\n", optarg); break;
//       case 'p': printf("p:%s\n", optarg);break;
//       case 'l': printf("l:%s\n", optarg); break;
//       case 'd': printf("d:%s\n", optarg); break;
//       case 'f': printf("f:%s\n", optarg); break;
//       case 1: printf("1:%s\n", optarg);return 0;
//       default:
//         printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
//         printf("\t-b,--batch              run with batch mode\n");
//         printf("\t-l,--log=FILE           output log to FILE\n");
//         printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
//         printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
//         printf("\t-f,--ftrace=ELFFILE     parse ELFFILE\n");
//         printf("\n");
//         exit(0);
//     }
//   }
//   return 0;
// }
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "getInput.h"
#include <fcntl.h>

void key_callback(void * obj, int key, int type){
    printf("key %d type %d\r\n", key, type);// 0 release 1 press 2 hold
}

int main(){
    pthread_t keyThread = keyRegister(NULL, &key_callback, "./build/key.log");
    printf("start recoding!\n");
    pthread_join(keyThread, NULL);
    // FILE * file = fopen("./a.log", "w");
    // int fd = open("./a.log", O_WRONLY | O_CREAT);
    // char buf[21];
    // sprintf(buf, "%13ld-%3d-%1d\n", 1l, 1, 1);
    // for(int i = 0; i++< 3; )
    //     write(fd, buf, 20);
    // // fclose(file);
    return 0;
}
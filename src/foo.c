#include <stdlib.h>
#include <stdio.h>
#include<unistd.h>


int main(int argc, char * argv[]){
    sleep(30);
    for (int i = 0; i < argc; ++i){
        printf("argv %d is %s\n",i,argv[i]);
    }
    printf("foo\n");
    exit(0);
}
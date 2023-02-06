/*
Requirements:
-makefile
-Readme.txt
-Prompt for user input
-Run programs in the background while still allowing new input
bg <path> -> runs program in the background
bgkill pid -> sends the TERM signal to the job with id pid to terminate the job
bgstop pid -> sends the STOP signal ...
bgstart pid -> sends the CONT signal ...
-The program must indicate to the user when background jobs have been terminated.
pstat pid -> lists information on process id pid
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "PMan.h"

int handleWait(pid_t pid){
    int status;
    //checks if process finished executing
    if(waitpid(pid,&status,WNOHANG) > 0){
        //if exited normally
        if(WIFEXITED(status)){
            printf("Child: %d - Exited Normally\n",pid); 
        } else{
            printf("Child: %d - Exited Abormally\n",pid); 
        }
        return 0;
    }
    return 0;
}

int checkProcesses(struct node ** head){
    applyFunction(head,handleWait);
}

int bg(char * program, char ** args){
    //run program
    printf("executing program %s\n", program);

    if(execvp(program, args) < 0){
        //error handling for failed execution
        printf("An error occured with execvp\n");
        exit(0);
    }  
}

int executeCMD(struct node ** head, char * args[]){
    pid_t curProcess;
        //BG COMMAND
    if(strcmp(args[0],"bg") == 0){

        curProcess = fork();

        //if process is CHILD
        if(curProcess == 0){

            //attempt to execute bg program
            bg(args[1], args + 1);
        
        //if ERROR
        } else if(curProcess < 0){
            //error handle
        } else{
            //add process to linked list
            appendNode(head,curProcess);
        }
    } else if(strcmp(args[0],"bgkill") == 0){

    }else if(strcmp(args[0],"bgstop") == 0){
        
    }else if(strcmp(args[0],"bgstart") == 0){
        
    }else if(strcmp(args[0],"pstat") == 0){
        
    }else if(strcmp(args[0],"exit") == 0){
        return 1;
    }
    return 0;
}

int main(){
    //Initiate Linked List with processes
    struct node * head = NULL;
    char input[MAX_ARG_LEN];
    char args[50][MAX_ARG_LEN];
    int argsLen;
    int i;
    int done = 0;
    
    while(!done){
        char input[MAX_ARG_LEN];
        char **words = NULL;
        int num_words = 0;

        printf("PMAN > ");
        //store input
        fgets(input,sizeof(input),stdin);
        
        char *token = strtok(input, " \n");
        while (token != NULL) {
            words = realloc(words, (num_words + 1) * sizeof(char*));
            words[num_words] = malloc((strlen(token) + 1) * sizeof(char));
            strcpy(words[num_words], token);
            num_words++;
            token = strtok(NULL, " \n");
        }

        words = realloc(words, (num_words + 1) * sizeof(char*));
        words[num_words] = NULL;

        done = executeCMD(&head, words);

        for (int i = 0; words[i] != NULL; i++) {
            free(words[i]);
        }
        free(words);

        checkProcesses(&head);
    }
    
    //process for PARENT (after error handling)
    //call waitpid with WNOHANG on every pid in linked list
    checkProcesses(&head);

    destroyList(head);

    wait(NULL);

    exit(0);
}
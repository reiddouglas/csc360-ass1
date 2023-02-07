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
#include <sys/signal.h>

#include "PMan.h"

int handleWait(struct node ** head, pid_t pid){
    int status;
    //checks if process finished executing
    if(waitpid(pid,&status,WNOHANG) > 0){
        //prints exit status
        if(WIFEXITED(status)){
        } else{
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
    if(program == NULL){
        printf("Error: missing program argument\n");
        exit(0);
    }
    if(sizeof(program) > 0){
        char * oldProgram = malloc(sizeof(program));
        strcpy(oldProgram,program);
        program = realloc(program, 2 * sizeof(char) + sizeof(program));
        strcpy(program,"./");
        strcat(program,oldProgram);
        free(oldProgram);
        printf("Executing program: %s\n",program);
    }
    if(execvp(program, args) < 0){
        //error handling for failed execution
        printf("Error: could not execute program\n");
        exit(0);
    } 
    exit(0);
}

int pidPath(struct node ** head, pid_t pid){

    char * command = (char *)malloc(45 * sizeof(char) + 2 * sizeof(pid_t));
    sprintf(command,"ls -l /proc/%d/exe | awk '{print \"%d:   \" $NF}'",pid,pid);

    int output = system(command);
    if(output != 0){
        //signal error
        return 1;
    }
    free(command);
    return 0;
}

int bglist(struct node ** head){
    applyFunction(head,pidPath);
    printf("Total background jobs: %d\n",size(head));
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
    }else if(strcmp(args[0],"bglist") == 0){

        checkProcesses(head);
        bglist(head);

    }else if(strcmp(args[0],"bgkill") == 0){
        if(args[1] == NULL){
            printf("Error: pid not defined\n");
        }
        if(kill((pid_t)strtol(args[1],NULL,10),SIGTERM) < 0){

            printf("An error occured with bgkill\n");
            //error handle
        }
    }else if(strcmp(args[0],"bgstop") == 0){
        if(args[1] == NULL){
            printf("Usage:\n bgstop pid\n");
        } else if(args[2] != NULL){
            printf("Error: unexpected argument\n")
        }
        if(kill((pid_t)strtol(args[1],NULL,10),SIGSTOP) < 0){
            
            printf("An error occured with bgstop\n");
            //error handle
        }
    }else if(strcmp(args[0],"bgstart") == 0){
        if(args[1] == NULL){
            printf("Error: pid not defined\n");
        }
        if(kill((pid_t)strtol(args[1],NULL,10),SIGCONT) < 0){
            
            printf("An error occured with bgstart\n");
            //error handle
        }
    }else if(strcmp(args[0],"pstat") == 0){

    }else if(strcmp(args[0],"exit") == 0){
        return 1;
    }else{
        printf("Invalid Input\n");
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

        sleep(1);

        printf("PMAN > ");
        //store input in an array of arguments
        fgets(input,sizeof(input),stdin);

        if(input[0] == '\n' && input[1] == '\0'){
            printf("Invalid Input\n");
        }else{        
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

            //look for command to execute with child process & execute it
            //returns 1 on exit.
            done = executeCMD(&head, words);

            //free array of arguments
            for (int i = 0; words[i] != NULL; i++) {
                free(words[i]);
            }
            free(words);
        }

        //check if processes have finished executing (zombie) and handles them.
        checkProcesses(&head);
    }
    //kill/exit all child processes and dismantle the linked list

    checkProcesses(&head);

    destroyList(head);

    wait(NULL);

    exit(0);
}
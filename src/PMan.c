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
#include <limits.h>
#include <errno.h>
#include <time.h>

#include "PMan.h"

int killChild(struct node * curNode){
    if(kill(curNode->data, SIGTERM) == -1 && errno == ESRCH){
        printf("Error: could not terminate process with id: %d (Does not exist)\n",curNode->data);
        return FALSE;
    }else{
        printf("Killed process with id: %d\n",curNode->data);
        return TRUE;
    }
}

//lol
int killChildren(struct node ** head){
    popNodes(head,killChild);
}

int isPid(const char *str) {
    char *endptr;
    errno = 0;
    long int pid = strtol(str, &endptr, 10);
    //checks if the entire string was converted
    if (endptr == str || *endptr != '\0') return FALSE;
    //checks for overflow/underflow errors
    if (errno == ERANGE && (pid == LONG_MAX || pid == LONG_MIN)) return FALSE;
    //checks if pid is within the suitable range of values
    if (pid < 0 || pid > INT_MAX) return FALSE;
    return TRUE;
}

int isZombie(struct node * curNode){
    int status;
    //checks if process finished executing
    if(waitpid(curNode->data,&status,WNOHANG) > 0) return TRUE;
    return FALSE;
}

int handleZombies(struct node ** head){
    popNodes(head,isZombie);
}

int bg(char * program, char ** args){
    //checks if program argument exists
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

int pidPath(struct node * node){

    char * command = (char *)malloc(45 * sizeof(char) + 2 * sizeof(pid_t));
    sprintf(command,"ls -l /proc/%d/exe | awk '{print \"%d:   \" $NF}'",node->data,node->data);

    // int output = system(command);
    if(system(command) != 0){
        //signal error
        return 1;
    }
    free(command);
    return 0;
}

void bglist(struct node ** head){
    applyFunction(head,pidPath);
    printf("Total background jobs: %d\n",size(head));
}

int bgkill(char ** args){
    //check if pid has been inputted
    if(args[1] == NULL){   
        printf("Usage: bgkill <pid>\n");
        return 1;
    //check if ONLY the pid has been inputed
    }
     if(args[2] != NULL){
        printf("Error: unexpected argument\n");
        return 1;
    
    }
    //check if pid inputted is of a t_pid type
    if(isPid(args[1]) == FALSE){
        printf("Error: arguement is not a pid\n");
        return 1;
    }
    //check if kill process can be executed on the pid
    else if(kill((pid_t)strtol(args[1],NULL,10),SIGTERM) < 0){
        printf("Error: bgkill could not execute (ensure the pid entered is for a valid child process)\n");
        return 1;
    }
}

int bgstop(char ** args){
    //check if pid has been inputted
    if(args[1] == NULL){   
        printf("Usage: bgstop <pid>\n");
        return 1;
    //check if ONLY the pid has been inputed
    }
     if(args[2] != NULL){
        printf("Error: unexpected argument\n");
        return 1;
    
    }
    //check if pid inputted is of a t_pid type
    if(isPid(args[1]) == FALSE){
        printf("Error: arguement is not a pid\n");
        return 1;
    }
    //check if kill process can be executed on the pid
    else if(kill((pid_t)strtol(args[1],NULL,10),SIGSTOP) < 0){
        printf("Error: bgstop could not execute (ensure the pid entered is for a valid child process)\n");
        return 1;
    }
}

int bgstart(char ** args){
    //check if pid has been inputted
    if(args[1] == NULL){   
        printf("Usage: bgstart <pid>\n");
        return 1;
    //check if ONLY the pid has been inputed
    }
     if(args[2] != NULL){
        printf("Error: unexpected argument after pid\n");
        return 1;
    
    }
    //check if pid inputted is of a t_pid type
    if(isPid(args[1]) == FALSE){
        printf("Error: argument is not a pid\n");
        return 1;
    }
    //check if kill process can be executed on the pid
    else if(kill((pid_t)strtol(args[1],NULL,10),SIGCONT) < 0){
        printf("Error: bgstart could not execute (ensure the pid entered is for a valid child process)\n");
        return 1;
    }
}

void clearStdin(void){
    fflush(stdin);
}


int executeCMD(struct node ** head, char * args[]){
    pid_t curProcess;
    //BG COMMAND
    if(strcmp(args[0],"bg") == 0){

        switch(curProcess = fork());

        //if process is CHILD
        if(curProcess == 0){

            //attempt to execute bg program
            bg(args[1], args + 1);
        
        //if ERROR
        } else if(curProcess < 0){
            printf("Error: unable to fork child\n");
            //error handle
        } else{
            //add process to linked list
            appendNode(head,curProcess);
        }
    }else if(strcmp(args[0],"bglist") == 0){

        handleZombies(head);
        bglist(head);

    }else if(strcmp(args[0],"bgkill") == 0){
        bgkill(args);
    }else if(strcmp(args[0],"bgstop") == 0){
        bgstop(args);
    }else if(strcmp(args[0],"bgstart") == 0){
        bgstart(args);
    }else if(strcmp(args[0],"pstat") == 0){

    }else if(strcmp(args[0],"exit") == 0){
        printf("Exiting program...\n");
        return 1;
    }else{
        printf("Invalid Input\n");
    }
    return 0;
}

int main(){
    //Initiate Linked List
    struct node * head = NULL;
    int done = 0;
    
    while(!done){
        char input[MAX_ARG_LEN];
        char **words = NULL;
        int numWords = 0;

        printf("PMAN > ");
        //store input into an array

        //check if -> fgets failed -> the input is  empty -> input exceeds array size (overflow)
        while(fgets(input,sizeof(input),stdin) == NULL || (input[0] == '\n' && input[1] == '\0') || !strchr(input, '\n')) {
                clearStdin();
                printf("Error: input unacceptable length [acceptable length: 1 to %d]\nPlease try again\nPMAN > ",MAX_ARG_LEN);
                input[strcspn(input, "\n")] = '\0';
                input[0] = '\0';
        }
        //allocate memory for arguments + store them in char ** words
        char *token = strtok(input, " \n");
        while (token != NULL) {
            words = realloc(words, (numWords + 1) * sizeof(char*));
            words[numWords] = malloc((strlen(token) + 1) * sizeof(char));
            strcpy(words[numWords], token);
            numWords++;
            token = strtok(NULL, " \n");
        }

        words = realloc(words, (numWords + 1) * sizeof(char*));
        words[numWords] = NULL;

        //look for command to execute with child process & execute it
        done = executeCMD(&head, words);

        //free array of arguments
        for (int i = 0; words[i] != NULL; i++) {
            free(words[i]);
        }
        free(words);
        usleep(50000);

        //check if processes have finished executing (zombie) and handles them.
        handleZombies(&head);
    }
    //kill/exit all child processes and dismantle the linked list

    killChildren(&head);

    handleZombies(&head);

    //waits for all processes to end (backup)
    while(wait(NULL) != -1);

    exit(0);
}
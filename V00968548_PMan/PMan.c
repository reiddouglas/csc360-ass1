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

/*
 * Function:  killChild 
 * --------------------
 * sends a sigterm signal to the process held by a node
 *
 *
 *  curNode: the node holding the process to which the SIGTERM signal will be sent
 *
 *  returns: TRUE (1) if the SIGTERM was successfully sent, FALSE (0) otherwise
 */
int killChild(struct node * curNode){
    if(kill(curNode->data, SIGTERM) == -1 && errno == ESRCH){
        printf("Error: could not terminate process with id: %d (Does not exist)\n",curNode->data);
        return FALSE;
    }else{
        printf("Killed process with id: %d\n",curNode->data);
        return TRUE;
    }
}

/*
 * Function:  killChildren 
 * --------------------
 * sends a sigterm signal to all processes stored in a linked list
 *
 *
 *  head: head of the linked list holding the currently executing processes
 *
 *  returns: none
 */
void killChildren(struct node ** head){
    applyFunction(head,killChild);
}

/*
 * Function:  isPid 
 * --------------------
 * checks if the given string is a valid pid by converting the strin using strtol, checking for a null pointer, checking for underflow/overflow, and whether it is within the valid range for a pid
 *
 *
 *  str: the string to be checked
 *
 *  returns: TRUE (1) if the string represents a pid, FALSE (0) otherwise
 */
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

/*
 * Function:  isZombie 
 * --------------------
 * checks if the process stored in the node has finished executing, but hasn't been cleared from the process table
 * reaps the process if it is a zombie
 *
 *  curNode: the node holding the process being checked
 *
 *  returns: TRUE (1) if the process was a zombie and was successfully reaped, FALSE (0) otherwise
 */
int isZombie(struct node * curNode){
    int status;
    //checks if process finished executing
    if(waitpid(curNode->data,&status,WNOHANG) > 0) return TRUE;
    return FALSE;
}
/*
 * Function:  handleZombies 
 * --------------------
 * clears all zombie nodes from a linked list of processes
 *
 *
 *  head: head of the linked list being reaped
 *
 *  returns: none
 */
void handleZombies(struct node ** head){
    popNodes(head,isZombie);
}
/*
 * Function:  bg 
 * --------------------
 * executed by a child process; attempts to run a program in the same directory by passing the program name and the argv list into execvp
 * if the function encounters an error, it will execute an exit(0) to close the program gracefully
 *
 *  program: pointer to the name of the program being executed (e.g.: foo)
 *  args: the pointer to the argv list for the program being executed; args[0] is the name of the program preceded by a './' (e.g.: ./foo, arg1, arg2)
 *
 *  returns: none
 */
void bg(char * program, char ** args){
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
/*
 * Function:  pidPath 
 * --------------------
 * displays the pid and file path of the program stored in a node in the format:
 * 123: /home/user/a1/foo
 *
 *  node: the node storing the pid being displayed
 *
 *  returns: 1 on failure, 0 on successful execution
 */
int pidPath(struct node * node){
    
    char * filename = (char *)malloc(30 * sizeof(char));
    if (sprintf(filename, "/proc/%d/exe", node->data)< 0){
        printf("Error reading /proc/: pid: %d does not exist\n",node->data);
        return 1;
    }

    char * filepath = (char *)malloc(261 * sizeof(char));
    ssize_t length = readlink(filename, filepath, 261);

    //error check readlink
    if (length < 0) {
        fprintf(stderr, "Error with readlink: %s\n", strerror(errno));
        free(filename);
        free(filepath);
        return 1;
    }

    filepath[length] = '\0';
    printf("%-d: %s\n", node->data, filepath);

    free(filename);
    free(filepath);
    return 0;
}
/*
 * Function:  bglist 
 * --------------------
 * display a list of all the programs currently executing in the background by executing pidPath on every node in a linked list of processes
 * Format:
 * 123: /home/user/a1/foo
 * 456: /home/user/a1/foo
 * Total background jobs: 2
 *
 *
 *  head: head of the linked list of pids
 *
 *  returns: none
 */
void bglist(struct node ** head){
    applyFunction(head,pidPath);
    printf("Total background jobs: %d\n",size(head));
}
/*
 * Function:  bgkill 
 * --------------------
 * sends SIGTERM to the pid stored in args[1]
 *
 *
 *  args: the arguments being passed as a bgkill command, args[0] is the "bgkill" string, args[1] should be the pid
 *
 *  returns: 1 on failure, 0 on successful execution
 */
int bgkill(char ** args){
    //check if pid has been inputted
    if(args[1] == NULL){   
        printf("Usage: bgkill <pid>\n");
        return 1;
    //check if ONLY the pid has been inputed
    }
     if(args[2] != NULL){
        printf("Error: Unexpected argument\n");
        return 1;
    
    }
    //check if pid inputted is of a t_pid type
    if(isPid(args[1]) == FALSE){
        printf("Error: Argument is not a valid pid\n");
        return 1;
    }
    pid_t pid = (pid_t)strtol(args[1],NULL,10);
    //check if kill process can be executed on the pid
    if(kill(pid,SIGTERM) < 0){
        printf("Error: Process %d does not exist.\n",pid);
        return 1;
    }
}
/*
 * Function:  bgstop 
 * --------------------
 * sends SIGSTOP to the pid stored in args[1]
 *
 *
 *  args: the arguments being passed as a bgstop command, args[0] is the "bgstop" string, args[1] should be the pid
 *
 *  returns: 1 on failure, 0 on successful execution
 */
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
        printf("Error: argument is not a pid\n");
        return 1;
    }
    pid_t pid = (pid_t)strtol(args[1],NULL,10);
    //check if kill process can be executed on the pid
    if(kill(pid,SIGSTOP) < 0){
        printf("Error: Process %d does not exist.\n",pid);
        return 1;
    }
}
/*
 * Function:  bgstart 
 * --------------------
 * sends SIGCONT to the pid stored in args[1]
 *
 *
 *  args: the arguments being passed as a bgstart command, args[0] is the "bgstart" string, args[1] should be the pid
 *
 *  returns: 1 on failure, 0 on successful execution
 */
int bgstart(char ** args){
    //check if pid has been inputted
    if(args[1] == NULL){   
        printf("Usage: bgstart <pid>\n");
        return 1;
    }
    //check if ONLY the pid has been inputed
    if(args[2] != NULL){
        printf("Error: unexpected argument after pid\n");
        return 1;
    
    }
    //check if pid inputted is of a t_pid type
    if(isPid(args[1]) == FALSE){
        printf("Error: argument is not a pid\n");
        return 1;
    }
    pid_t pid = (pid_t)strtol(args[1],NULL,10);
    //check if kill process can be executed on the pid
    if(kill(pid,SIGCONT) < 0){
        printf("Error: Process %d does not exist.\n",pid);
        return 1;
    }
}
/*
 * Function:  printStat 
 * --------------------
 * looks through /proc/pid/stat for information on the comm, state, utime, stime, and rss, of a pid and prints them to the console
 *
 *
 *  pid: the process id of the process for which the information is being collected and displayed
 *
 *  returns: 1 on failure, 0 on successful execution
 */
int printStat(pid_t pid){

    char * filename = (char *)malloc(30 * sizeof(char));
    if(sprintf(filename, "/proc/%d/stat", pid) < 0){
        printf("Error: Process %d does not exist.\n",pid);
        free(filename);
        return 1;
    }

    FILE * statInfo = fopen(filename,"r");
    if(statInfo == NULL){
        printf("Error: Could not read proc/\n");
        fclose(statInfo);
        free(filename);
        return 1;
    }
    
    char comm[TASK_COMM_LEN];
    char state;
    unsigned long int utime;
    unsigned long int stime;
    long int rss;


    fscanf(statInfo,"%*d (%255[^)]) %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %*u %ld %*s",comm,&state,&utime,&stime,&rss);

    printf("Comm: %s\n",comm);
    printf("State: %c\n",state);
    printf("Utime: %.2f\n",(float)((float)utime/sysconf(_SC_CLK_TCK)));
    printf("Stime: %.2f\n",(float)((float)stime/sysconf(_SC_CLK_TCK)));
    printf("RSS: %ld\n",rss);

    free(filename);
    free(statInfo);
    return 0;
}
/*
 * Function:  printStatus 
 * --------------------
 * looks through /proc/pid/status for information on the switches of a pid and prints them to the console
 *
 *
 *  pid: the process id of the process for which the information is being collected and displayed
 *
 *  returns: 1 on failure, 0 on successful execution
 */
int printStatus(pid_t pid){

    char * filename = (char *)malloc(32 * sizeof(char));
    if(sprintf(filename, "/proc/%d/status", pid) < 0){
        printf("Error: Process %d does not exist.\n",pid);
        free(filename);
        return 1;
    }

    FILE * statusInfo = fopen(filename,"r");
    if(statusInfo == NULL){
        printf("Error: Could not read proc/\n");
        fclose(statusInfo);
        free(filename);
        return 1;
    }

    char line[MAX_LINE_LEN];
    unsigned long int voluntarySwitches, nonVoluntarySwitches = -1;

    while(fgets(line,sizeof(line),statusInfo)){
        sscanf(line, "voluntary_ctxt_switches: %lu", &voluntarySwitches);
        sscanf(line, "nonvoluntary_ctxt_switches: %lu", &nonVoluntarySwitches);
    } 
    if(voluntarySwitches < 0){
        printf("Error: couldn't retrieve voluntary_ctxt_switches from /prod/%d/status",pid);
    }else{
        printf("Voluntary ctxt switches: %lu\n",voluntarySwitches);
    }
    if(nonVoluntarySwitches < 0){
        printf("Error: couldn't retrieve nonvoluntary_ctxt_switches from /prod/%d/status",pid);
    }else{
        printf("Non-voluntary ctxt switches: %lu\n",nonVoluntarySwitches);
    }

    free(filename);
    free(statusInfo);
    return 0;
}
/*
 * Function:  pstat 
 * --------------------
 * displays information on the comm, state, utime, stime, rss, and switches of a process and prints them to the console
 *
 *  pid: the process id of the process for which the information is being displayed
 *
 *  returns: 1 on failure, 0 on successful execution
 */
int pstat(char ** args){
    
    if(args[1] == NULL){   
        printf("Usage: pstat <pid>\n");
        return 1;
    }

    //check if ONLY the pid has been inputed   
    if(args[2] != NULL){
        printf("Error: unexpected argument after pid\n");
        return 1;
    
    }

    //check if pid inputted is of a t_pid type
    if(isPid(args[1]) == FALSE){
        printf("Error: argument is not a pid\n");
        return 1;
    }

    pid_t pid = (pid_t)strtol(args[1],NULL,10);

    //check if the strtol worked
    if(pid == 0){
        printf("Error: conversion from char * to t_pid failed.\n");
        return 1;
    }

    printStat(pid);
    printStatus(pid);
}
/*
 * Function:  clearStdin 
 * --------------------
 * gets rid of excess input in the console that couldn't be read by fgets() or other input-reading functions
 *
 *
 *  returns: none
 */
void clearStdin(void){
    fflush(stdin);
}
/*
 * Function:  execute CMD 
 * --------------------
 * from a list of passed arguments to PMan, executes the function appropriate to each argument
 * for bg function, forks a child process to execute the appropriate function
 * 
 *
 *  head: head of the linked list of child processes
 *  args: list of arguments being passed to each function
 *
 *  returns: 1 if given "exit" command, 0 otherwise
 */
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

        pstat(args);

    }else if(strcmp(args[0],"exit") == 0){

        printf("PMAN:  > Exiting program...\n");
        return 1;

    }else{
        printf("PMAN:  > %s: command not found\n",args[0]);
    }
    return 0;
}

int main(){
    //Initiate Linked List
    struct node * head = NULL;
    int done = 0;
    
    //loop until user enters "exit" command
    while(!done){
        char input[MAX_ARG_LEN];
        char **words = NULL;
        int numWords = 0;

        printf("PMAN:  > ");
        //store input into an array

        //check if -> fgets failed -> the input is  empty -> input exceeds array size (overflow)
        while(fgets(input,sizeof(input),stdin) == NULL || (input[0] == '\n' && input[1] == '\0') || !strchr(input, '\n')) {
                clearStdin();
                printf("Error: input unacceptable length [acceptable length: 1 to %d]\nPlease try again\nPMAN:  > ",MAX_ARG_LEN);
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

        //look for appropriate function to execute for each command
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

    //kill all children and clear linked list of all zombie processes
    while(!isEmpty(head)){
        //wait for kill signal to reach children before checking for zombie processes
        usleep(1000);
        //clean zombie proccess from linked list (dealloc memory)
        if(!isEmpty(head)) handleZombies(&head);
        //kill any remaining processes
        if(!isEmpty(head)) killChildren(&head);
    }

    exit(0);
}
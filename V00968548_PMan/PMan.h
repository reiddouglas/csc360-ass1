#ifndef HEADER_FILE
#define HEADER_FILE

#define MAX_ARG_LEN 256
#define MAX_LINE_LEN 256
#define FALSE 0
#define TRUE 1
#define TASK_COMM_LEN 20

struct node {
    pid_t data;
    struct node* next;
};
int appendNode(struct node **head, int data);
int applyFunction(struct node ** head, int (*fc)(struct node *));
int popNodes(struct node ** head, int (*fc)(struct node *));
int popNode(struct node** head, pid_t pid);
int destroyList(struct node * head);
int printList(struct node * head);
int size(struct node ** head);
int isEmpty(struct node * head);
#endif
#ifndef HEADER_FILE
#define HEADER_FILE

#define MAX_ARG_LEN 50

struct node;
int appendNode(struct node **head, int data);
int applyFunction(struct node ** head, int (*fc)(pid_t));
pid_t popNode(struct node ** head, pid_t pid);
pid_t peekNode(struct node ** head);
int destroyList(struct node * head);
int printList(struct node * head);
#endif
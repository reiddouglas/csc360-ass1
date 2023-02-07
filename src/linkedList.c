#include <stdio.h>
#include <stdlib.h>
#include "PMan.h"

struct node {
    pid_t data;
    struct node* next;
};



int appendNode(struct node **head, pid_t data){
    struct node * newNode = (struct node *)malloc(sizeof(struct node));
    newNode->data = data;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
        return 0;
    }

    struct node *current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
    return 0;
}

//TODO: error checking apply function

int applyFunction(struct node **head, int (*fc)(struct node **, pid_t)){
    struct node* cur = *head;
    int exit_code = 0;
    if (cur == NULL) {
        return 0;
    }
    while (cur != NULL) {
        if(fc(head, cur->data) != 0){
            exit_code = 1;
        };
        cur = cur->next;
    }
    return 1;
}

pid_t popNode(struct node** head, pid_t pid) {
    struct node* current = *head;
    struct node* prev = NULL;
    pid_t data;
    if (*head == NULL) {
        return 1;
    }
    while (current != NULL) {
        if(current->data == pid){
            data = current -> data;
            if(prev == NULL){
                *head = current->next;
                current->next = NULL;
                free(current);
            } else{
                prev->next = current -> next;
                current->next = NULL;
                free(current);
            }
            return data;
        }
        prev = current;
        current = current->next;
    }
    return 1;
}

pid_t peekNode(struct node** head) {
    struct node* cur = *head;
    struct node* prev = NULL;
    pid_t data;
    if (*head == NULL) {
        return 0;
    }
    while (cur->next != NULL) {
        prev = cur;
        cur = cur->next;
    }
    data = cur->data;
    return data;
}

int destroyList(struct node * head){
    struct node *cur = head;
    struct node *next;
    
    while (cur != NULL) {
        next = cur->next;
        free(cur);
        cur = next;
    }
}

int printList(struct node * head){
    struct node* cur = head;
    while (cur != NULL) {
        printf("Printing: %d\n",cur->data);
        cur = cur->next;
    }
    return 0;
}

int size(struct node ** head){
    struct node * cur = *head;
    int size = 0;
    while (cur != NULL) {
        size ++;
        cur = cur->next;
    }
    return size;
}


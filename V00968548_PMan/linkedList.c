#include <stdio.h>
#include <stdlib.h>
#include "PMan.h"

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

int applyFunction(struct node **head, int (*fc)(struct node *)){
    struct node* cur = *head;
    int exit_code = 0;
    if (cur == NULL) {
        return 0;
    }
    while (cur != NULL) {
        if(fc(cur) != 0){
            exit_code = 1;
        };
        cur = cur->next;
    }
    return 1;
}

pid_t popNodes(struct node** head, int (*fc)(struct node *)) {

    struct node *cur = *head;
    struct node *prev = NULL;
    
    while (cur != NULL) {
        if(fc(cur)){
            if(prev == NULL){
                *head = cur->next;
                free(cur);
                cur = *head;
            }else{
                prev->next = cur->next;
                free(cur);
                cur = prev->next;
            }
        }else{
            prev = cur;
            cur = cur->next;
        }
    }
    return 0;
}

int popNode(struct node** head, pid_t pid) {

    struct node *cur = *head;
    struct node *prev = NULL;
    
    while (cur != NULL) {
        if(cur->data == pid){
            if(prev == NULL){
                *head = cur->next;
                free(cur);
                cur = *head;
                return 0;
            }else{
                prev->next = cur->next;
                free(cur);
                cur = prev->next;
                return 0;
            }
        }else{
            prev = cur;
            cur = cur->next;
        }
    }
    return 1;
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

int isEmpty(struct node * head){
    return (head == NULL);
}
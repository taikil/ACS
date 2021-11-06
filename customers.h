#ifndef CUSTOMERS
#define CUSTOMERS

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

//Customer Struct
typedef struct customer{
    int cid;
    int class_type;
    int service_time;
    int arrival_time;
}customer;

//Customer Node Struct
typedef struct cNode{
    customer *cid;
    double entry_time;
    struct cNode* next;
} cNode;

//Queue Struct
typedef struct cQueue{
    cNode *front;
    cNode *rear;
    int len;
} cQueue;

cNode* newNode();
cQueue* createQueue();
void enQueue(struct cQueue *q, customer *c, double entry_time); //Place customer at back of Queue
cNode* deQueue(struct cQueue *q); // Remove front customer from Queue





#endif //"CUSTOMERS"

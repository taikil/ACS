#include "customers.h"

cNode* newNode(){
    struct cNode* n = (cNode*)malloc(sizeof(cNode));
    if (n == NULL){
        printf("Error, Malloc unsuccessful.\n");
        return NULL;
    }
    return n;
}

cQueue* createQueue(){
    struct cQueue *q = (cQueue*)malloc(sizeof(cQueue));
    if (q == NULL){
        printf("Error, Malloc unsueccessful.\n");
        return NULL;
    }
    q->front = q->rear = NULL;
    q->len = 1;
    return q;
}

void enQueue(cQueue *q, customer *c, double entry_time){
    cNode *temp = newNode();
    temp->cid = c;
    temp->entry_time = entry_time;
    temp->next = NULL;

    if (q->rear == NULL) { 
        q->front = q->rear = temp;
	q->len++;
        return;
    }

    q->rear->next = temp;
    q->rear = temp;
    q->len++;
}

cNode* deQueue(struct cQueue *q){
    if (q->front == NULL)
        return NULL;
 
    // Store previous front an move front one node ahead
    cNode* temp = q->front;
    q->front = q->front->next;
 
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
//    cNode* finished_customer = temp->cid;
 //   free(temp);
    q->len--;
    return temp;
}

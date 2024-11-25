#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

// Define a structure for the node
typedef struct queue_node_t {
    void* data;
    struct queue_node_t* next;
} queue_node_t;

// Define a structure for the queue
typedef struct queue_t {
    queue_node_t* front;
    queue_node_t* rear;
} queue_t;

// Function to create a new node
static inline queue_node_t* create_node(void* value) {
    queue_node_t* newNode = (queue_node_t*)malloc(sizeof(queue_node_t));
    if (newNode == NULL) {
        printf("Memory allocation failed!\n");
        exit(1); // Exit the program if memory allocation fails
    }
    newNode->data = value;
    newNode->next = NULL;
    return newNode;
}

// Function to initialize a queue
static inline queue_t* create_queue() {
    queue_t* q = (queue_t*)malloc(sizeof(queue_t));
    if (q == NULL) {
        printf("Memory allocation failed!\n");
        exit(1); // Exit if memory allocation fails
    }
    q->front = q->rear = NULL;
    return q;
}

// Enqueue operation: Add an element to the rear of the queue
static inline void push(queue_t* q, void* value) {
    queue_node_t* newNode = create_node(value);

    if (q->rear == NULL) {
        // If the queue is empty, both front and rear should point to the new node
        q->front = q->rear = newNode;
        return;
    }

    // Otherwise, add the new node at the end of the queue
    q->rear->next = newNode;
    q->rear = newNode;
}

// Dequeue operation: Remove an element from the front of the queue
static inline void* get(queue_t* q) {
    if (q->front == NULL) {
        printf("Queue is empty!\n");
        return NULL; // Return -1 if the queue is empty
    }

    queue_node_t* temp = q->front;
    void* value = temp->data;

    // Move the front pointer to the next node
    q->front = q->front->next;

    // If the queue is empty after dequeue, set rear to NULL
    if (q->front == NULL) {
        q->rear = NULL;
    }

    // Free the memory of the dequeued node
    free(temp);
    return value;
}

// Function to check if the queue is empty
static inline int is_empty(queue_t* q) {
    return (q->front == NULL);
}

// Function to peek at the front element of the queue (without dequeuing)
static inline void* peek(queue_t* q) {
    if (q->front != NULL) {
        return q->front->data;
    }
    printf("Queue is empty!\n");
    return NULL;
}

// Function to display the contents of the queue
// static inline void displayQueue(Queue* q) {
//     if (q->front == NULL) {
//         printf("Queue is empty!\n");
//         return;
//     }

//     Node* temp = q->front;
//     printf("Queue: ");
//     while (temp != NULL) {
//         printf("%d ", temp->data);
//         temp = temp->next;
//     }
//     printf("\n");
// }


#endif // QUEUE_H

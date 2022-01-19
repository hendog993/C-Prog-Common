#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct Queue_t
{
	size_t write; // write ptr
	size_t read; // read ptr 
	uint32_t* data;
	size_t capacity;
	bool isQueueFull;
} Queue ;

void q_push(Queue* q,
			uint32_t value);
uint32_t q_pop(Queue* q);
uint32_t q_peek(Queue* q);
void q_reset(Queue* q);
size_t q_numMsgsInQueue(Queue* q);
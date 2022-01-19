#include "queue.h"
// Push operation determines is q is full
// Pop operaetion determines is q should be reset.
// Control: Pop only allows a reset if all values have been read. Popping a value 
				// that empties the queue wil reset both read/write points to zero. 

void q_push(Queue* q, 
			uint32_t value)
{
	// if queue is full, exit
	if (((q->write) == (q->capacity - 1))  // The write ptr is at the end.
		&& (true == q->isQueueFull))	    // The write ptr can be at final index and still write only ONE time 
	{
		return; 
	}

	// Here, the queue isn't full, and can stand to push data. 
	q->data[q->write] = value;

	// The write ptr may not be able to be incremeneted if this push is at the final index.
	if ((q->write) == (q->capacity - 1))
	{
		q->isQueueFull = true;
	}
	else
	{
		q->write++;
		q->isQueueFull = false;
	}
}

/* Returns the first FIFO value 
1. Resets the queue if the popped value was the last written value
2. Checks if queue is empty based on read - write, and if the last written value was the at final index
3. Can't pop anything if nothing was pushed in the first place
*/
uint32_t q_pop(Queue* q)
{
	/* If the queue is empty, there is nothing to read */
	if ( q->write == 0 && q->read == 0 )
	{
		return 0; // empty queue 
	}

	// At this point, a value CAN be read. 
	uint32_t returnval = q->data[q->read];

	if ( (q->write == ( q->read )) || // Final pushed value was read 
		( q->isQueueFull && ( q->read == (q->capacity-1))) ) // Final write value was at boundary and final read index is at capacity  
	{
		// Execute here in the condition that the read index is at the final data index and the 
		q_reset(q);
		return returnval;
	}
	// Normal operation 
	q->read++;
	return returnval;
}

/* Reads the first in value but doesn't pop */
uint32_t q_peek(Queue* q, size_t offset)
{
	// Can't peek if read + offset > capacity
	// todo don't like return zero, because zero could be a valid peek value. 
	return ( (q->read + offset) < (q->capacity-1) )? q->data[q->read + offset] : 0 ;
}

// Returns number of pushed entries in queue 
size_t q_numMsgsInQueue(Queue* q)
{
	size_t returnval = (q->isQueueFull) ?  q->capacity :  q->write - q->read;
	return returnval;
}	

void q_reset(Queue* q)
{
	q->read = 0;
	q->write = 0;
	q->isQueueFull = false;
}
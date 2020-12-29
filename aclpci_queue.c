/* 
 * Copyright (c) 2019, Intel Corporation.
 * Intel, the Intel logo, Intel, MegaCore, NIOS II, Quartus and TalkBack 
 * words and logos are trademarks of Intel Corporation or its subsidiaries 
 * in the U.S. and/or other countries. Other marks and brands may be 
 * claimed as the property of others.   See Trademarks on intel.com for 
 * full list of Intel trademarks or the Trademarks & Brands Names Database 
 * (if Intel) or See www.Intel.com/legal (if Altera).
 * All rights reserved
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD 3-Clause license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *      - Neither Intel nor the names of its contributors may be 
 *        used to endorse or promote products derived from this 
 *        software without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Queue of fixed size.
 * Uncomment below to run in user-space unit-test mode.
 * Otherwise, will compile in kernel mode. */
// #define UNIT_TEST_MODE

#include "aclpci_queue.h"

#ifdef UNIT_TEST_MODE
#include <stdlib.h> // for calloc
#include <stdio.h>  // for printf
#include <string.h> // for memcpy
#else
#include "aclpci.h"
#endif


void queue_init (struct queue *q, unsigned int elem_size, unsigned int size) {
  // printk ("queue_init %p, elem_size = %u, size = %u\n", q, elem_size, size);
  if (q == 0) { return; }
  #ifdef UNIT_TEST_MODE
    q->buffer = calloc (size, elem_size);
  #else
    q->buffer = kzalloc (elem_size * size, GFP_KERNEL);
  #endif
  if (q->buffer == 0) {
    printk ("Couldn't allocate queue buffer!\n");
    return;
  }
  q->size = size;
  q->elem_size = elem_size;
  q->count = 0;
  q->out = 0;
}


void queue_fini (struct queue *q) {
  // printk ("queue_init %p\n", q);
  if (q == 0) { return; }
  #ifdef UNIT_TEST_MODE
    free (q->buffer);
  #else
    kfree (q->buffer);
  #endif
  q->buffer = NULL;
  q->size = 0;
  q->elem_size = 0;
  q->count = 0;
  q->out = 0;
}


unsigned int queue_size (struct queue *q) {
  return q->count;
}

int queue_empty(struct queue *q) {
  return (q->count == 0);
}

/* localize ugly casts */
void *queue_addr (struct queue *q, unsigned int offset) {
  unsigned long buffer_loc = (unsigned long)q->buffer + offset * q->elem_size;
  return (void*)buffer_loc;
}

/* When working with the circular buffer, values can wrap around
 * at most once. So instead of doing val % size, can do a simple comparison */
unsigned int fast_mod (unsigned int val, unsigned int size) {
  if (val >= size)
    return val - size;
  else
    return val;
}

void queue_push (struct queue *q, void *e) {
  unsigned int loc;
  void* dest; //add by pxx
  if (q->count == q->size) {
    /* queue is full! */
    return;
  }
  loc = fast_mod ( (q->out + q->count), q->size );
  /* SDL requirement for memcpy()):
   * 1. Never specify source and destination buffers that overlap, as unpredictable program behaviour can result
   * 2. Specify num values that will not cause a buffer overflow:
   *   2.1. Always use unsigned values for num to avoid integer underflows that can lead to a buffer overflow
   *   2.2. Ensure that num does not cause memcpy() to index memory outside the bounds of the destination buffer
   *   2.3. Avoid denial of service vulnerabilities by ensuring that num does not cause memcpy() to index memory outside of the source buffer
   */

  dest = queue_addr(q, loc);
  // The following checks are for memcpy(dest, e, q->elem_size).
  // If there's a failure, print an error message and return from the function without changing anything
  // 1. e and dest can't overlap.
  if ( (e + q->elem_size > dest) ||   //end of e overlap with begin of dest
       (e < dest + q->elem_size) )  { //begin of e overlap with end of dest
    printk("queue_push() failed at memcpy(): Source and Destination buffers overlap");
    return;
  }

  // 2.1. q->elem_size has to be unsigned: Already done
  // 2.2. q->elem_size does not cause dest to index out of bound: Already done since:
  //        loc = fast_mod ( (q->out + q->count), q->size ); //loc is in-bound and is a legit index
  //        dest = queue_addr(q, loc);  //dest is a legit address of an element in q->buffer
  //      So *dest will not be out-of-bound
  // 2.3. q->elem_size does not cause e to index out of bound: Already done since the intended usage of queue_push is for e to be the address of 1 single element
  //      If this is somehow not the case, it's the fault of the caller of queue_push() (most likely a functional failure in the first place), not of queue_push() itself
  memcpy (dest, e, q->elem_size);
  q->count++;
}

void queue_pop (struct queue *q) {
  if (q->count == 0) {
    return;
  }
  q->count--;
  q->out = fast_mod ( (q->out + 1), q->size );
}

void *queue_front (struct queue *q) {
  if (q->count == 0) {
    return NULL;
  }
  return queue_addr (q, q->out);
}

void *queue_back (struct queue *q) {
  if (q->count == 0) {
    return NULL;
  }
  return queue_addr (q, fast_mod( (q->out + q->count - 1), q->size ) );
}


/* Unit tests. */
#ifdef UNIT_TEST_MODE
int main() {
  struct queue q;
  int i, j, k;
  queue_init (&q, sizeof(int), 5);
  i = 1; queue_push(&q, &i);
  i = 2; queue_push(&q, &i);
  i = 3; queue_push(&q, &i);
  j = *(int*)queue_front(&q); k = *(int*)queue_back(&q);
  printf ("%d, %d\n", j, k);
  
  queue_pop(&q);
  j = *(int*)queue_front(&q); k = *(int*)queue_back(&q);
  printf ("%d, %d\n", j, k);
  
  queue_pop(&q);
  queue_pop(&q);
  i = 11; queue_push(&q, &i);
  i = 12; queue_push(&q, &i);
  i = 13; queue_push(&q, &i);
  i = 14; queue_push(&q, &i);
  i = 15; queue_push(&q, &i);
  i = 16; queue_push(&q, &i);
  j = *(int*)queue_front(&q);
  k = *(int*)queue_back(&q);
  printf ("%d, %d\n", j, k);
  
  while (!queue_empty(&q)) {
    int s = *(int*)queue_front(&q); queue_pop(&q);
    printf ("%d\n", s);
  }
  
  return 0;
}
#endif

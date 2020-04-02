/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <nanvix/runtime/rmem.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/ulib.h>
#include <posix/stddef.h>

#define NUM_NODES 100

/*
 * Dijkstra routine from MiBench Benchmark.
 */

/* Foward definitions. */
extern void *nanvix_malloc(size_t size);
extern void nanvix_free(void *ptr);

/* Item struct definition. */
struct node
{
	int dist;
	int prev;
};
struct item
{
	int node;
	int dist;
	int prev;
};

/* Node path. */
struct node path_nodes[NUM_NODES];
/* Queue. */
struct item *queue;
int queue_rear = NUM_NODES*NUM_NODES-1, queue_front = 0;
int queue_size = 0;

/*
 * @brief Enqueues a node with it's distance and previous element.
 *
 * @details Enqueue a new element with the value for node and more information
 * to make a new item. Each item know about the elements next to itself.
 * Moreover, each new item is inserted in the end of the queue by iterating over
 * all known nodes.
 */
int enqueue (int node, int dist, int prev)
{
	if (queue_size == NUM_NODES*NUM_NODES)
		return -1;
	queue_rear = (queue_rear+1)%(NUM_NODES*NUM_NODES);
	queue[queue_rear].node = node;
	queue[queue_rear].dist = dist;
	queue[queue_rear].prev = prev;
	queue_size++;
	return 0;
}


/*
 * @brief Dequeues a node with it's distance and previous element.
 *
 * @details Dequeues a node by eliminating the head. Information about the
 * deleted node will be passed by reference to posterior use.
 */
int dequeue (int *dequeue_node, int *dequeue_dist, int *dequeue_prev)
{
	if (queue_size == 0)
		return -1;
	struct item it = queue[queue_front];
	uprintf("Item it %d %d %d %d %d\n", queue[queue_front], it, it.node, it.dist, it.prev);
	*dequeue_node = it.node;
	*dequeue_dist = it.dist;
	*dequeue_prev = it.prev;
	queue_front = (queue_front+1)%(NUM_NODES*NUM_NODES);
	queue_size--;
	return 0;
}

/*
 * @brief Executes the dijkstra algorithm.
 *
 * @details The algorithm is responsible to initalize the node path and insert
 * the first node in the queue. After this first stage, each node of the queue
 * will be analysed to see if its path will be the lowest possible. The analysis
 * will be done for each node adjacent to queue nodes.
 */
int nanvix_dummy(int example)
{
	queue =  nanvix_malloc(NUM_NODES*NUM_NODES*sizeof(struct item));
	int dequeue_prev, dequeue_node, dequeue_dist;


	/*
	 * In this example it is possible to see that the variable it.prev is not
	 * the same returned and stored inside dequeue_prev.
	 */
	if (example == 1)
	{
		int size = NUM_NODES;
		enqueue (0, 0, size);
		uprintf("Enqueue dnode: %d ddist: %d dprev: %d\n", 0, 0, size);

		dequeue (&dequeue_node, &dequeue_dist, &dequeue_prev);
		uprintf("Dequeue dnode: %d ddist: %d dprev: %d\n", dequeue_node, dequeue_dist, dequeue_prev);
		return 0;
	}

	/*
	 * In this example it is possible to see that the enqueue and dequeue seems
	 * to work.
	 */
	if (example == 2)
	{
		int for_size = NUM_NODES;
		for (int i = 0; i < for_size; i++)
		{
			enqueue (i, i, for_size+i);
			uprintf("Enqueue dnode: %d ddist: %d dprev: %d\n", i, i, for_size+i);
		}
		for (int i = 0; i < for_size; i++)
		{
			dequeue (&dequeue_node, &dequeue_dist, &dequeue_prev);
			uprintf("Dequeue dnode: %d ddist: %d dprev: %d\n", dequeue_node, dequeue_dist, dequeue_prev);
		}
		return 0;
	}

	/*
	 * In this example it is possible to see that the enqueue and dequeue do not
	 * work when more than one table from vmem is used.
	 */
	if (example == 3)
	{
		int for_size = NUM_NODES*NUM_NODES;
		for (int i = 0; i < for_size; i++)
		{
			enqueue (i, i, for_size+i);
			uprintf("Enqueue dnode: %d ddist: %d dprev: %d\n", i, i, for_size+i);
		}
		for (int i = 0; i < for_size; i++)
		{
			dequeue (&dequeue_node, &dequeue_dist, &dequeue_prev);
			uprintf("Dequeue dnode: %d ddist: %d dprev: %d\n", dequeue_node, dequeue_dist, dequeue_prev);
		}
		return 0;
	}
	return -1;
}

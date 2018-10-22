/* Tests cetegorical mutual exclusion with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */
#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "timer.h"
#include "lib/random.h" //generate random numbers

#define BUS_CAPACITY 3
#define SENDER 0
#define RECEIVER 1
#define NORMAL 0
#define HIGH 1


/*
 *  initialize task with direction and priority
 *  call o
 * */
typedef struct {
    int direction;
    int priority;
} task_t;

struct lock thread_lock;
struct condition drive_condition[2][2];
int waiters[2];     
int open_slots; // 0 <= open_slots <= BUS_CAPACITY
int currentDirection;


void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
        unsigned int num_priority_send, unsigned int num_priority_receive);

void senderTask(void *);
void receiverTask(void *);
void senderPriorityTask(void *);
void receiverPriorityTask(void *);



void oneTask(task_t task);/*Task requires to use the bus and executes methods below*/
void getSlot(task_t task); /* task tries to use slot on the bus */
void transferData(task_t task); /* task processes data on the bus either sending or receiving based on the direction*/
void leaveSlot(task_t task); /* task release the slot */



/* initializes semaphores */
void init_bus(void){

    random_init((unsigned int)123456789);
    open_slots = BUS_CAPACITY;

    int i;
    int j;
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2; j++)
        {
            cond_init(&drive_condition[i][j]);
        }
    }

    lock_init(&thread_lock);
    currentDirection=SENDER;

}
/*
 *  Creates a memory bus sub-system  with num_tasks_send + num_priority_send
 *  sending data to the accelerator and num_task_receive + num_priority_receive tasks
 *  reading data/results from the accelerator.
 *
 *  Every task is represented by its own thread.
 *  Task requires and gets slot on bus system (1)
 *  process data and the bus (2)
 *  Leave the bus (3).
 */

void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
        unsigned int num_priority_send, unsigned int num_priority_receive)
{

    int i;
        for (i=0; i < (int)num_tasks_send;i++){
            thread_create ("task_Send_noPrio",NORMAL, *senderTask,NULL);
        }
        for (i=0; i < (int)num_task_receive;i++){
            thread_create ("task_Receive_noPrio",NORMAL, *receiverTask,NULL);
        }
        for (i=0; i < (int)num_priority_send;i++){
            thread_create ("task_Send_Prio",HIGH, *senderPriorityTask,NULL);
        }
        for (i=0; i < (int)num_priority_receive;i++){
            thread_create ("task_Receive_Prio",HIGH, *receiverPriorityTask,NULL);
        }

}

/* Normal task,  sending data to the accelerator */
void senderTask(void *aux UNUSED){
        task_t task = {SENDER, NORMAL};
        oneTask(task);
}

/* High priority task, sending data to the accelerator */
void senderPriorityTask(void *aux UNUSED){
        task_t task = {SENDER, HIGH};
        oneTask(task);
}

/* Normal task, reading data from the accelerator */
void receiverTask(void *aux UNUSED){
        task_t task = {RECEIVER, NORMAL};
        oneTask(task);
}

/* High priority task, reading data from the accelerator */
void receiverPriorityTask(void *aux UNUSED){
        task_t task = {RECEIVER, HIGH};
        oneTask(task);
}

/* abstract task execution*/
void oneTask(task_t task) {
  getSlot(task);
  transferData(task);
  leaveSlot(task);
}



/* task tries to get slot on the bus subsystem */
void getSlot(task_t task)
{
    lock_acquire(&thread_lock); // aquire lock or sleep until it can be aquired

    /* Wait:
        -if all slots occupied OR
        -if there are free slots <3 AND
        -- if task has normal priority AND highprioritylists are empty
        -- or wrong direction
            Release lock and wait until signaled
    */

    while(
         open_slots == 0 
         ||
         (open_slots <3 && ((task.priority == NORMAL && (!list_empty(&drive_condition[task.direction][HIGH].waiters)
         ||
         !list_empty(&drive_condition[1-task.direction][HIGH].waiters)))
         ||
         currentDirection != task.direction))) {

            cond_wait(&drive_condition[task.direction][task.priority], &thread_lock);
    }

    // get on the busTasks
    open_slots--;
    currentDirection = task.direction;
    lock_release(&thread_lock);
}

/* task processes data on the bus send/receive */
void transferData(task_t task)
{
    int timetowait = 15;//math.random(1,10);
        timer_sleep(timetowait);
}

/* task releases the slot */
void leaveSlot(task_t task)
{
        lock_acquire(&thread_lock);
        // exit bus
        /*
            if any prio tasks in current direction waiting
                signal one
            if prio task waiting to drive in other direction
                if broadcast bus is free
                    broadcast it

            And then same for normal priority
        */
        open_slots++;
        if(!list_empty(&(drive_condition[currentDirection][HIGH].waiters))) { 
                cond_signal(&drive_condition[currentDirection][HIGH], &thread_lock); 
        } else if(!list_empty(&drive_condition[1-currentDirection][HIGH].waiters)) { 

                        cond_broadcast(&drive_condition[1-currentDirection][HIGH], &thread_lock);
        } else if (!list_empty(&drive_condition[currentDirection][NORMAL].waiters)) {
                
                cond_signal(&drive_condition[currentDirection][NORMAL], &thread_lock); 
        
        } else if (!list_empty(&drive_condition[1-currentDirection][NORMAL].waiters)) {

                if (open_slots==BUS_CAPACITY) { 

                        cond_broadcast(&drive_condition[1-currentDirection][NORMAL], &thread_lock);
                }
        }

        lock_release(&thread_lock);
}
/**
 * G8RTOS_Scheduler.c
 * uP2 - Fall 2022
 */
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "BoardSupport/inc/RGBLedDriver.h"
#include "G8RTOS_Scheduler.h"
#include "G8RTOS_Structures.h"
#include "G8RTOS_CriticalSection.h"

/*
 * G8RTOS_Start exists in asm
 */
extern void G8RTOS_Start();

/* System Core Clock From system_msp432p401r.c */
extern uint32_t SystemCoreClock;

/*
 * Pointer to the currently running Thread Control Block
 */
//extern tcb_t * CurrentlyRunningThread;

/*********************************************** Dependencies and Externs *************************************************************/
// Pointer to the currently running Thread Control Block
tcb_t * CurrentlyRunningThread;
uint16_t currentMaxPriority;
threadId_t nextThreadID;

/*********************************************** Defines ******************************************************************************/

/* Status Register with the Thumb-bit Set */
#define THUMBBIT 0x01000000

/*********************************************** Defines ******************************************************************************/


/*********************************************** Data Structures Used *****************************************************************/

/* Thread Control Blocks
 *	- An array of thread control blocks to hold pertinent information for each thread
 */
static tcb_t threadControlBlocks[MAX_THREADS];

/* Thread Stacks
 *	- An array of arrays that will act as individual stacks for each thread
 */
static int32_t threadStacks[MAX_THREADS][STACKSIZE];

/* Periodic Event Threads
 * - An array of periodic events to hold pertinent information for each thread
 */
static ptcb_t Pthread[MAXPTHREADS];

/*********************************************** Data Structures Used *****************************************************************/


/*********************************************** Private Variables ********************************************************************/

/*
 * Current Number of Threads currently in the scheduler
 */
static uint32_t NumberOfThreads;

/*
 * Current Number of Periodic Threads currently in the scheduler
 */
static uint32_t NumberOfPthreads;

/*********************************************** Private Variables ********************************************************************/


/*********************************************** Private Functions ********************************************************************/

/*
 * Initializes the Systick and Systick Interrupt
 * The Systick interrupt will be responsible for starting a context switch between threads
 * Param "numCycles": Number of cycles for each systick interrupt
 */
static void InitSysTick(uint32_t numCycles)
{
    // your code
    SysTickPeriodSet(numCycles);
}

/*
 * Chooses the next thread to run.
 * Lab 2 Scheduling Algorithm:
 * 	- Simple Round Robin: Choose the next running thread by selecting the currently running thread's next pointer
 * 	- Check for sleeping and blocked threads
 */
void G8RTOS_Scheduler()
{
//    // your code
//    tcb_t* temp = CurrentlyRunningThread->nextTCB;
//    uint16_t PriorityBelowCurrent = LEAST_PRIORITY;
//    tcb_t* ThreadBelowCurrent = 0;
//    while(temp->isAlive == false || temp->priority > currentMaxPriority || temp->blocked != 0 || temp->asleep == true)
//    {
//        if(temp->priority > currentMaxPriority && temp->priority <= PriorityBelowCurrent)
//        {
//            PriorityBelowCurrent = temp->priority;
//            ThreadBelowCurrent = temp;
//        }
//
//        temp = temp->nextTCB;
//    }
//
//    CurrentlyRunningThread = temp;
//    currentMaxPriority = CurrentlyRunningThread->priority;

    // your code
    tcb_t* temp = CurrentlyRunningThread->nextTCB;
    uint16_t PriorityBelowCurrent = LEAST_PRIORITY;

    tcb_t* ThreadAboveCurrent = 0;
    tcb_t* ThreadBelowCurrent = 0;

    do
    {
        if(temp->blocked == 0 && temp->asleep == false)//if valid thread
        {
            if(temp->priority <= currentMaxPriority)//if thread is higher or equal priority than actual current
            {
                currentMaxPriority = temp->priority;
                ThreadAboveCurrent = temp;
            }
            else
            {
                if(temp->priority <= PriorityBelowCurrent)//if thread is higher or equal priority than next alternative
                {
                    PriorityBelowCurrent = temp->priority;
                    ThreadBelowCurrent = temp;
                }
            }
        }

        temp = temp->nextTCB;
    }while(temp != CurrentlyRunningThread);//while(temp != CurrentlyRunningThread->nextTCB);

    if(ThreadAboveCurrent != 0)
    {
        CurrentlyRunningThread = ThreadAboveCurrent;
    }
    else if(!(CurrentlyRunningThread->blocked == 0 && CurrentlyRunningThread->asleep == false))
    {
        CurrentlyRunningThread = ThreadBelowCurrent;
    }

    currentMaxPriority = CurrentlyRunningThread->priority;
}


/*
 * SysTick Handler
 * The Systick Handler now will increment the system time,
 * set the PendSV flag to start the scheduler,
 * and be responsible for handling sleeping and periodic threads
 */
void SysTick_Handler()
{
    // your code
    SystemTime++;

    ptcb_t *PeriodicTemp = &Pthread[0];

    for(uint32_t i = 0; i < NumberOfPthreads; i++)
    {
        if(PeriodicTemp->executeTime == SystemTime)
        {
            runningPeriodic = true;
            CurrentlyRunningPThread = Pthread + i;

            PeriodicTemp->executeTime = PeriodicTemp->period + SystemTime;
            PeriodicTemp->handler();

            runningPeriodic = false;
            CurrentlyRunningPThread = 0;
        }

        PeriodicTemp = PeriodicTemp->nextPTCB;
    }

    tcb_t* temp = CurrentlyRunningThread;
    do
    {
        if(temp->asleep == true && temp->sleepCount <= SystemTime)
        {
            temp->asleep = false;
        }
        temp = temp->nextTCB;
    }while(temp != CurrentlyRunningThread);

    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
}

/*********************************************** Private Functions ********************************************************************/


/*********************************************** Public Variables *********************************************************************/

/* Holds the current time for the whole System */
uint32_t SystemTime;

/*********************************************** Public Variables *********************************************************************/


/*********************************************** Public Functions *********************************************************************/

/*
 * Sets variables to an initial state (system time and number of threads)
 * Enables board for highest speed clock and disables watchdog
 */
void G8RTOS_Init()
{
    // your code
    // Initialize system time to zero
    SystemTime = 0;
    // Set the number of threads to zero
    NumberOfThreads = 0;
    NumberOfPthreads = 0;
    currentMaxPriority = LEAST_PRIORITY;

    //relocate interrupt vectors to SRAM
    uint32_t newVTORTable = VTORTableAddr;
    uint32_t * newTable = (uint32_t*)newVTORTable;
    uint32_t * oldTable = (uint32_t *)0;
    for (int i = 0; i < 155; i++)
    {
    newTable[i] = oldTable[i];
    }
    HWREG(NVIC_VTABLE) = newVTORTable;

    for(int i = 0; i < MAX_THREADS; ++i)//here to guarantee isAlive is false initialized for all threads
    {
        threadControlBlocks[i].isAlive = false;
    }
}

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes the Systick
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
int G8RTOS_Launch()
{
    // your code
    InitSysTick(SysCtlClockGet() / 1000); // 1 ms tick

    // add your code
    CurrentlyRunningThread = &threadControlBlocks[0];
    tcb_t * temp = &threadControlBlocks[0];

    while((temp = temp->nextTCB) != &threadControlBlocks[0])
    {

        if(temp->priority < CurrentlyRunningThread->priority)//if higher priority
        {
            CurrentlyRunningThread = temp;
        }
    }

    currentMaxPriority = CurrentlyRunningThread->priority;

    //HWREG(NVIC_SYS_PRI3) |= (uint32_t)((0b11100000111)<<21);
    IntPrioritySet(FAULT_PENDSV, 0xE0);
    IntPrioritySet(FAULT_SYSTICK, 0xE0);
    SysTickIntEnable();
    G8RTOS_Start(); // call the assembly function
    return 0;
}


/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are stil available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread to hold a "fake context"
 * 	- Sets stack tcb stack pointer to top of thread stack
 * 	- Sets up the next and previous tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads
 */
sched_ErrCode_t G8RTOS_AddThread(void (*threadToAdd)(void), uint8_t priority, char *name)
{
    IBit_State = StartCriticalSection();
    if(priority > LEAST_PRIORITY)
    {
        EndCriticalSection(IBit_State);
        return HWI_PRIORITY_INVALID;
    }

    if (NumberOfThreads > MAX_THREADS)
    {
        EndCriticalSection(IBit_State);
        return THREAD_LIMIT_REACHED;
    }
    else
    {
        uint32_t nextAvailable = 0;
        uint32_t lastInRobin = 0;

        if (NumberOfThreads == 0)
        {
            // There is only one thread (in the linked list), so both the next and previous threads are itself
            threadControlBlocks[nextAvailable].nextTCB = &threadControlBlocks[nextAvailable];
            threadControlBlocks[nextAvailable].previousTCB = &threadControlBlocks[nextAvailable];
        }
        else
        {
            for(int i = 0; i < MAX_THREADS; ++i)
            {
                if(threadControlBlocks[i].isAlive == false)
                {
                    nextAvailable = i;
                }
                else
                {
                    lastInRobin = i;
                }

            }
            /*
            Append the new thread to the end of the linked list
            * 1. Number of threads will refer to the newest thread to be added since the current index would be NumberOfThreads-1
            * 2. Set the next thread for the new thread to be the first in the list, so that round-robin will be maintained
            * 3. Set the current thread's nextTCB to be the new thread
            * 4. Set the first thread's previous thread to be the new thread, so that it goes in the right spot in the list
            * 5. Point the previousTCB of the new thread to the current thread so that it moves in the correct order
            */
            tcb_t* firstInRobin = threadControlBlocks[lastInRobin].nextTCB;

            threadControlBlocks[lastInRobin].nextTCB = &threadControlBlocks[nextAvailable];
            threadControlBlocks[nextAvailable].previousTCB = &threadControlBlocks[lastInRobin];
            firstInRobin->previousTCB = &threadControlBlocks[nextAvailable];
            threadControlBlocks[nextAvailable].nextTCB = firstInRobin;

        }

        threadControlBlocks[nextAvailable].blocked = 0;
        threadControlBlocks[nextAvailable].asleep = false;
        threadControlBlocks[nextAvailable].sleepCount = 0;
        threadControlBlocks[nextAvailable].priority = priority;
        threadControlBlocks[nextAvailable].isAlive = true;

        for(int i = 0; i < MAX_NAME_LENGTH-1; ++i)
        {
            threadControlBlocks[nextAvailable].Threadname[i] = name[i];
        }

        threadControlBlocks[nextAvailable].Threadname[MAX_NAME_LENGTH-1] = 0;
        threadControlBlocks[nextAvailable].ThreadID = nextThreadID++;


        // Set up the stack pointer
        threadControlBlocks[nextAvailable].stackPointer = &threadStacks[nextAvailable][STACKSIZE - 16];
        threadStacks[nextAvailable][STACKSIZE - 1] = THUMBBIT;
        threadStacks[nextAvailable][STACKSIZE - 2] = (uint32_t)threadToAdd;

        // Increment number of threads present in the scheduler
        NumberOfThreads++;
    }

    EndCriticalSection(IBit_State);
    return NO_ERROR;
}


/*
 * Adds periodic threads to G8RTOS Scheduler
 * Function will initialize a periodic event struct to represent event.
 * The struct will be added to a linked list of periodic events
 * Param Pthread To Add: void-void function for P thread handler
 * Param period: period of P thread to add
 * Returns: Error code for adding threads
 */
int G8RTOS_AddPeriodicEvent(void (*PthreadToAdd)(void), uint32_t period, uint32_t execution)
{
    // your code
    if (NumberOfPthreads > MAXPTHREADS)
    {
        return 0;
    }
    else
    {
        if (NumberOfPthreads == 0)
        {
            // There is only one thread (in the linked list), so both the next and previous threads are itself
            Pthread[NumberOfPthreads].nextPTCB = &Pthread[NumberOfPthreads];
            Pthread[NumberOfPthreads].previousPTCB = &Pthread[NumberOfPthreads];
        }
        else
        {
            /*
            Append the new thread to the end of the linked list
            * 1. Number of threads will refer to the newest thread to be added since the current index would be NumberOfThreads-1
            * 2. Set the next thread for the new thread to be the first in the list, so that round-robin will be maintained
            * 3. Set the current thread's nextTCB to be the new thread
            * 4. Set the first thread's previous thread to be the new thread, so that it goes in the right spot in the list
            * 5. Point the previousTCB of the new thread to the current thread so that it moves in the correct order
            */
            Pthread[NumberOfPthreads-1].nextPTCB = &Pthread[NumberOfPthreads];
            Pthread[NumberOfPthreads].previousPTCB = &Pthread[NumberOfPthreads-1];
            Pthread[0].previousPTCB = &Pthread[NumberOfPthreads];
            Pthread[NumberOfPthreads].nextPTCB = &Pthread[0];
        }

        Pthread[NumberOfPthreads].handler = PthreadToAdd;
        Pthread[NumberOfPthreads].period = period;
        Pthread[NumberOfPthreads].executeTime = execution;


        // Increment number of threads present in the scheduler
        NumberOfPthreads++;
    }
    return 1;
}

sched_ErrCode_t G8RTOS_AddAPeriodicEvent(void (*AthreadToAdd)(void), uint8_t priority, int32_t IRQn)
{
    // your code
    if(IRQn <= 0 || IRQn >= 155)
    {
        return IRQn_INVALID;
    }

    if(priority > 6)
    {
        return HWI_PRIORITY_INVALID;
    }

    void (** VTORTable)(void) = (void (**)(void)) VTORTableAddr;
    VTORTable[IRQn] = AthreadToAdd;

    IntPrioritySet(IRQn, priority<<5);
    IntEnable(IRQn);

    return NO_ERROR;
}


/*
 * Puts the current thread into a sleep state.
 *  param durationMS: Duration of sleep time in ms
 */
void sleep(uint32_t durationMS)
{
    CurrentlyRunningThread->sleepCount = durationMS + SystemTime;   //Sets sleep count
    CurrentlyRunningThread->asleep = 1;                             //Puts the thread to sleep
    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;                  //Start context switch
}

threadId_t G8RTOS_GetThreadId()
{
    return CurrentlyRunningThread->ThreadID;        //Returns the thread ID
}

sched_ErrCode_t G8RTOS_KillThread(threadId_t threadID)
{
    IBit_State = StartCriticalSection();
    // your code
    if(NumberOfThreads == 0)
    {
        EndCriticalSection(IBit_State);
        return NO_THREADS_SCHEDULED;
    }

    if(NumberOfThreads == 1)
    {
        EndCriticalSection(IBit_State);
        return CANNOT_KILL_LAST_THREAD;
    }

    tcb_t* temp = CurrentlyRunningThread;

    do
    {
        if(temp->ThreadID == threadID)
        {
            temp->isAlive = false;
            --NumberOfThreads;

            temp->nextTCB->previousTCB = temp->previousTCB;
            temp->previousTCB->nextTCB = temp->nextTCB;

            if(temp != CurrentlyRunningThread)
            {
                EndCriticalSection(IBit_State);
                return NO_ERROR;
            }
            else
            {
                EndCriticalSection(IBit_State);
                HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
            }
        }

        temp = temp->nextTCB;
    }while(temp != CurrentlyRunningThread);

    EndCriticalSection(IBit_State);
    return THREAD_DOES_NOT_EXIST;
}

//Thread kills itself
sched_ErrCode_t G8RTOS_KillSelf()
{
    // your code
    IBit_State = StartCriticalSection();
    // your code
    if(NumberOfThreads == 0)
    {
        EndCriticalSection(IBit_State);
        return NO_THREADS_SCHEDULED;
    }

    if(NumberOfThreads == 1)
    {
        EndCriticalSection(IBit_State);
        return CANNOT_KILL_LAST_THREAD;
    }

    CurrentlyRunningThread->isAlive = false;
    --NumberOfThreads;

    CurrentlyRunningThread->nextTCB->previousTCB = CurrentlyRunningThread->previousTCB;
    CurrentlyRunningThread->previousTCB->nextTCB = CurrentlyRunningThread->nextTCB;

    EndCriticalSection(IBit_State);
    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;

    //here so compiler doesn't complain, should never reach
    return NO_ERROR;
}

uint32_t GetNumberOfThreads(void)
{
    return NumberOfThreads;         //Returns the number of threads
}

void G8RTOS_KillAllThreads()
{
    // your code
}

uint32_t getClock()
{
    return SystemTime;
}






/*********************************************** Public Functions *********************************************************************/

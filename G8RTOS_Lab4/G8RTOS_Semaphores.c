/**
 * G8RTOS_Semaphores.c
 * uP2 - Fall 2022
 */

/*********************************************** Dependencies and Externs *************************************************************/

#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "G8RTOS_Semaphores.h"
#include "G8RTOS_CriticalSection.h"
#include "G8RTOS_Scheduler.h"

bool runningPeriodic = false;
ptcb_t* CurrentlyRunningPThread = 0;
bool periodicFault = false;


/*********************************************** Dependencies and Externs *************************************************************/


/*********************************************** Public Functions *********************************************************************/

/*
 * Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_InitSemaphore(semaphore_t *s, int32_t value)
{
    // add your code
    IBit_State = StartCriticalSection();
    *s = value;
    EndCriticalSection(IBit_State);
}

/*
 * No longer waits for semaphore
 *  - Decrements semaphore
 *  - Blocks thread if sempahore is unavailable
 * Param "s": Pointer to semaphore to wait on
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_WaitSemaphore(semaphore_t *s)
{
    // add your code
    IBit_State = StartCriticalSection();
    (*s)--;

    if(*s < 0)
    {
        if(runningPeriodic == false)
        {
            CurrentlyRunningThread->blocked = s;
            EndCriticalSection(IBit_State);
            HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
        }
        else
        {
            periodicFault = true;
            EndCriticalSection(IBit_State);
        }
    }
    else
    {
        EndCriticalSection(IBit_State);
    }
}

/*
 * Signals the completion of the usage of a semaphore
 *  - Increments the semaphore value by 1
 *  - Unblocks any threads waiting on that semaphore
 * Param "s": Pointer to semaphore to be signaled
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_SignalSemaphore(semaphore_t *s)
{
    // add your code
    IBit_State = StartCriticalSection();
    (*s)++;

    if(*s <= 0)
    {
        tcb_t* startThread = CurrentlyRunningThread;
        tcb_t* temp = CurrentlyRunningThread->nextTCB;

        while(temp != startThread)
        {
            if(temp->blocked == s)
            {
                temp->blocked = 0;
                break;
            }

            temp = temp->nextTCB;
        }

        EndCriticalSection(IBit_State);

    }
    else
    {
        EndCriticalSection(IBit_State);
    }
}

/*********************************************** Public Functions *********************************************************************/



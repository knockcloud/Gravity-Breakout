/**
 * G8RTOS IPC - Inter-Process Communication
 * @author:
 * uP2 - Fall 2022
 */
#include <stdint.h>
#include "G8RTOS_IPC.h"
#include "G8RTOS_Semaphores.h"

#define FIFOSIZE 16
#define MAX_NUMBER_OF_FIFOS 4

/*
 * FIFO struct will hold
 *  - buffer
 *  - head
 *  - tail
 *  - lost data
 *  - current size
 *  - mutex
 */

/* Create FIFO struct here */

typedef struct FIFO_t {
    int32_t buffer[16];
    int32_t *head;
    int32_t *tail;
    uint32_t lostData;
    semaphore_t currentSize;
    semaphore_t mutex;
    semaphore_t producerLock;
} FIFO_t;


/* Array of FIFOS */
static FIFO_t FIFOs[4];


/*
 * Initializes FIFO Struct
 */
int G8RTOS_InitFIFO(uint32_t FIFOIndex)
{
    // your code
    if(FIFOIndex > MAX_NUMBER_OF_FIFOS)
    {
        return 1;
    }

    FIFOs[FIFOIndex].head = FIFOs[FIFOIndex].buffer;
    FIFOs[FIFOIndex].tail = FIFOs[FIFOIndex].buffer;
    FIFOs[FIFOIndex].lostData = 0;
    G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].currentSize, 0);
    G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].mutex, 1);
    G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].producerLock, 1);
    return 0;
}

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param: "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
int32_t readFIFO(uint32_t FIFOChoice)
{
    // your code
    G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].mutex);
    G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].currentSize);
    int32_t result = *(FIFOs[FIFOChoice].head);

    if(FIFOs[FIFOChoice].head == (FIFOs[FIFOChoice].buffer + FIFOSIZE - 1))
    {
        FIFOs[FIFOChoice].head = FIFOs[FIFOChoice].buffer;
    }
    else
    {
        ++FIFOs[FIFOChoice].head;
    }
    G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].mutex);

    return result;
}

/*
 * Writes to FIFO
 *  Writes data to Tail of the buffer if the buffer is not full
 *  Increments tail (wraps if necessary)
 *  Param "FIFOChoice": chooses which buffer we want to read from
 *        "Data': Data being put into FIFO
 *  Returns: error code for full buffer if unable to write
 */
int writeFIFO(uint32_t FIFOChoice, uint32_t Data)
{
    // your code

    G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].producerLock);
    int returnVal = 0;

    //mid-interrupt of readFifo can make currentSize==FIFOSIZE-1 actually mean its full and tail is at head; or it could actually be correct if no weird stuff happened
    if(FIFOs[FIFOChoice].currentSize >= FIFOSIZE-1)
    {
        //wait for current reader to finish, if any, and prevent any from starting
        G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].mutex);

        //now we have sole access to FIFO and no race conditions are possible woth currentSize, its static
        if(FIFOs[FIFOChoice].currentSize == FIFOSIZE)
        {
            ++FIFOs[FIFOChoice].lostData;
            returnVal = 1;

            //move head to next block
            if(FIFOs[FIFOChoice].head == (FIFOs[FIFOChoice].buffer + FIFOSIZE - 1))
            {
                FIFOs[FIFOChoice].head = FIFOs[FIFOChoice].buffer;
            }
            else
            {
                ++FIFOs[FIFOChoice].head;
            }

        }
        G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].mutex);

        //G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].producerLock);

    }

    *FIFOs[FIFOChoice].tail = Data;

    if(FIFOs[FIFOChoice].tail == (FIFOs[FIFOChoice].buffer + FIFOSIZE - 1))
    {
        FIFOs[FIFOChoice].tail = FIFOs[FIFOChoice].buffer;
    }
    else
    {
        ++FIFOs[FIFOChoice].tail;
    }

    if(FIFOs[FIFOChoice].currentSize < FIFOSIZE)
    {
        G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].currentSize);
    }
    G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].producerLock);

    return returnVal;
}


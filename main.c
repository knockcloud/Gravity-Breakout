/**
 * main.c
 * @author:
 * uP2 - Fall 2022
 */


// your includes and defines

#include "BoardSupport/inc/ILI9341_Lib.h"
#include <G8RTOS_Lab4/G8RTOS_Scheduler.h>
#include <G8RTOS_Lab4/G8RTOS_Semaphores.h>
#include <G8RTOS_Lab4/G8RTOS_Structures.h>
#include <G8RTOS_Lab4/G8RTOS_IPC.h>
#include "driverlib/watchdog.h"
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "BoardSupport/inc/BoardInitialization.h"
#include <stdbool.h>
#include "BoardSupport/inc/RGBLedDriver.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

#include "threads.h"
#include "Colors.h"

volatile uint16_t temp1 = 0;
void main(void)
{
    // your code
    G8RTOS_Init();
    bool isBoardSetup = InitializeBoard();

    LCD_Init(1);

    G8RTOS_AddThread(emptyThread, LEAST_PRIORITY, "emptyThread");
//    G8RTOS_AddThread(readAccelThread, 1, "readAccel");
    G8RTOS_AddThread(gameThread, 0, "gameThread");
    G8RTOS_Launch();
}

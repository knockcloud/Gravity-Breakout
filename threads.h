/**
 * thread.h
 * @author:
 * uP2 - Fall 2022
 */

#ifndef THREADS_H_
#define THREADS_H_

#include "G8RTOS_Semaphores.h"
#include "G8RTOS_Structures.h"
#include "BoardSupport/inc/ILI9341_Lib.h"


//semaphores
extern semaphore_t sensorsMutex;
extern semaphore_t lcdMutex;
extern semaphore_t ballsMutex;



//threads
/*******************************************************************************
 * Function Name  : emptyThread
 * Description    : Idle thread
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void emptyThread(void);

void readAccelThread(void);

/*******************************************************************************
 * Function Name  : gameThread
 * Description    : Background thread to handle updating game variables and game state
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void gameThread(void);

/*******************************************************************************
 * Function Name  : ballThread
 * Description    : Background thread to handle updating ball variables and ball state.
 *                  Also checks for collisions
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void ballThread(void);

/*******************************************************************************
 * Function Name  : renderThead
 * Description    : Periodic thread to handle drawing to LCD screen.
 *                  Often uses dirty rectangle method for efficiency.
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void renderThread(void);

/*******************************************************************************
 * Function Name  : velocityThread
 * Description    : Periodic thread to update the paddle's velocity.
 *                  Does so by polling IMU or Joystick, depending on gravity mode
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void velocityThread(void);


#endif /* THREADS_H_ */

/**
 * thread.c
 * @author:
 * uP2 - Fall 2022
 */

// your includes and defines
#include <stdbool.h>
#include "BoardSupport/inc/ILI9341_Lib.h"
#include <G8RTOS_Lab4/G8RTOS_Scheduler.h>
#include <G8RTOS_Lab4/G8RTOS_Semaphores.h>
#include "BoardSupport/inc/bmi160_support.h"
#include <G8RTOS_Lab4/G8RTOS_IPC.h>
#include "threads.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "BoardSupport/inc/Joystick.h"
#include <stdio.h>

#include "Game.h"
#include <stdlib.h>


semaphore_t gameStatusMutex;
semaphore_t ballMutex;
#define COORD_FIFO 0

volatile bool wasTouched = false;

Ball ball;
Paddle paddle;
Brick allBricks[8][BRICKS_PER_ROW];

uint16_t gameScore = 0;
BrickType highestBrickCollided = iterBeforeBrick;
int16_t lives = 3;
uint8_t level = 1;
bool updateGameStatus = false;
bool lifeLost = false, gameOver = false;
EventStatus collidedWithTop = NEVER, speedUp = NEVER, gravityMode = NEVER;
uint8_t vel_min=1, vel_max=2;
uint16_t gravityScore=0, gravityDuration=0;

/*******************************************************************************
 * Function Name  : gameThread
 * Description    : Background thread to handle updating game variables and game state
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void gameThread(void)
{
    G8RTOS_AddAPeriodicEvent(tapISRThread, 0, INT_GPIOB);//set up touch handler

    /******INIT SEQUENCE******/
    //semaphores
    G8RTOS_InitSemaphore(&ballMutex, 1);
    G8RTOS_InitSemaphore(&gameStatusMutex, 1);

    LCD_Clear(BACKGROUND_COLOR);//background
    LCD_DrawRectangle(0, 0, SIDE_WALL_WIDTH, SIDE_WALL_HEIGHT, WALL_COLOR);//left wall
    LCD_DrawRectangle(SIDE_WALL_WIDTH, 0, TOP_WALL_WIDTH, TOP_WALL_HEIGHT, WALL_COLOR);//top wall
    LCD_DrawRectangle(RIGHT_EDGE, 0, SIDE_WALL_WIDTH, SIDE_WALL_HEIGHT, WALL_COLOR);//right wall

    //game status
    LCD_Text(0, 0, "Score: ", TEXT_COLOR);
    drawHeart(FIRST_HEART_X,0);
    drawHeart(SECOND_HEART_X,0);
    drawHeart(THIRD_HEART_X,0);

    gameScore = 0;
    gravityScore = getRand(5, 50);
    gravityDuration = getRand(5,50);

    //bricks
    drawLevel();

    //paddle
    paddle.x = paddle.x_old = 112, paddle.y = paddle.y_old = PADDLE_Y
            , paddle.width = PADDLE_WIDTH, paddle.height = PADDLE_HEIGHT
            , paddle.updatedOld = false;
    LCD_DrawRectangle(paddle.x, paddle.y, paddle.width, paddle.height, PADDLE_COLOR);

    //ball
    G8RTOS_AddThread(ballThread, 0, "ballThread");
    G8RTOS_AddPeriodicEvent(renderThread, 10, getClock()+20);
    G8RTOS_AddPeriodicEvent(velocityThread, 10, getClock()+21);
    sleep(100);

    Ball myBall;

    while(true)
    {
        //obtain local copy of global ball variable
        G8RTOS_WaitSemaphore(&ballMutex);
        myBall = ball;
        G8RTOS_SignalSemaphore(&ballMutex);

        G8RTOS_WaitSemaphore(&gameStatusMutex);

        //check if game is over
        if(lives == 0)
        {
            G8RTOS_SignalSemaphore(&gameStatusMutex);
            gameOver = true;
            reset(&myBall, "GAME OVER !");
        }
        else
        {
            G8RTOS_SignalSemaphore(&gameStatusMutex);
        }

        //check if we should increase speed of ball
        if(speedUp == NEVER && gameScore >= 20)
        {
            vel_min += 1, vel_max += 1;
            myBall.x_vel = (myBall.x_vel > 0) ? getRand(vel_min, vel_max) : -getRand(vel_min, vel_max);
            myBall.y_vel = (myBall.y_vel > 0) ? getRand(vel_min, vel_max) : -getRand(vel_min, vel_max);
            speedUp = ONCE;
        }
        else if(speedUp == ONCE && gameScore >= 125)
        {
            vel_min += 1, vel_max += 1;
            myBall.x_vel = (myBall.x_vel > 0) ? getRand(vel_min, vel_max) : -getRand(vel_min, vel_max);
            myBall.y_vel = (myBall.y_vel > 0) ? getRand(vel_min, vel_max) : -getRand(vel_min, vel_max);
            speedUp = MULTIPLE;
        }
        else if(speedUp == MULTIPLE && gameScore >= 250)
        {
            vel_min += 1, vel_max += 1;
            myBall.x_vel = (myBall.x_vel > 0) ? getRand(vel_min, vel_max) : -getRand(vel_min, vel_max);
            myBall.y_vel = (myBall.y_vel > 0) ? getRand(vel_min, vel_max) : -getRand(vel_min, vel_max);
            speedUp = FINAL;
        }

        //recount game score and check if they've cleared the level
        bool levelCleared = true;
        for(BrickType i = yellowBricks1; i != iterEndBrick; i++)
        {
            for(int j = 0; j < BRICKS_PER_ROW; j++)
            {
                G8RTOS_WaitSemaphore(&gameStatusMutex);
                if(!allBricks[i][j].isAlive && !allBricks[i][j].isAccounted)
                {
                    gameScore += ((i>>1) << 1) + 1;
                    allBricks[i][j].isAccounted = true;
                    updateGameStatus = true;
                }
                else if(allBricks[i][j].isAlive)
                {
                    levelCleared = false;
                }
                G8RTOS_SignalSemaphore(&gameStatusMutex);
            }
        }

        //handle whether to turn gravity mode on or off
        if(gameScore >= gravityScore && gameScore < gravityScore+gravityDuration)
        {
            if(gravityMode == NEVER)
            {
                gravityMode = ONCE;
            }
            else
            {
                gravityMode = MULTIPLE;
            }
        }
        else if(gameScore >= gravityScore+gravityDuration && gravityMode!=NEVER)
        {
            gravityMode = FINAL;
        }

        //display level cleared message and wait for player to rester
        if(levelCleared)
        {
            gameOver = true;
            reset(&myBall, "CONGRATULATIONS");
        }

        //update global ball variable with local
        G8RTOS_WaitSemaphore(&ballMutex);
        ball = myBall;
        G8RTOS_SignalSemaphore(&ballMutex);

        if(lifeLost)
        {
            sleep(1000);
            lifeLost = false;
        }
        else
        {
            sleep(100);
        }
    }
}

/*******************************************************************************
 * Function Name  : ballThread
 * Description    : Background thread to handle updating ball variables and ball state.
 *                  Also checks for collisions
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void ballThread(void)
{
    ball.x = ball.x_old = (rand() % 20) + 110;
    ball.y = ball.y_old = (rand() % 20) + YELLOW_LEVEL_1 + 1;
    ball.x_vel = getRand(vel_min, vel_max);
    ball.y_vel = -getRand(vel_min, vel_max);
    ball.updatedOld = false;
    int16_t x_old, y_old;
    Ball myBall;
    bool x_flipped = false, y_flipped = false, brick_collision = false;

    while(true)
    {
        if(lifeLost || gameOver)
        {
            continue;
        }

        brick_collision = false;

        //obtain local copy of global ball variable
        G8RTOS_WaitSemaphore(&ballMutex);
        myBall = ball;
        G8RTOS_SignalSemaphore(&ballMutex);

        if(!myBall.updatedOld)
        {
            x_old = myBall.x;
            y_old = myBall.y;
        }
        int16_t deltaX = myBall.x_vel;
        int16_t deltaY = -myBall.y_vel;

        //move ball
        myBall.x += deltaX;
        myBall.y += deltaY;

        //edge cases and bound checks; corrects x and y coordinates if need be
        if(myBall.x < 0)
        {
            myBall.x = 0;
        }
        else if(myBall.x + BALL_WIDTH > X_MAX)
        {
            myBall.x = X_MAX - BALL_WIDTH;
        }

        if(myBall.x <= LEFT_EDGE || myBall.x+BALL_WIDTH >= RIGHT_EDGE)
        {
            brick_collision = true;
            if(!x_flipped)
            {
                x_flipped = true;
                myBall.x_vel = -myBall.x_vel;
            }
        }

        //checks for ball collision with bricks
        for(BrickType i = yellowBricks1; i != iterEndBrick && !brick_collision; i++)
        {
            if( (myBall.y < allBricks[i][0].y+BRICK_HEIGHT) && (myBall.y+BALL_HEIGHT > allBricks[i][0].y) )
            {
                for(int j = 0; j < BRICKS_PER_ROW; j++)
                {
                    if(!y_flipped && allBricks[i][j].isAlive && (myBall.x < allBricks[i][j].x+BRICK_WIDTH) && (myBall.x+BALL_WIDTH > allBricks[i][j].x))
                    {
                        brick_collision = true;
                        y_flipped = true;
                        myBall.y_vel = -myBall.y_vel;

                        G8RTOS_WaitSemaphore(&gameStatusMutex);
                        highestBrickCollided = (i>highestBrickCollided) ? i : highestBrickCollided;
                        allBricks[i][j].isAlive = false;
                        G8RTOS_SignalSemaphore(&gameStatusMutex);
                        break;
                    }
                }
            }
        }

        myBall.x_old = x_old;
        myBall.y_old = y_old;
        myBall.updatedOld = true;

        //check for ball collision with either bottom edge (loses life) or top edge
        if(!y_flipped && (myBall.y <= TOP_EDGE || myBall.y > Y_MAX))
        {
            brick_collision = true;
            y_flipped = true;
            myBall.y_vel = -myBall.y_vel;

            if(myBall.y > Y_MAX && !lifeLost)
            {
                G8RTOS_WaitSemaphore(&gameStatusMutex);
                lives = ((lives-1) < 0) ? 0 : --lives;
                lifeLost = true;
                updateGameStatus = true;
                G8RTOS_SignalSemaphore(&gameStatusMutex);

                myBall.x = (rand() % 20) + 110;
                myBall.y = (rand() % 20) + YELLOW_LEVEL_1 + 1;
                myBall.y_vel = (myBall.y_vel > 0) ? -myBall.y_vel : myBall.y_vel;
            }

            if(myBall.y <= TOP_EDGE && collidedWithTop == NEVER)
            {
                paddle.width >>= 1;
                collidedWithTop = ONCE;
            }
        }

        //checks for ball collison with paddle
        if(!y_flipped && !x_flipped && !lifeLost && isColliding(myBall.x, myBall.y, BALL_WIDTH, BALL_HEIGHT, paddle.x, paddle.y, paddle.width, paddle.height))
        {
            brick_collision = true;
            y_flipped = true;
            myBall.y_vel = (myBall.y_vel < 0) ? -myBall.y_vel : myBall.y_vel;
            myBall.x_vel = (myBall.x+(BALL_WIDTH>>1) < paddle.x + (paddle.width>>1))
                    ? -getRand(vel_min, vel_max) : getRand(vel_min, vel_max);
        }


        if(!brick_collision && x_flipped)
        {
            x_flipped = false;
        }
        if(!brick_collision && y_flipped)
        {
           y_flipped = false;
        }

        //update global ball variable with local
        G8RTOS_WaitSemaphore(&ballMutex);
        ball = myBall;
        G8RTOS_SignalSemaphore(&ballMutex);

        sleep(10);
    }

}

/*******************************************************************************
 * Function Name  : velocityThread
 * Description    : Periodic thread to update the paddle's velocity.
 *                  Does so by polling IMU or Joystick, depending on gravity mode
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void velocityThread(void)
{
    if(gameOver)
    {
        return;
    }

    int32_t coord[2];
    int16_t x_vel = 0, accelY;

    if(!paddle.updatedOld)
    {
        paddle.x_old = paddle.x;
        paddle.y_old = paddle.y;
        paddle.updatedOld = true;
    }

    //get x_vel value either from IMU or joystick
    if(gravityMode != NEVER && gravityMode!=FINAL)
    {
        while(bmi160_read_accel_y(&accelY));
        x_vel = -accelY / (2*384);
    }
    else
    {
        GetJoystickCoordinates((uint32_t*)coord);
        x_vel = (coord[1] - 2048) / 384;
    }
    paddle.x_vel = x_vel;
    paddle.x += paddle.x_vel;

    //edge cases and boudn checks for paddle
    if(paddle.x <= LEFT_EDGE)
    {
        paddle.x = LEFT_EDGE+1;
    }
    else if(paddle.x + paddle.width >= RIGHT_EDGE)
    {
        paddle.x = RIGHT_EDGE - paddle.width - 1;
    }

}

/*******************************************************************************
 * Function Name  : renderThead
 * Description    : Periodic thread to handle drawing to LCD screen.
 *                  Often uses dirty rectangle method for efficiency.
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void renderThread(void)
{
    Ball myBall = ball;

    if(gameOver)
    {
        return;
    }

    //updates game score and hearts if need be
    if(updateGameStatus)
    {
        LCD_DrawRectangle(40,0,30,16,WALL_COLOR);
        char temp[256] = {0};
        snprintf(temp, 256, "%3u", gameScore);
        LCD_Text(40, 0, (uint8_t*)temp, TEXT_COLOR);

        switch(lives)
        {
            case 0:
                LCD_DrawRectangle(THIRD_HEART_X, 0, 20, 20, WALL_COLOR);
                break;
            case 1:
                LCD_DrawRectangle(SECOND_HEART_X, 0, 20, 20, WALL_COLOR);
                break;
            case 2:
                LCD_DrawRectangle(FIRST_HEART_X, 0, 20, 20, WALL_COLOR);
                break;
        }
        updateGameStatus = false;
    }

    //removes old paddle when shrinking paddle
    if(collidedWithTop==ONCE)
    {
        LCD_DrawRectangle(paddle.x, paddle.y, PADDLE_WIDTH, PADDLE_HEIGHT, BACKGROUND_COLOR);
        collidedWithTop = MULTIPLE;
    }

    //display message alerting player gravity mode has been enabled
    //or clean it up if disable
    if(gravityMode == ONCE)
    {
        LCD_Text(GRAVITY_TEXT_X, 0, "GRAVITY ON", TEXT_COLOR);
    }
    else if(gravityMode == FINAL)
    {
        LCD_DrawRectangle(GRAVITY_TEXT_X,0,85,16,WALL_COLOR);
        gravityMode = NEVER;
    }

    //removes bricks that have been collided with
    for(BrickType i = yellowBricks1; i != iterEndBrick; i++)
     {
         for(int j = 0; j < BRICKS_PER_ROW; j++)
         {
             if(!allBricks[i][j].isAlive && !allBricks[i][j].isCleared)
             {
                 LCD_DrawRectangle(allBricks[i][j].x, allBricks[i][j].y, BRICK_WIDTH, BRICK_HEIGHT, BACKGROUND_COLOR);
                 allBricks[i][j].isCleared = true;
             }
         }
     }

    //draws ball
     LCD_DrawRectangle(myBall.x_old, myBall.y_old, BALL_WIDTH, BALL_HEIGHT, BACKGROUND_COLOR);

     if(!lifeLost)
     {
         LCD_DrawRectangle(myBall.x, myBall.y, BALL_WIDTH, BALL_HEIGHT, BALL_COLOR);
     }

     //cleans up left, right, and top wall if ball left a mark
     if(myBall.x_old <= LEFT_EDGE)
     {
         LCD_DrawRectangle(myBall.x_old, myBall.y_old, LEFT_EDGE-myBall.x_old+1, BALL_HEIGHT, WALL_COLOR);
     }
     else if(myBall.x_old+BALL_WIDTH >= RIGHT_EDGE)
     {
         LCD_DrawRectangle(RIGHT_EDGE, myBall.y_old, myBall.x_old+BALL_WIDTH-RIGHT_EDGE+1, BALL_HEIGHT, WALL_COLOR);
     }

     if(isColliding(myBall.x_old, myBall.y_old, BALL_WIDTH, BALL_HEIGHT, SIDE_WALL_WIDTH, 0, TOP_WALL_WIDTH, TOP_WALL_HEIGHT))
     {
         LCD_DrawRectangle(myBall.x_old, myBall.y_old, BALL_WIDTH, TOP_EDGE-myBall.y_old+1, WALL_COLOR);
     }

     //draws paddle
     if(paddle.x != paddle.x_old || paddle.y != paddle.y_old)
     {
         LCD_DrawRectangle(paddle.x_old, paddle.y_old, paddle.width, PADDLE_HEIGHT, BACKGROUND_COLOR);
         LCD_DrawRectangle(paddle.x, paddle.y, paddle.width, PADDLE_HEIGHT, PADDLE_COLOR);
     }
     else if(isColliding(myBall.x_old, myBall.y_old, BALL_WIDTH, BALL_HEIGHT, paddle.x, paddle.y, paddle.width, paddle.height))
     {
         LCD_DrawRectangle(paddle.x, paddle.y, paddle.width, PADDLE_HEIGHT, PADDLE_COLOR);
     }

     myBall.updatedOld = false;
     ball = myBall;

     paddle.updatedOld = false;
}

/*******************************************************************************
 * Function Name  : tapISRThread
 * Description    : Aperiodic thread to detect taps on LCD touchscreen
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void tapISRThread(void)
{
    GPIOIntDisable(GPIO_PORTB_BASE,GPIO_INT_PIN_4);
    if(gameOver)
    {
        wasTouched = true;
    }
}

/*******************************************************************************
 * Function Name  : reset
 * Description    : Resets variables to initial game state so that threads perceive it as such
 * Input          : myBall_ptr - Pointer to ball object
 *                  message - String to display when game is paused or over
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void reset(Ball* myBall_ptr, uint8_t* message)
{
  //  Ball myBall = *myBall_ptr;
    LCD_DrawRectangle(myBall_ptr->x_old, myBall_ptr->y_old, BALL_WIDTH, BALL_HEIGHT, BACKGROUND_COLOR);
    LCD_DrawRectangle(paddle.x_old, paddle.y_old, paddle.width, PADDLE_HEIGHT, BACKGROUND_COLOR);

    paddle.x = paddle.x_old = 112, paddle.y = paddle.y_old = PADDLE_Y
                , paddle.width = PADDLE_WIDTH, paddle.height = PADDLE_HEIGHT
                , paddle.updatedOld = false;
    LCD_DrawRectangle(paddle.x, paddle.y, paddle.width, PADDLE_HEIGHT, PADDLE_COLOR);

    LCD_Text(80, YELLOW_Y_1 + 20, message, RED);
    if(wasTouched)
    {
        wasTouched = false;


        //reset all game variable to their initial values
        drawLevel();
        drawHeart(FIRST_HEART_X,0);
        drawHeart(SECOND_HEART_X,0);
        drawHeart(THIRD_HEART_X,0);
        LCD_DrawRectangle(GRAVITY_TEXT_X,0,85,16,WALL_COLOR);
        gameScore = 0;
        lives = 3;
        level = 1;
        updateGameStatus = false;
        highestBrickCollided = iterBeforeBrick;

        LCD_DrawRectangle(80, YELLOW_Y_1 + 20, 150, 16, BACKGROUND_COLOR);
        lifeLost = false, gameOver = false, wasTouched = false;
        collidedWithTop = NEVER, speedUp = NEVER;
        vel_min=1, vel_max=2;
        myBall_ptr->x_vel = getRand(vel_min, vel_max);
        myBall_ptr->y_vel = -getRand(vel_min, vel_max);

        gravityMode = NEVER;
        gravityScore = getRand(5, 50);
        gravityDuration = getRand(5,50);
    }

    GPIOIntClear(GPIO_PORTB_BASE, GPIO_INT_PIN_4);
    GPIOIntEnable(GPIO_PORTB_BASE,GPIO_INT_PIN_4);
}

/*******************************************************************************
 * Function Name  : constructBricks
 * Description    : Constructs bricks; initialize variables and draw on LCD
 * Input          : bricks - Array of bricks to be constructed
 *                  y - y-coordinate of bricks
 *                  color - color of bricks
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void constructBricks(Brick* bricks, uint16_t y, BrickColor color)
{
    for(int i = 0; i < BRICKS_PER_ROW; ++i)
    {
        bricks[i].x = i*(BRICK_WIDTH+BRICK_SPACE) + SIDE_WALL_WIDTH;
        bricks[i].y = y;

        bricks[i].color = color;
        bricks[i].isAlive = true;
        bricks[i].isCleared = bricks[i].isAccounted = false;

        LCD_DrawRectangle(bricks[i].x, bricks[i].y, BRICK_WIDTH, BRICK_HEIGHT, bricks[i].color);
    }
}

/*******************************************************************************
 * Function Name  : isColliding
 * Description    : Inline function to check if two rectangles are colliding
 * Input          : x1,y1,w1,h1 - x-corodinate, y-coordinate of upper left corner and width, height of one of the rectangles
 *                  x2,y2,w2,h2 - x-corodinate, y-coordinate of upper left corner and width, height of one of the rectangles
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline bool isColliding(uint16_t x1, uint16_t y1, uint16_t w1, uint16_t h1, uint16_t x2, uint16_t y2, uint16_t w2, uint16_t h2)
{
    return x1 < x2+w2 && x1+w1 > x2 && y1 < y2+h2 && y1+h1 > y2;
}

/*******************************************************************************
 * Function Name  : drawLevel
 * Description    : Helper function to draw and reset all bricks
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void drawLevel(void)
{
    constructBricks(allBricks[yellowBricks1], YELLOW_Y_1, BRICK_YELLOW);
    constructBricks(allBricks[yellowBricks2], YELLOW_Y_2, BRICK_YELLOW);

    constructBricks(allBricks[greenBricks1], GREEN_Y_1, BRICK_GREEN);
    constructBricks(allBricks[greenBricks2], GREEN_Y_2, BRICK_GREEN);

    constructBricks(allBricks[orangeBricks1], ORANGE_Y_1, BRICK_ORANGE);
    constructBricks(allBricks[orangeBricks2], ORANGE_Y_2, BRICK_ORANGE);

    constructBricks(allBricks[redBricks1], RED_Y_1, BRICK_RED);
    constructBricks(allBricks[redBricks2], RED_Y_2, BRICK_RED);
}

/*******************************************************************************
 * Function Name  : drawHeart
 * Description    : Helper function to draw heart
 * Input          : x - x-corodinate of upper left corner
 *                  y - y-corodinate of upper left corner
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void drawHeart(uint16_t x, uint16_t y)
{
    for(int i = 0; i < 20; i++)
    {
        for(int j = 0; j < 20; j++)
        {
            LCD_SetPoint(x+j, y+i, heart[i*20+j]);
        }
    }
}

/*******************************************************************************
 * Function Name  : getRand
 * Description    : Inline function that randomly select a integer between provided min and max. inclusive of both
 * Input          : min - Minimum bound
 *                  max - Maximum bound
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline uint16_t getRand(uint16_t min, uint16_t max)
{
    return (rand() % (max-min + 1)) + min;
}

/*******************************************************************************
 * Function Name  : emptyThread
 * Description    : Idle thread
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void emptyThread(void)
{
    while(true);
}

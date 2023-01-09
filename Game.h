#ifndef GAME_H
#define GAME_H

#include "Colors.h"

#define BRICKS_PER_ROW      14
#define BRICK_WIDTH         14
#define BRICK_SPACE         2
#define BRICK_SPACE_COL     3
#define BRICK_HEIGHT        8

#define PADDLE_WIDTH        28
#define PADDLE_HEIGHT       10
#define PADDLE_LEVEL        24
#define PADDLE_Y            (Y_MAX - PADDLE_LEVEL)

#define BALL_WIDTH          4
#define BALL_HEIGHT         4

#define SIDE_WALL_WIDTH     9
#define SIDE_WALL_HEIGHT    Y_SIZE

#define TOP_WALL_WIDTH      222
#define TOP_WALL_HEIGHT     20

#define LEFT_EDGE           (SIDE_WALL_WIDTH-1)
#define RIGHT_EDGE          (SIDE_WALL_WIDTH+TOP_WALL_WIDTH)
#define TOP_EDGE            (TOP_WALL_HEIGHT-1)

#define GAME_WIDTH          (X_SIZE - SIDE_WALL_WIDTH*2)


#define YELLOW_LEVEL_1      186
#define YELLOW_LEVEL_2      (YELLOW_LEVEL_1+BRICK_HEIGHT+BRICK_SPACE_COL)
#define GREEN_LEVEL_1       (YELLOW_LEVEL_2+BRICK_HEIGHT+BRICK_SPACE_COL)
#define GREEN_LEVEL_2       (GREEN_LEVEL_1+BRICK_HEIGHT+BRICK_SPACE_COL)
#define ORANGE_LEVEL_1      (GREEN_LEVEL_2+BRICK_HEIGHT+BRICK_SPACE_COL)
#define ORANGE_LEVEL_2      (ORANGE_LEVEL_1+BRICK_HEIGHT+BRICK_SPACE_COL)
#define RED_LEVEL_1         (ORANGE_LEVEL_2+BRICK_HEIGHT+BRICK_SPACE_COL)
#define RED_LEVEL_2         (RED_LEVEL_1+BRICK_HEIGHT+BRICK_SPACE_COL)

#define YELLOW_Y_1          (Y_MAX - YELLOW_LEVEL_1)
#define YELLOW_Y_2          (Y_MAX - YELLOW_LEVEL_2)
#define GREEN_Y_1           (Y_MAX - GREEN_LEVEL_1)
#define GREEN_Y_2           (Y_MAX - GREEN_LEVEL_2)
#define ORANGE_Y_1          (Y_MAX - ORANGE_LEVEL_1)
#define ORANGE_Y_2          (Y_MAX - ORANGE_LEVEL_2)
#define RED_Y_1             (Y_MAX - RED_LEVEL_1)
#define RED_Y_2             (Y_MAX - RED_LEVEL_2)

#define FIRST_HEART_X       160
#define SECOND_HEART_X      FIRST_HEART_X + 25
#define THIRD_HEART_X       SECOND_HEART_X + 25

#define GRAVITY_TEXT_X      75

//objects
typedef struct Brick
{
    int16_t x, y;
    BrickColor color;
    bool isAlive, isCleared, isAccounted;
} Brick;

typedef struct Paddle
{
    int16_t x, y, x_old, y_old;
    int16_t x_vel;
    uint8_t width, height;
    bool updatedOld;
} Paddle;

typedef struct Ball
{
    int16_t x, y, x_old, y_old;
    int16_t x_vel, y_vel;
    bool updatedOld;
} Ball;

typedef enum BrickType
{
    iterBeforeBrick=-1, yellowBricks1, yellowBricks2,
    greenBricks1, greenBricks2,
    orangeBricks1, orangeBricks2,
    redBricks1, redBricks2, iterEndBrick
} BrickType;

typedef enum EventStatus
{
    NEVER, ONCE, MULTIPLE, FINAL
} EventStatus;

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
void constructBricks(Brick* bricks, uint16_t y, BrickColor color);

/*******************************************************************************
 * Function Name  : isColliding
 * Description    : Inline function to check if two rectangles are colliding
 * Input          : x1,y1,w1,h1 - x-corodinate, y-coordinate of upper left corner and width, height of one of the rectangles
 *                  x2,y2,w2,h2 - x-corodinate, y-coordinate of upper left corner and width, height of one of the rectangles
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline bool isColliding(uint16_t x1, uint16_t y1, uint16_t w1, uint16_t h1, uint16_t x2, uint16_t y2, uint16_t w2, uint16_t h2);

/*******************************************************************************
 * Function Name  : drawLevel
 * Description    : Helper function to draw and reset all bricks
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void drawLevel(void);

/*******************************************************************************
 * Function Name  : drawHeart
 * Description    : Helper function to draw heart
 * Input          : x - x-corodinate of upper left corner
 *                  y - y-corodinate of upper left corner
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void drawHeart(uint16_t x, uint16_t y);
void tapISRThread(void);

/*******************************************************************************
 * Function Name  : reset
 * Description    : Resets variables to initial game state so that threads perceive it as such
 * Input          : myBall_ptr - Pointer to ball object
 *                  message - String to display when game is paused or over
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void reset(Ball* myBall_ptr, uint8_t* message);

/*******************************************************************************
 * Function Name  : getRand
 * Description    : Inline function that randomly select a integer between provided min and max. inclusive of both
 * Input          : min - Minimum bound
 *                  max - Maximum bound
 * Output         : None
 * Return         : random sample
 * Attention      : None
 *******************************************************************************/
inline uint16_t getRand(uint16_t min, uint16_t max);

//heart sprite
uint16_t heart[400] = {
                       WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR,
                       WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, TEXT_COLOR, TEXT_COLOR, TEXT_COLOR, TEXT_COLOR, TEXT_COLOR, TEXT_COLOR, TEXT_COLOR, TEXT_COLOR, TEXT_COLOR, TEXT_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR,
                       WALL_COLOR, WALL_COLOR, WALL_COLOR, TEXT_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, TEXT_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR,
                       WALL_COLOR, WALL_COLOR, TEXT_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, TEXT_COLOR, WALL_COLOR, WALL_COLOR,
                       WALL_COLOR, TEXT_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, TEXT_COLOR, WALL_COLOR,
                       WALL_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, WALL_COLOR,
                       WALL_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, WALL_COLOR,
                       WALL_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, WALL_COLOR,
                       WALL_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, WALL_COLOR,
                       WALL_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, WALL_COLOR,
                       WALL_COLOR, TEXT_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, TEXT_COLOR, WALL_COLOR,
                       WALL_COLOR, WALL_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, WALL_COLOR, WALL_COLOR,
                       WALL_COLOR, WALL_COLOR, TEXT_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, TEXT_COLOR, WALL_COLOR, WALL_COLOR,
                       WALL_COLOR, WALL_COLOR, WALL_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR,
                       WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, TEXT_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, TEXT_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR,
                       WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR,
                       WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, TEXT_COLOR, TEXT_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, HEART_COLOR, TEXT_COLOR, TEXT_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR,
                       WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, TEXT_COLOR, TEXT_COLOR, TEXT_COLOR, TEXT_COLOR, TEXT_COLOR, TEXT_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR,
                       WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, TEXT_COLOR, TEXT_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR,
                       WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR, WALL_COLOR
                       };




#endif //GAME_H

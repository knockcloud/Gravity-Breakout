#ifndef COLORS_H
#define COLORS_H

#include "BoardSupport/inc/ILI9341_Lib.h"

#define BACKGROUND_COLOR    BLACK
#define WALL_COLOR          WHITE
#define PADDLE_COLOR        BLUE
#define BALL_COLOR          WHITE
#define TEXT_COLOR          BLACK
#define HEART_COLOR         RED

typedef enum BrickColor
{
    BRICK_YELLOW = YELLOW,
    BRICK_GREEN = GREEN,
    BRICK_ORANGE = ORANGE,
    BRICK_RED = RED
} BrickColor;

#endif //COLORS_H

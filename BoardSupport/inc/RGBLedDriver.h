#ifndef RGBLEDDRIVER_H_
#define RGBLEDDRIVER_H_

#include <stdint.h>

uint32_t REGISTER_LEDS;
uint32_t LEDShiftTemp;

typedef enum device{
    LED_BLUE = 0,
    LED_GREEN = 1,
    LED_RED = 2
} uint_desig;

//Send PWM data to LEDs
void LP3943_PWMColorSet(uint32_t unit, uint32_t PWM_DATA, uint32_t LED_DATA);

//Turn LEDs on or off
void LP3943_LedModeSet(uint32_t unit, uint16_t LED_DATA);

//Initializes the LEDs
void InitializeRGBLEDs();

// Turns LEDs off
void TurnOffLEDs(uint_desig color);

#endif /* RGBLEDDRIVER_H_ */

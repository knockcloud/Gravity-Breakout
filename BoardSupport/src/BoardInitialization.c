/*
 * BoardInitialization.c
 */

#include <stdbool.h>
#include "BoardSupport/inc/BoardInitialization.h"
#include "BoardSupport/inc/I2CDriver.h"
#include "BoardSupport/inc/RGBLedDriver.h"
#include "BoardSupport/inc/opt3001.h"
#include "BoardSupport/inc/bmi160_support.h"
#include "BoardSupport/inc/tmp007.h"
#include "BoardSupport/inc/bme280_support.h"
#include "BoardSupport/inc/Joystick.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

static void InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}

bool InitializeBoard(void)
{
    // WatchDog timer is off by default (will generate errors in debug window unless clock gating is enabled to it)

    // Set clock speed higher using the PLL (50 MHz)
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    // Initialize I2C for the LEDs and the Sensor BooserPack
    InitializeLEDI2C(I2C0_BASE);
    InitializeSensorI2C();

    // Initialize and Test TMP007 IR Temperature Sensor
    sensorTmp007Enable(true);
    bool isTempSensorFunctional = sensorTmp007Test();

    // Check if temperature sensor is functional
    //if (!isTempSensorFunctional)
    //    return false;

    // Initialize BMI160 Accelerometer and Gyroscope
    bmi160_initialize_sensor();

    // Initialize BME280 Humidity Sensor
    bme280_initialize_sensor();

    // Initialize Joystick
    Joystick_Init_Without_Interrupt();
    //Joystick_Push_Button_Init_With_Interrupt();

    // Initialize Console UART from TIVA SDK
    InitConsole();

    // Initialize RGB LEDs
    //InitializeRGBLEDs();

    // Initialize and Test OPT3001 Digital Ambient Light Sensor (ALS)
    sensorOpt3001Enable(true);
    bool isLightSensorFunctional = sensorOpt3001Test();

    // Check if ALS is functional
    if (!isLightSensorFunctional)
        return false;

    return true;
}



/**
 * @file    sensors.h
 * @brief   sensors input library for the game NotBopIt
 * @author  Daniel Retta, Stephanie Scott, Danyang Hu
 * @date    January 23rd, 2025
 * */

 #ifndef sensors_H
 #define sensors_H

 #define LONG_PRESS  1000    // milliseconds constituting a captouch long press

 #define X_ACC_BIAS 4.75
 #define X_ACC_SCALE 1.0047
 #define Y_ACC_BIAS -47.5
 #define Y_ACC_SCALE 0.994
 #define Z_ACC_BIAS -8.75
 #define Z_ACC_SCALE 1.0014

 #define FLEX_PIN  ADC_0
 #define PIEZO_PIN ADC_1
 #define PING_PIN  GPIO_PIN_0
 #define TOUCH_PIN GPIO_PIN_5
 #define IR_PIN    GPIO_PIN_12

#include <Board.h>
#include "QEI.h"
#include "BNO055.h"
#include "ADC.h"
#include "PING.h"

typedef enum {    
      none        // 0
    , captouch    // 1
    , infrared    // 2
    , flex        // 3
    , ultrasonic  // 4
    , rotary      // 5
    , piezo       // 6
    , IMU         // 7

    } sensor_t;   // used to store selected trial sensor value

uint32_t timeResponse; // used to evaluate sensor response time in MICROseconds
    
/**
* @function    SENSORS_Init()
* @brief       initialize all dependent sensor libraries
*/
void SENSORS_Init();

// user navigation //

/**
* @function    encoderChangeCW()
* @brief       if encoder moved clockwise read high once
*/
int encoderChangeCW();

/**
* @function    encoderChangeCCW()
* @brief       if encoder moved counter clockwise read high once
*/
int encoderChangeCCW();

/**
* @function    int captouchPressed()
* @brief       if capacitive touch sensor is touched read high once
*/
int captouchPressed();

/**
* @function    captouchReleased()
* @brief       if capacitive touch sensor is released read high once
*/
int captouchReleased();

/**
* @function    captouchHeld()
* @brief       if capacitive touch sensor has been held for this long read high once
*/
int captouchHeld(int duration);

/**
* @function    activatedSensor()
* @brief       return value of activated sensor, zero if none
*/
int sensorActivated();



// user response interpretation //

// determines which sensor is currently face up based on accelerometer reading

/**
* @function    int sensorFaceUp()
* @brief       return value of face up sensor, zero if none
*/
int sensorFaceUp();

// each of these are flags that go high for a single cycle if user has interacted with the sensor correctly

/**
* @function    int flexActivated()
* @brief       if the flex resistor has been flexed correctly read high once
*/
int flexActivated();

/**
* @function    int piezoActivated()
* @brief       if the flex piezo has been shaken correctly read high once
*/
int piezoActivated();

/**
* @function    int rotaryActivated()
* @brief       if the encoder has been rotated correctly read high once
*/
int rotaryActivated();

/**
* @function    int ultrasonicActivated()
* @brief       if the ultrasonic sensor has been triggered correctly read high once
*/
int ultrasonicActivated();

/**
* @function    int captouchActivated()
* @brief       if the capacitic touch sensor has been triggered correctly read high once
*/
int captouchActivated();

/**
* @function    int infraredActivated()
* @brief       if the infrared sensor has been triggered correctly read high once
*/
int infraredActivated();

/**
* @function    int infraredActivated()
* @brief       if the IMU has been triggered correctly read high once
*/
int IMUActivated();

/**
* @function    void GPIO_Init()
* @brief       Configure GPIO pins : PC4 PC5 PC12
*/
void GPIO_Init();

 #endif
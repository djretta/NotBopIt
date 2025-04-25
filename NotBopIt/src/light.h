/**
 * @file    light.h
 * @brief   rgb light output library for the game NotBopIt
 * @author  Daniel Retta, Stephanie Scott, Danyang Hu
 * @date    January 23rd, 2025
 * */

 #ifndef light_H
 #define light_H

 const int sensorColor [8][3];

 /**
 * @function    LIGHT_Init()
 * @brief       initializes the RGB LED PWMs
 */
 void LIGHT_Init();

 /**
 * @function    updateColor()
 * @brief       pushes color and light changes to the RGB LED PWMs
 */
void updateRGBLED();

 /**
 * @function    brightness(int percent)
 * @brief       sets the RGB PWM duty cycle ranging from 0 to 100%
 */
void brightness(int percent);

 /**
 * @function    void color(int R, int G, int B)
 * @brief       outputs a color to the RGB LED given values R, G and B
 */
void color(int R, int G, int B);

 /**
 * @function    void color(int R, int G, int B)
 * @brief       outputs a color to the RGB LED given colorwheel degrees
 *              yellow is zero, red is 60, blue is 180
 */
void colorInDegrees(int degrees);

 /**
 * @function    lightOff()
 * @brief       turns the RGB brightness to zero
 */
 void lightOff();

 /**
 * @function    colorWheel(int degreeInitial, int period)
 * @brief       outputs and increments the RGB color wheel
 *              resets cycle when fed new arguments
 */
void colorWheel(int direction, int degreeInitial, int degreeFinal, int period);

 /**
 * @function    void continuousColorWheel(int direction, int period) {
 * @brief       outputs and increments the RGB color wheel continuously
 *              resets cycle when fed new arguments, direction, final value is opposite in height of initial value
 */
void continuousColorWheel(int period);

 /**
 * @function    brightnessFade(int startingBrightness, int endingBrightness, int period, int transition)
 * @brief       fades brightness from an initial brightness to a ending brightness over a period of time
 *              nonblocking incremental implementation resets to new cycle when new parameters are applied
 */
void brightnessFade(int startingBrightness, int endingBrightness, int period, int transition);

 #endif
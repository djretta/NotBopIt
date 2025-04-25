#include <light.h>
#include <timers.h>
#include <pwm.h>
#include <stdlib.h>

// additional function insights are provided in the header of this file, but can be access by simply hovering over the function name in VSCode

int bright = 100;           // these global variable values are applied to the RGB LED PWM using the function updateColor
int r = 0, g = 0, b = 0;    // which is called at the end of soundAndLight() in NotBopIt.c
    
// this array defines the color of the LED RGB to indicate a particular sensor to interact with
const int sensorColor [8][3] = {
    {0, 0, 0},          // 0:none:          none
    {100, 0, 0},        // 1:captouch:      Red
    {0, 100, 0},        // 2:infrared:      Green
    {0, 0, 100},        // 3:flex:          Blue
    {100, 100, 0},      // 4:ultrasonic:    Yellow
    {0, 100, 100},      // 5:rotary:        Cyan
    {100, 0, 100},      // 6:piezo:         Magenta
    {100, 100, 100}     // 7:IMU            White
};

void LIGHT_Init() { PWM_Init(); }

void updateRGBLED() {

    r = r * bright / 100; g = g * bright / 100; b = b * bright / 100;                         // apply brightness setting

    unsigned int R_duty = 100 - r, G_duty = 100 - g, B_duty = 100 - b;      // apply color setting (the PWM values are inverted)

    PWM_SetDutyCycle(PWM_1, R_duty);
    PWM_SetDutyCycle(PWM_2, G_duty);
    PWM_SetDutyCycle(PWM_3, B_duty);
    
}

void color(int R, int G, int B) { r = R, g = G, b = B; }               // update global variables r,g,b for brightness operations   

void colorInDegrees(int degrees) {

    // Note: a duty cycle of 0 results in full brightness; a 100% duty cycle turns LED off
    // printf("In function colorInDegrees, degrees = %d\n", degrees);

    unsigned int R_duty = 0, G_duty = 0, B_duty = 0;

    float theta = 0;         // for mapping the degrees of each zone to the range -60 to +60

    if(abs(degrees) <= 60)          // if in the Green - Red zone
    {
        theta = degrees;
        G_duty = 50 + 50 * (theta/60.0);
        R_duty = 50 - 50 * (theta/60.0);
        B_duty = 100;
    }
    else if(abs(degrees) >= 300)    // if in the other Green - Red zone
    {
        if(degrees < 0) theta = degrees + 360;
        else theta = degrees - 360;
        G_duty = 50 + 50 * (theta/60.0);
        R_duty = 50 - 50 * (theta/60.0);
        B_duty = 100;
    }
    else if(degrees >= 60 && degrees <=180) // if in the positive Red - Blue zone
    {
        theta = degrees - 120;
        R_duty = 50 + 50 * (theta/60.0);
        B_duty = 50 - 50 * (theta/60.0);
        G_duty = 100;
    }
    else if(degrees >= -300 && degrees <= -180) // if in the negative Red - Blue zone
    {
        theta = degrees + 240;
        R_duty = 50 + 50 * (theta/60.0);
        B_duty = 50 - 50 * (theta/60.0);
        G_duty = 100;
    }
    else if(degrees > 180 && degrees <= 300) // if in the positive Blue - Green zone
    {
        theta = degrees - 240;
        B_duty = 50 + 50 * (theta/60.0);
        G_duty = 50 - 50 * (theta/60.0);
        R_duty = 100;
    }
    else if(degrees >= -180 && degrees <= -60) // if in the negative Blue - Green zone
    {
        theta = degrees + 120;
        B_duty = 50 + 50 * (theta/60.0);
        G_duty = 50 - 50 * (theta/60.0);
        R_duty = 100;
    }

    color(100 - R_duty, 100 - G_duty, 100 - B_duty);    // this function was initially written to be applied directly to the PWM
                                                        // instead these values are passed to color to work on global variables in order to apply brightness settings

}

void brightness(int percent) { bright = percent; }

void lightOff() { bright = 0; }

void brightnessFade(int startingBrightness, int endingBrightness, int period, int transition) {

    static int 
    lastStartingBrightness = 0, // using used variables to maintain intrastate orientation
    lastEndingBrightness = 0,
    lastPeriod = 0,
    lastTransition = 0,         // using transition count to maintain long term interstate orientation
    timeStart = 0,              // store initial time at first entry
    timeElapsed = 0;            // holds current elapsed time since initial entry

    static double change = 0;   // holds change per millisecond to apply

    timeElapsed = TIMERS_GetMilliSeconds() - timeStart;

    if (lastStartingBrightness == startingBrightness && lastEndingBrightness == endingBrightness &&  lastPeriod == period && lastTransition == transition) {
        if (timeElapsed>period) { 
            brightness(endingBrightness);
            return;    // this task has completed its lifecycle
        } else {
            brightness(startingBrightness + timeElapsed * change);
        }
    } else {                    // treat as new instance
        timeStart = TIMERS_GetMilliSeconds();
        lastStartingBrightness = startingBrightness;
        lastEndingBrightness = endingBrightness;
        lastPeriod = period;
        lastTransition = transition;
        change = (endingBrightness - startingBrightness) / period;
    }
    
}

void colorWheel(int direction, int degreeInitial, int degreeFinal, int period) {
    static int lastDirection = 0, lastInitial = 0, lastFinal = 0, lastPeriod = 0;
    static int timeStart = 0, timeElapsed = 0;
    static double change = 0;           // change in degrees per millisecond
    
    timeElapsed = TIMERS_GetMilliSeconds() - timeStart;
    
    if (lastDirection == direction && lastInitial == degreeInitial && 
        lastFinal == degreeFinal && lastPeriod == period) {
        
        if (timeElapsed > period) { 
            colorInDegrees(degreeFinal); // complete the transition
            return;
        } else {
            int currentDegrees = degreeInitial + (int)(timeElapsed * change);
            colorInDegrees(currentDegrees);
        }
    } else {                            // treat as a new instance
        timeStart = TIMERS_GetMilliSeconds();
        lastDirection = direction;
        lastInitial = degreeInitial;
        lastFinal = degreeFinal;
        lastPeriod = period;
        
        change = (double)(degreeFinal - degreeInitial) / period;
    }
}

void continuousColorWheel(int period) {
    static int timeStart = 0, timeElapsed = 0;
    
    const int FULL_CIRCLE = 360;
    
    timeElapsed = TIMERS_GetMilliSeconds() - timeStart;
    
    // calculate current position in the color wheel
    int currentDegree = (timeElapsed % period) * FULL_CIRCLE / period;
    
    // set the color based on the current position in the wheel
    colorInDegrees(currentDegree);
}
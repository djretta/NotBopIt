#include <stdlib.h>
#include <stdio.h>
#include <Board.h>          // lib: provides core functionality for STM32 HAL framework
#include <timers.h>         // lib: provides access to time information
#include <light.h>          // lib: provides RGB control functions
#include <sound.h>          // lib: provides sound control functions
#include <sensors.h>        // lib: provides sensor interpretation functions

#define TRIALS 6                // trials per level
#define LEVELS 6                // levels per game

void    state();                        // run state machine
void    soundAndLight();                // control light and sound output
void    transitionTo (int state);       // transition between states
int     selectSensor();                 // select sensor from random number generator
int     checkLevelChange(int level);    // update level changed flag
void    printStatus();                  // debugging print key global variables

char  	strOut[96];             // debugging string to print to OLED and/or serial, not used for game

int     timeEntry       = 0;    // holds time reading taken upon state change
int     timeInState     = 0;    // holds time since entering state
int     timeSpan[9]     = {0};  // for time regulated states, use timeSpan[STATUS]
                                // timespan values are set in state machine

int     transition      = 0;    // count of state transitions; only resets at power off

int     trial           = 0;    // current trial (there are multiple trials per level); range: [0:TRIALS]
int     level           = 0;    // current level (there are multiple levels per game); range: [0:LEVELS]

int     levelChanged    = 0;    // flag: high if level changed since last cycle

int     activeSensor    = 0;    // sensor whose input was successfully logged

enum    { 
          initialization        // 0: welcome, simply cycles RGB awaiting user input, plays a little lively tune
        , selection             // 1: level selection, optional, controlled by encoder
        , introduction          // 2: intermediate starting stage, requires continuous start button holding to start
        , abortion              // 3: enters when start button was let go too early
        , indication            // 4: indicates to player which activity they are supposed to perform
        , response              // 5: awaits player input, time limited
        , lose                  // 6: player didn't perform the activity
        , levelup               // 7: player completed the level
        , win                   // 8: player completed all of the levels

    }   status = 0;             // state machine current state

sensor_t sensor = none;         // initialize sensor variable


    //      General Notes

    //      - it is recommended that this code be viewed on a 27" screen and above (like the one it was written on)

    //      - all time is in milliseconds

    //      - function inputs are minimalist and global variable oriented

    //      - low level functions are intended to make use of static internal memory
    //          to determine what they should be doing at any point in time, resetting when arguments change

    //      - sound() & light functions direct output subfunctions based on status, passing parameters: time, level, transition, etc



int main(void) {

	BOARD_Init();           // initialize HAL framework, serial clocks & pins, LEDs & Big Blue Button

    SENSORS_Init();         // initialize sensor components

    LIGHT_Init();           // initialize PWMs for the RGB LEDs

    SOUND_Init();           // initialize PWMs for the speaker

    srand(timeEntry);                       // seed the random number generator

    sensor = none;                          // initialize sensor variable

    timeEntry = TIMERS_GetMilliSeconds();   // initial state entry time

        HAL_Delay(3000);                                // diagnostic: give serial client moment to open
        printf("\n\nNotBopIt initialized.\n\n");        // diagnostic: initialization announcement

    transitionTo(initialization);

	while (TRUE) {

        timeInState = TIMERS_GetMilliSeconds() - timeEntry;         // update timeInState regularly

        levelChanged = checkLevelChange(level);                     // flag: check if level was changed since last cycle

        state();                                                    // work on state machine

		soundAndLight();                                            // work on sounds and lights

        levelChanged = FALSE;                                       // reset flag at end of cycle

        updateRGBLED();

	}

}

void printStatus() {

    printf("\nprintStatus:: \ttimeEntry: %d status: %d level: %d trial: %d sensor: %d\n\n", timeEntry, status, level, trial, sensor);

}

void transitionTo ( int newState) {             // controls transition between states

    transition++;                               // keep track of total number of state transitions

        //printf("transitionTo:: \tTime in last state: \t%d\n", timeInState);         // diagnostic: state change data printed to serial
        printf("transitionTo:: \tState transition #: \t%d\n", transition);          // diagnostic: state change data printed to serial
        printf("transitionTo:: \tfrom: \t%d to: %d \n" , status, newState);         // diagnostic: state change data printed to serial
        printf("transitionTo:: \tActivated sensor: \t%d\n", activeSensor);          // diagnostic: state change data printed to serial
        // printf("transitionTo:: \tResponse time: \t%d\n [us]", timeResponse);          // diagnostic: state change data printed to serial
        // printStatus();
        
    activeSensor = 0;

    status    = newState;                       // update status with provided state

    timeEntry = TIMERS_GetMilliSeconds();       // update new entry time

    // tasks to be completed upon entering a state only once
    if (status == indication) {
        sensor = selectSensor();
    }

}

// timeElapsed() returns TRUE if the time elapsed is longer than the time submitted
int timeElapsed ( int presentTime) { return ( timeInState >= presentTime ) ; }

// selectSensor() acquires a random value with a range from 1:7
int selectSensor() {

    int selected = (rand() % 7) + 1;                      // outputs 1 to 7

        printf("Sensor selected: %d.\n", selected);       // diagnostic sensor selection data printed to serial

    return selected;

}

// state() directs state machine changes and regulates timing and key game variables
// variables set within a state cannot be changed by other parts of the program and will immediately revert
void state() {

    if          (status == initialization)  {

        timeSpan[initialization] = 1500;    // three half second tones

            // printf("\nQEI_GetPosition:%d", QEI_GetPosition());  // diagnostic: raw encoder
        
        if      ( encoderChangeCW() ) { transitionTo(selection);    }
        else if ( captouchPressed() ) { transitionTo(introduction); }

    } else if   (status == selection)       {

        timeSpan[selection] = 333;      // single short tone upon level change

        // alter level per turn of encoder
        if      ( encoderChangeCW()  && level <= LEVELS ) { level++; }
        else if ( encoderChangeCCW() && level != 0)       { level--; }
 
        if      ( captouchPressed() ) { transitionTo(introduction); }

    } else if   (status == introduction)    {

        timeSpan[introduction] = 1500;      // three half seconds

        if      ( captouchHeld(LONG_PRESS * 3) ) { transitionTo(indication); }
        else if ( captouchReleased()           ) { transitionTo(abortion  );  }

    } else if   (status == abortion)        {

        timeSpan[abortion] = 3000;          // three one second actions

        if      ( timeElapsed(timeSpan[abortion]) )   { transitionTo(initialization);                       trial = 0; level = 0; }

    } else if   (status == indication)      {

        timeSpan[indication] = 1000;        // the player is offered one second of tone and RGB to indicate interactive sensor

        if      ( timeElapsed(timeSpan[indication]) )   { transitionTo(response); }

    } else if   (status == response)        {

        activeSensor = sensorActivated(sensor);

        if      ( timeElapsed( ((LEVELS - level + 1) * 1000) / 2) )           { transitionTo(lose); }
        else if ( activeSensor ) { // THIS SHOULD BE AN ELSE-IF FOR TIME CONSTRAINT 
            if ( activeSensor == sensor ) {                                                                 trial++;
                if ( trial <= TRIALS )        { transitionTo(indication);                                               }
                if ( trial >  TRIALS )        { transitionTo(levelup);                                      trial = 0; level++; printf("\n\n=== LEVEL UP ===\n\n");}
            } else                            { transitionTo(lose); }
        }

    } else if   (status == lose)            {

        timeSpan[lose] = 9000;

        if      ( timeElapsed(timeSpan[lose]) )   { transitionTo(initialization);                           trial = 0; level = 0; }
        
    } else if   (status == levelup)         {

        timeSpan[levelup] = level*1000;

        if      ( timeElapsed(timeSpan[levelup]) && level<=LEVELS ) { transitionTo(indication); }
        else if ( timeElapsed(timeSpan[levelup]) && level>LEVELS) { transitionTo(win); }

    } else if   (status == win)             {

        timeSpan[win] = LEVELS*1000;

        if      ( timeElapsed(timeSpan[win])) { transitionTo(initialization);                               trial = 0; level = 0; }

    }

}

// soundAndLight() directs light and sound output based on status, time and perhaps sensor input
// - below contents should call upon functions from sound.c/.h and light.c/.h
// - timing dependent output should rely in relation with timeSpan[status]

// use variables timeInState, timeSpan[status], transition level to control RGB & speaker
//  timeInState: time in current state [ms]
//  timeSpan[status]: relative time constant [ms] for state cycle, selected using enum
//  transition: incrementing state transition counter, only resets at power off

void soundAndLight() {

    if      (status == initialization)  { 
        // upon entry, speaker plays 3 ascending upbeat tones for half second each
        //  then rgb slowly cycles color wheel indefinitely awaiting user input

        if      ( timeInState < (timeSpan[initialization] * 1 / 3 ) ) { playTone(100);}
        else if ( timeInState < (timeSpan[initialization] * 2 / 3 ) ) { playTone(200);}
        else if ( timeInState < (timeSpan[initialization]         ) ) { playTone(300);}
        else if ( timeInState > (timeSpan[initialization]         ) ) { continuousColorWheel(5000);}

    }
    else if (status == selection)       {
        // speaker plays short tone upon level selection change, low:easy through high:hard
        // rgb shows level (yellow:easy through red:hard)

        // turn brightness to max, color in degrees based on level, level is based on encoder selection
        brightness(100); colorInDegrees(60 * level / (LEVELS+1) );

        // play sound for short period if change in variable level
        // if (levelChanged) { playToneForPeriod( 100 * (level+1), timeSpan[selection] ); }

    }
    else if (status == introduction)    {
        // this state increases tone and frequency through three half second levels while captouch is held down
        // reminiscent of a racing game starting sequence
        // initial level is low-tone dim-green, final level is high-tone bright-green

        if      ( timeInState < (timeSpan[introduction] * 1 / 3 ) ) { playTone(400);  colorInDegrees(300); brightness(33) ;}
        else if ( timeInState < (timeSpan[introduction] * 2 / 3 ) ) { playTone(800);  colorInDegrees(300); brightness(66) ;}
        else if ( timeInState < (timeSpan[introduction]         ) ) { playTone(1200); colorInDegrees(300); brightness(100);}

    }
    else if (status == abortion)        {
        // RGB turns bright red for 1 second
        //  then two descending 1 second low tones are played
        //   for a total of 3 seconds

        if      ( timeInState < (timeSpan[abortion] * 1 / 3 ) ) { soundOff();    brightness(100); colorInDegrees(60); }
        else if ( timeInState < (timeSpan[abortion] * 2 / 3 ) ) { playTone(800); brightness(0); }
        else if ( timeInState < (timeSpan[abortion]         ) ) { playTone(400); brightness(0); }

    }
    else if (status == indication)      {
        // RGB shows sensor associated color
        // speaker plays sensor associated tone(s)

        playTone( 444 * sensor ); 
        brightness(100);
        color(sensorColor[sensor][0], sensorColor[sensor][1], sensorColor[sensor][2]); // sensor RGB value out
    }
    else if (status == response)        {
        // RGB continues to show sensor associated color

        brightness(100);
        color(sensorColor[sensor][0], sensorColor[sensor][1], sensorColor[sensor][2]); // sensor RGB value out
    }
    else if (status == lose)            {
        // series of events play out:
        //  RGB turns max white for 1 second
        //  speaker plays three low descending 1 second tones
        //  RGB fades to off over 2 seconds
        //  device is dark for 3 seconds

        if      ( timeInState < (timeSpan[lose] * 1 / 9 ) ) { soundOff();    brightness(100); color(100,100,100); }
        else if ( timeInState < (timeSpan[lose] * 2 / 9 ) ) { playTone(300); brightness(100); color(100,100,100); }
        else if ( timeInState < (timeSpan[lose] * 3 / 9 ) ) { playTone(200); brightness(100); color(100,100,100); }
        else if ( timeInState < (timeSpan[lose] * 4 / 9 ) ) { playTone(100); brightness(100); color(100,100,100); }
        else if ( timeInState < (timeSpan[lose] * 6 / 9 ) ) { soundOff();    brightnessFade(100, 0, (timeSpan[lose] * 2 / 9 ), transition); }
        else if ( timeInState < (timeSpan[lose]         ) ) { soundOff();    brightness(0); }
    }
    else if (status == levelup)         {
        // speaker plays 3 upbeat ascending tones one second each
        // RGB cycles through the color wheel level over the above time

        continuousColorWheel(level*100);

        if      ( timeInState < (timeSpan[levelup] * 1      ) ) { playTone(300); }
        else if ( timeInState < (timeSpan[levelup] * 2      ) ) { playTone(600); }
        else if ( timeInState < (timeSpan[levelup] * 3 / 9  ) ) { playTone(900); }

    }
    else if (status == win)             {

        continuousColorWheel(1000);

    }
}

int checkLevelChange(int level) {

    static int output = FALSE, lastLevel = 0;
    
    output = (level != lastLevel);

    lastLevel = level;

    return output;

}
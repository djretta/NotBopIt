#include <sound.h>
#include <timers.h>
#include <pwm.h>

int frequency = 0;  // holds tone value, default (sound off) is zero 


void SOUND_Init() { PWM_Init(); }

void playTone(int tone) {       frequency = tone;

    // add code to play tone

}

void soundOff() {               frequency = 0;

    // add code to turn sound off

}

void playToneForPeriod(int tone, int period) {

    // play a tone until period is over; reset if a new tone or new period is submitted
    static int lastTone = 0, lastPeriod = 0, timeStart = 0;

    int timeElapsed = TIMERS_GetMilliSeconds() - timeStart;

    if (tone==lastTone && period==lastPeriod) { 
        if (timeElapsed > period)   { soundOff();     }  // play period completed, turn sound off
        else                        { playTone(tone); }  // period incomplete, continue playing tone
    } else {
        // if last variables aren't the same, reset and play recently passed tone
        timeStart   = TIMERS_GetMilliSeconds();
        lastTone    = tone;
        lastPeriod  = period;
        playTone(tone);
    }
    
}
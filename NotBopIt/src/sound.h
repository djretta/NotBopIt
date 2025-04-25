/**
 * @file    sound.h
 * @brief   sound output library for the game NotBopIt
 * @author  Daniel Retta, Stephanie Scott, Danyang Hu
 * @date    January 23rd, 2025
 * */

 #ifndef sound_H
 #define sound_H

 /**
 * @function    SOUND_Init()
 * @brief       initializes the speaker PWMs
 */
void SOUND_Init();

 /**
 * @function    playTone(int frequency)
 * @brief       Play the provided tone out over the speaker.
 */
void playTone(int frequency);

 /**
 * @function    soundOff();
 * @brief       Turn the speaker off.
 */
void soundOff();

 /**
 * @function    playToneForPeriod(int tone, int period)
 * @brief       Play the provided tone out over the speaker for period [ms] or until tone is changed
 */
void playToneForPeriod(int tone, int period);

 #endif
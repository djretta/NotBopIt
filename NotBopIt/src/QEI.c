/* Author: Stephanie Scott stmscott@ucsc.edu
 * 
 * ECE-167 Sensing and Sensor Technologies
 * Lab 2 Part 1: Quadrature Encoder Interface
 * 
 * QEI.c
 * Implements the library described in QEI.h.
 *
 * Due 23:59 on 31 January, 2025
 */


#include "QEI.h"


#define MAX_INCREMENT 96


typedef struct {
    GPIO_PinState old_state;
    GPIO_PinState new_state;
} PinState_t;

static volatile PinState_t A = {SET, SET}, B = {SET, SET};

static volatile int count = 0;    // ranges from +/- 95 before rolling over to 0
static volatile int position = 0; // for recording the position in degrees

/**
 * @function QEI_Init(void)
 * @param none
 * @brief  Enables the external GPIO interrupt, anything
 *         else that needs to be done to initialize the module. 
 * @return none
*/
void QEI_Init(void)
{
    // Configure GPIO pins : PB4 PB5 
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
 
    // EXTI interrupt init
    HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
 
    // the rest of the function goes here
}


/**
 * @function QEI_GetPosition(void) 
 * @param none
 * @brief This function returns the current position of the Quadrature Encoder in degrees.      
*/
int QEI_GetPosition(void)
{
    position = count * 360 / MAX_INCREMENT;
    return position;
}


/**
 * @Function QEI_ResetPosition(void) 
 * @param  none
 * @return none
 * @brief  Resets the encoder such that it starts counting from 0.
*/
void QEI_ResetPosition()
{
    // Instruction from Lab Assignment Part 1.2:
    // QEI_ResetPosition() should reset the count (module variable) and the state of the QEI state machine

    count = 0;
    A.new_state = SET; A.old_state = SET;
    B.new_state = SET; B.old_state = SET;
}



 
 void QEI_IRQ() 
 {
    //state machine of your design
    
    // Since only one bit of the encoder changes at a time, there are two cases to handle:
    // either A changed or B changed
    if(A.new_state != A.old_state)
    {
        if(B.old_state)
        {
            if(A.new_state) // If A changed to 1 while B was 1
            {               // then the encoder moved counter-clockwise;
                count--;    // decrement count for CCW movement.
            }               // Otherwise, A must have changed to 0 while B was 1;
            else count++;   // increment count for CW movement.
        }
        else 
        {
            if(A.new_state) // If A changed to 1 while B was 0
            {               // then the encoder moved clockwise;
                count++;    // increment count for CW movement.
            }               // Otherwise, A must have changed to 0 while B was 0;
            else count--;   // decrement count for CCW movement.
        }
        A.old_state = A.new_state; // update state of encoder A
    }
    else if(B.new_state != B.old_state)
    {
        if(A.old_state)
        {
            if(B.new_state) // If B changed to 1 while A was 1
            {               // then the encoder moved clockwise;
                count++;    // increment count for CW movement.
            }               // Otherwise, B must have changed to 0 while A was 1;
            else count--;   // decrement count for CCW movement.
        }
        else
        {
            if(B.new_state) // If B changed to 1 while A was 0
            {               // then the encoder moved counter-clockwise;
                count--;    // decrement count for CCW movement.
            }               // Otherwise, B must have changed to 0 while A was 0;
            else count++;   // increment count for CW movement.
        }
        B.old_state = B.new_state; // update state of encoder B
    }

    if(count == MAX_INCREMENT || count == -MAX_INCREMENT) count = 0;
 }


  // external interrupt ISR for pin PB5 
 void EXTI9_5_IRQHandler(void) {
    // EXTI line interrupt detected 
    // PIN_5 is ENC_B
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_5) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5); // clear interrupt flag
        B.new_state = HAL_GPIO_ReadPin(GPIOB, ENC_B); // read new encoder pin state
        QEI_IRQ();
    }
 }
 

 // external interrupt ISR for pin PB4
 void EXTI4_IRQHandler(void) {
    // EXTI line interrupt detected 
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET) {
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4); // clear interrupt flag
        A.new_state = HAL_GPIO_ReadPin(GPIOB, ENC_A); // read new encoder pin state
        QEI_IRQ();
    }
 }
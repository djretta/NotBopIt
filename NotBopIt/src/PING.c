/* Author: Stephanie Scott stmscott@ucsc.edu
 * 
 * ECE-167 Sensing and Sensor Technologies
 * Lab 2 Part 2: Ultrasonic Distance Sensor
 * 
 * PING.c
 * Implements the library described in PING.h.
 *
 * Due 23:59 on 31 January, 2025
 */


 #include "PING.h"

 #define VELOCITY 34
 // speed of sound in air = 340m/s = 0.34mm/us
 // to avoid using floats, use 34 and divide by 100 in the calculation in PING_GetDistance()
 
 enum State {TRIGGER, WAIT}; // state machine states
 static volatile enum State state =  WAIT;
 
 static volatile uint32_t rise_time = 0, fall_time = 0; // for recording edge times of echo signal
 static volatile uint8_t echo_received = FALSE;
 static unsigned int flight_time = 0, distance = 0;
 
 
 /**
  * @function    PING_GetDistance(void)
  * @brief       Returns the calculated distance in mm using the sensor model determined
  *              experimentally. 
  *              No I/O should be done in this function
  * @return      distance in mm
  */
 unsigned int PING_GetDistance(void)
 {
    if(echo_received)
    {
        flight_time = fall_time - rise_time;
        echo_received = FALSE;
    }
    distance = (flight_time * VELOCITY) / 200;
     return distance;
 }
 
 /**
  * @function    PING_GetTimeofFlight(void)
  * @brief       Returns the raw microsecond duration of the echo from the sensor.
  *              NO I/O should be done in this function.
  * @return      time of flight in uSec
  */
 unsigned int PING_GetTimeofFlight(void)
 {
     if(echo_received)
     {
         flight_time = fall_time - rise_time;
         echo_received = FALSE;
     }
     return flight_time;
 }
 
 
 //Sets up both the timer extrenal interrupt peripherals along with their
 //Also configures PWM_5 as a GPIO output for the trigger pin. 
 char PING_Init(void) {
     // init other libraries
     BOARD_Init();
     TIMER_Init();
 
     // this block initializes the GPIO output pin (PB8, PWM_5 on shield)
     GPIO_InitTypeDef GPIO_InitStruct = {0};
     GPIO_InitStruct.Pin = GPIO_PIN_8;
     GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
     GPIO_InitStruct.Pull = GPIO_NOPULL;
     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
 
     // this block inits the timer generated interrupt
     TIM_ClockConfigTypeDef sClockSourceConfig = {0};
     TIM_MasterConfigTypeDef sMasterConfig = {0};
     htim3.Instance = TIM3;
     htim3.Init.Prescaler = 83; // divide by 1 prescaler (84-1) = 1 Mhz tick
     htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
     htim3.Init.Period = 59999; // period of (number of clock cycles between) interrupts
     htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
     htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
     if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
     {
         return ERROR;
     }
     sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
     if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
     {
         return ERROR;
     }
     sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
     sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
     if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
     {
         return ERROR;
     }
     HAL_TIM_Base_Start_IT(&htim3); // start interrupt
 
     // this block inits the external pin-generted interrupt on any change of pin PC0
     GPIO_InitStruct.Pin = GPIO_PIN_0;
     GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
     GPIO_InitStruct.Pull = GPIO_NOPULL;
     HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
     HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
     HAL_NVIC_EnableIRQ(EXTI0_IRQn);
 
     return SUCCESS;
 }
 
  // external interrupt ISR for pin PC0,
  // used to record the state of the sensor's Echo pin
  // This interrupt is triggered on both the rising and falling edges of the Echo pin.
 
  void EXTI0_IRQHandler(void) {
     // EXTI line interrupt detected 
     if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET) {
         __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0); // clear interrupt flag
  
         // PING sensor echo detection logic
         if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0)){  // If a rising edge was detected,
             rise_time = TIMERS_GetMicroSeconds(); // record the microsecond time.
         }
         else {                                    // Otherwise, a falling edge was detected;
             fall_time = TIMERS_GetMicroSeconds(); // record the microsecond time and
             echo_received = TRUE;                 // flag that an echo signal was received.
         }
     }
  }
   
  // From Appendix on Timer interrupts:
  // We can generate an interrupt when the timer reaches some desired value.
  // This is done at initialization by setting the period field to a value between 0-65535. 
  // You can also change this value at runtime by modifying the ARR register associated with the timer. 
  // For example, if we were using TIM3, we would modify the period by doing 
  // TIM3->ARR = x;  // change the period of TIM3 to x
 
  // TIM3 ISR
  void TIM3_IRQHandler(void) {
     if (__HAL_TIM_GET_IT_SOURCE(&htim3, TIM_IT_UPDATE) != RESET) {
         __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE); // clear interrupt flag
   
         // state machine of your design
 
         switch(state)
         {
             case(TRIGGER):
                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET); // trigger sensor
                 TIM3->ARR = 9; // generate interrupt in 10us
                 state = WAIT;  // switch state machine to WAIT state
                 break;
             case(WAIT):
                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET); // lower trigger pin
                 TIM3->ARR = 59999; // generate interrupt in 60ms
                 state = TRIGGER;   // switch state machine to TRIGGER state
                 break;
         }
     }
  }
#include <sensors.h>
#include <stdio.h>
#include <stdlib.h>
#include <timers.h>
#include <pwm.h>

static int degrees_new = 0, degrees_old = 0;

uint32_t timeInitial = 0, timeFinal = 0, timeResponse = 0;

void SENSORS_Init() {
    QEI_Init();
    BNO055_Init();
    ADC_Init();
    PING_Init();
    PWM_Init();
    PWM_SetDutyCycle(PWM_5, 50); // for ping sensor
}

int encoderChangeCW()           {
    
    degrees_new = QEI_GetPosition();
    if (degrees_new >= (degrees_old + 15)){
        degrees_old = degrees_new;

            printf("\n\nEncoder moved ClockWise.\n\n");        // diagnostic data printed to serial

        return TRUE;
    }
    else return 0;
}
int encoderChangeCCW()          {
    
    degrees_new = QEI_GetPosition();
    if (degrees_new <= (degrees_old - 15)){
        degrees_old = degrees_new;
        return TRUE;

        printf("\n\nEncoder moved CounterClockWise.\n\n");        // diagnostic data printed to serial

    }
    else return 0;
}
int captouchPressed()           {return HAL_GPIO_ReadPin(GPIOC, TOUCH_PIN);}
int captouchReleased()          {return !HAL_GPIO_ReadPin(GPIOC, TOUCH_PIN);}
int captouchHeld(int duration)  {
    static int timestamp = 0; timestamp = TIMERS_GetMilliSeconds();
    while (HAL_GPIO_ReadPin(GPIOC, TOUCH_PIN)){
        if (TIMERS_GetMilliSeconds() - timestamp > duration) return 1;}
    return 0;}

// checks all sensors for activation flag and returns sensor
int sensorActivated(int sensor) {

    // Get face up ONCE for consistency throughout this function
    sensor_t face_up = sensorFaceUp();
    sensor_t activated = none;
    
    timeInitial = TIMERS_GetMicroSeconds();

    if      ( face_up == flex        && flexActivated()       )   { activated =  flex;       }
    else if ( face_up == piezo       && piezoActivated()      )   { activated =  piezo;      }
    else if ( face_up == rotary      && rotaryActivated()     )   { activated =  rotary;     }
    else if ( face_up == ultrasonic  && ultrasonicActivated() )   { activated =  ultrasonic; }
    else if ( face_up == captouch    && captouchActivated()   )   { activated =  captouch;   }
    else if ( face_up == infrared    && infraredActivated()   )   { activated =  infrared;   }
    else if ( sensor  == IMU         && IMUActivated()        )   { activated =  IMU;        }

    timeFinal = TIMERS_GetMicroSeconds();
    timeResponse = timeFinal - timeInitial;

    return activated;
}

int sensorFaceUp(){

    int AccX = (BNO055_ReadAccelX() - X_ACC_BIAS),
        AccY = (BNO055_ReadAccelY() - Y_ACC_BIAS),            
        AccZ = (BNO055_ReadAccelZ() - Z_ACC_BIAS);
    
    if      (AccZ > 900)  {return flex;}
    else if (AccZ < -900) {return captouch;}
    else if (AccX > 900)  {return rotary;}
    else if (AccX < -900) {return piezo;}
    else if (AccY > 900)  {return infrared;}
    else if (AccY < -900) {return ultrasonic;}
    else return none;
}

// filtered sensor reading functions
// ensure activation meets criteria for correct interaction
// note: captouch, IR, flex, ping, and piezo all use the same rising edge detection algorithm

int captouchActivated(){     
    static int prev_state     =  FALSE;
    int        rising_edge    =  FALSE;
    int        current_state  =  HAL_GPIO_ReadPin(GPIOC, TOUCH_PIN);

    // Detect rising edge
    if (current_state == TRUE && prev_state == FALSE) { rising_edge = TRUE; }
    // Store current state for next call
    prev_state = current_state;  
    return rising_edge;
}

int infraredActivated(){ 
    static int prev_state     =  FALSE;
    int        rising_edge    =  FALSE;
    int        current_state  =  HAL_GPIO_ReadPin(GPIOC, IR_PIN);

    // Detect rising edge
    if (current_state == TRUE && prev_state == FALSE) { rising_edge = TRUE; }
    // Store current state for next call
    prev_state = current_state;  
    return rising_edge; 
}

int flexActivated(){
    static int prev_state     =  FALSE;
    int        rising_edge    =  FALSE;
    int        current_state  =  ADC_Read(FLEX_PIN) < 2100;

    // Detect rising edge
    if (current_state == TRUE && prev_state == FALSE) { rising_edge = TRUE; }
    // Store current state for next call
    prev_state = current_state;  
    return rising_edge;
}

int ultrasonicActivated(){ 
    static int prev_state     =  FALSE;
    int        rising_edge    =  FALSE;
    int        current_state  =  PING_GetDistance() < 60;

    // Detect rising edge
    if (current_state == TRUE && prev_state == FALSE) { rising_edge = TRUE; }
    // Store current state for next call
    prev_state = current_state;  
    return rising_edge;          
}

int piezoActivated(){
    static int prev_state     =  FALSE;
    int        rising_edge    =  FALSE;
    int        current_state  =  ADC_Read(PIEZO_PIN) > 35;

    // Detect rising edge
    if (current_state == TRUE && prev_state == FALSE) { rising_edge = TRUE; }
    // Store current state for next call
    prev_state = current_state; 
    return rising_edge;
}

int rotaryActivated(){
    
    static int total_rotation = 0;
    static int last_position  = 0;
    int current_position = QEI_GetPosition();
    int delta = current_position - last_position;

    if (delta == 0) return 0;               // No movement detected
    
    total_rotation += abs(delta);           // Accumulate movement if in the correct direction
    last_position = current_position;       // Update position tracking

    if (total_rotation >= 180){             // Player met rotation requirement
        total_rotation = 0;                 // Reset rotation for next time
        return 1;
    }
    
    return 0;
}





int IMUActivated(){
    
    static sensor_t initial_face = none;
    static int waiting_for_flip = 0;

    sensor_t current_face = sensorFaceUp();

          // printf("Initial face: %d, Current face: %d\n", initial_face, current_face);

    // Store the initial face the first time IMUActivated() is called
    if (initial_face == none && !waiting_for_flip) {
        int AccX = (BNO055_ReadAccelX() - X_ACC_BIAS),
            AccY = (BNO055_ReadAccelY() - Y_ACC_BIAS),            
            AccZ = (BNO055_ReadAccelZ() - Z_ACC_BIAS);

        int absX = abs(AccX), absY = abs(AccY), absZ = abs(AccZ);

        if          ( absX > absY && absX > absZ ) { // check if X has greatest magnitude
            if (AccX > 0)   { initial_face = rotary; }
            else            { initial_face = piezo; }

        } else if   ( absY > absX && absY > absZ ) { // check if Y has greatest magnitude
            if (AccY > 0)   { initial_face = infrared; } 
            else            { initial_face = ultrasonic; }

        } else if   ( absZ > absX && absZ > absY ) { // check if Z has greatest magnitude
            if (AccZ > 0)   { initial_face = flex; } 
            else            { initial_face = captouch; }
        }
        waiting_for_flip = 1;
            // printf("IMU: Initial face recorded as %d, waiting for flip\n", initial_face);
        return 0;
    }
    
    // Define the expected opposite faces and Check if flipped to opposite face
    if (waiting_for_flip) {
        int flipped = 0;
        
        switch (initial_face) {
            case captouch:   flipped = (current_face == flex);       break;
            case flex:       flipped = (current_face == captouch);   break;
            case rotary:     flipped = (current_face == piezo);      break;
            case piezo:      flipped = (current_face == rotary);     break;
            case infrared:   flipped = (current_face == ultrasonic); break;
            case ultrasonic: flipped = (current_face == infrared);   break;
            default: break;}
        
        // Once flipped, reset initial_face and waiting_for_flip for next time and return success
        if (flipped) {
            printf("IMU: Flip detected! From %d to %d\n", initial_face, current_face);
            initial_face = none;
            waiting_for_flip = 0;
            return 1;
        }
    }
    return 0;
}

void GPIO_Init(){
    // Configure GPIO pins : PC5 PC12
    GPIO_InitTypeDef GPIO_InitStruct_C = {0};
    GPIO_InitStruct_C.Pin = GPIO_PIN_5|GPIO_PIN_12;
    GPIO_InitStruct_C.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct_C.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct_C);
 
    // In case interrupts are needed
    // EXTI interrupt init
    // HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
    // HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    // HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    // HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}
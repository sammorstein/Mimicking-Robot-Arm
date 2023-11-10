#include "msp.h"
#include "driverlib.h"
#include "msp432p401r.h"

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>
//#include <string.h>


volatile int PWM; //PWM val
volatile char direction; //motor direction
volatile char running1 = 0; //direction running check
volatile char running2 = 0; //other direction check
volatile float angle; //encoder angle
volatile float shoulderAng = 90; //target angle
volatile float kp = 0.5; //proportional gain
volatile float ki = 1; //integral gain

volatile float error;//error
volatile float accError //integral error

//volatile float kd = 1;

void main(void)
{
//reset errors when desAngle is changed
}

void TA0_0_IRQHandler(void){
    error = shoulderAng-angle; //error
    accError = accError + error*.02; //accel error
    //deltaError = (error-prevError)/.02;
    //prevError = error;

    //PWM = 60000*(kp*error+ki*accError+kd*deltaError);
    PWM = 60000*(kp*error+ki*accError); //PWM value

    if(PWM>60000){ //if value is larger than possible limit it to max
        PWM = 60000;
    }



    if(error > 0){
        direction = 0; //ccw
    }
    if(error < 0){
        direction = 1; //cw
    }

    if(direction == 0 && running1 == 1){ //ccw direction
        Interrupt_disableInterrupt(INT_PORT5);
        TA0CCR2 = 0; //disable cw pin
        TA0CCR3 = 0;
        /*int i = 0;
            while(i < 50){
                i++;
            }*/
        TA0CCR1 = PWM; //enable ccw pin
        TA0CCR4 = PWM;
        Interrupt_enableInterrupt(INT_PORT5);
    }

    if (direction ==1 && running2 ==1){
        Interrupt_disableInterrupt(INT_PORT5);
        TA0CCR1 = 0; //disable ccw pin
        TA0CCR4 = 0;
        /*int i = 0;
                    while(i < 50){
                        i++;
                    }*/
        TA0CCR2 = PWM; //enable cw pin
        TA0CCR3 = PWM;
        Interrupt_enableInterrupt(INT_PORT5);
    }

    if (direction == 0 && running1 == 0){
        Interrupt_disableInterrupt(INT_PORT5);
        TA0CCR2 = 0; //disable cw pin
        TA0CCR3 = 0;
        /*int i = 0;
            while(i < 50){
                i++;
            }*/
        TA0CCR1 = 0; //disable ccw pin
        TA0CCR4 = 0;
        Interrupt_enableInterrupt(INT_PORT5);
    }

    if (direction == 1 && running2 == 0){
        Interrupt_disableInterrupt(INT_PORT5);
        TA0CCR2 = 0; //disable cw pin
        TA0CCR3 = 0;
        /*int i = 0;
            while(i < 50){
                i++;
            }*/
        TA0CCR1 = 0; //disable ccw pin
        TA0CCR4 = 0;
        Interrupt_enableInterrupt(INT_PORT5);
    }
    // Clear Timer A Interrupt Flag
    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

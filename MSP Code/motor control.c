#include "msp.h"
#include "driverlib.h"
#include "msp432p401r.h"

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>
//#include <string.h>

volatile int PWM = 45000; //PWM val
volatile char direction; //motor direction
volatile char running1 = 0; //running check
volatile char running2 = 0; //running check other direction

// Configure Timer A for UpMode operation
const Timer_A_UpModeConfig upConfig_0 =
{    TIMER_A_CLOCKSOURCE_SMCLK,
     TIMER_A_CLOCKSOURCE_DIVIDER_1,
     60000,
     TIMER_A_TAIE_INTERRUPT_DISABLE,
     TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
     TIMER_A_DO_CLEAR
};



void main(void)
{
    CS_setDCOFrequency(3E+6);                       // Sets clock speed
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    // Set up Sub-Master Clock
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    ///// *** Configure Timer A and its interrupt *** /////
    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig_0);
    Interrupt_enableInterrupt(INT_TA0_0);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    TA0CCR1 = 0;    // Set duty cycle on pin 2.4 to 0%
    TA0CCR2 = 0;     //Set duty cycle on pin 2.5 to 0%
    TA0CCR3 = 0;    //Set duty cycle on pin 2.6 to 0%
    TA0CCR4 = 0;    // Set duty cycle on pin 2.7 to 0%
    TA0CCTL1 = OUTMOD_3 ;    //Set/Reset Mode
    TA0CCTL2 = OUTMOD_7;    //Reset/Set Mode
    TA0CCTL3 = OUTMOD_3;    //Set/Reset Mode
    TA0CCTL4 = OUTMOD_7;    //Reset/Set Mode
    P2SEL0 |= 0xF0;
    P2SEL1 &= ~0xF0;
    P2DIR |= 0xF0;

    // Configure GPIO ports 2.4-2.7 for motor driving
    //GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    //2.4 is 1
    //2.5 is 2
    //2.6 is 3
    //2.7 is 4

    /*GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN6); //PNP init
    GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN5|GPIO_PIN7); //NPN init*/

    /*GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN5|GPIO_PIN6);
    GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN6);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN5);*/

    char Button1 = 1; //button1 tracker
    char Button2 = 1; //button2 tracker
    char prevButton1 = 1;
    char prevButton2 = 1; //tracks button2's previous state for change to prevent lights cycling

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1); //buttons
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4); //buttons

    GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0); //LED1
    GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN0); //LED2

    GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN0);

    Interrupt_disableMaster();

    Interrupt_enableMaster();

    while(1){
        //PNP turn on when output set low
        //NPN turn on when output set high

        Button1 = GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1); //get Button1 value
        Button2 = GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4); //get Button2 value


        if(Button1==GPIO_INPUT_PIN_LOW && running1 == 0 && Button1 != prevButton1){
            Interrupt_disableInterrupt(INT_TA0_0);
            /*int i = 0;
                        while(i < 5000){
                            i++;
                        }*/
            direction = 0;
            running1 = 1;
            GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN0);

            Interrupt_enableInterrupt(INT_TA0_0);
        }
        else if(Button1==GPIO_INPUT_PIN_LOW && running1 == 1 && Button1 != prevButton1){
            Interrupt_disableInterrupt(INT_TA0_0);
            /*int i = 0;
                                    while(i < 5000){
                                        i++;
                                    }*/
            direction = 0;
            running1 = 0;
            GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN0);

            Interrupt_enableInterrupt(INT_TA0_0);
        }
        if(Button2==GPIO_INPUT_PIN_LOW  && running2 == 0 && Button2 != prevButton2){ //button s2 pressed and not currently running
            Interrupt_disableInterrupt(INT_TA0_0);
            /*int i = 0;
                    while(i < 5000){
                        i++;
                    }*/
            direction = 1;
            running2 = 1;
            GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0);

            Interrupt_enableInterrupt(INT_TA0_0);
        } else if(Button2==GPIO_INPUT_PIN_LOW && running2 == 1 && Button2 != prevButton2){ //button s2 pressed and running
            Interrupt_disableInterrupt(INT_TA0_0);
            /*int i = 0;
                    while(i < 5000){
                        i++;
                    }*/
            direction = 1;
            running2 = 0;
            GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN0);

            Interrupt_enableInterrupt(INT_TA0_0);
        }

        prevButton1 = Button1; //updates Button1's previous state
        prevButton2 = Button2; //updates Button2's previous state
    }

}



//////////////// *** Timer A Interrupt Service Routine *** ///////////////////
void TA0_0_IRQHandler(void){


    //PWM = 60000*(int)result/16384;     // Update PWM Value

    // Limit PWM duty cycle to 5-10%
    /*if(PWM<3000){
        PWM=3000; }  // Limit to above 5% duty cycle
    else if(PWM>6000){
        PWM=6000; }  // Limit to below 10% duty cycle*/
    /*if(PWM < 60000){
    PWM = PWM+500;
    } else {
        PWM = 30000;
    }*/

    if(direction == 0 && running1 == 1){ //ccw direction
        TA0CCR2 = 0; //disable cw pin
        TA0CCR3 = 0;
        /*int i = 0;
        while(i < 50){
            i++;
        }*/
        TA0CCR1 = PWM; //enable ccw pin
        TA0CCR4 = PWM;
    }

    if (direction ==1 && running2 ==1){
        TA0CCR1 = 0; //disable ccw pin
        TA0CCR4 = 0;
        /*int i = 0;
                while(i < 50){
                    i++;
                }*/
        TA0CCR2 = PWM; //enable cw pin
        TA0CCR3 = PWM;
    }

    if (direction == 0 && running1 == 0){
        TA0CCR2 = 0; //disable cw pin
        TA0CCR3 = 0;
        /*int i = 0;
        while(i < 50){
            i++;
        }*/
        TA0CCR1 = 0; //disable ccw pin
        TA0CCR4 = 0;
    }

    if (direction == 1 && running2 == 0){
        TA0CCR2 = 0; //disable cw pin
        TA0CCR3 = 0;
        /*int i = 0;
        while(i < 50){
            i++;
        }*/
        TA0CCR1 = 0; //disable ccw pin
        TA0CCR4 = 0;
    }

    // Verify PWM values
    printf("PWM Value: %i \r\n\n", PWM) ;

    // Clear Timer A Interrupt Flag
    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

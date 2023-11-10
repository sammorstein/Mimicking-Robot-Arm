#include "msp.h"
#include <driverlib.h>
#include <stdio.h>
#include "msp432p401r.h"

int angle = 0;
float dutyCycle = .075;
int PWM = 0;
int i = 0;
int j = 4500;

// Configure Timer A for UpMode operation
const Timer_A_UpModeConfig upConfig_0 =
{    TIMER_A_CLOCKSOURCE_SMCLK,
     TIMER_A_CLOCKSOURCE_DIVIDER_1,
     60000, // 20 ms
     TIMER_A_TAIE_INTERRUPT_DISABLE,
     TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
     TIMER_A_DO_CLEAR
};

// Main Loop
void main(){

    // Disable the Watchdog Timer
    WDT_A_holdTimer() ;

    // Set DCO Frequency to 3 Mhz
    CS_setDCOFrequency(3E+6);

    // Set up Sub-Master Clock
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    ///// *** Configure Timer A and its interrupt *** /////
    Timer_A_configureUpMode(TIMER_A2_BASE, &upConfig_0);
    //Interrupt_enableInterrupt(INT_TA2_0);
    Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);

    ///// *** Setup Pin 5.6 as PWM output signal *** /////
    P5->SEL0 |= 0x40 ;    // Set bits 4-7 of P2SEL0 to enable TA0.1 functionality on P2.4
    P5->SEL1 &= ~0x40 ;   // Clear bit 4-7 of P2SEL1 to enable TA0.1 functionality on P2.4
    P5->DIR |= 0x40 ;     // Set pins 2.4-2.7 as an output pin
    TA2CCR1 = 0 ;    // Set duty cycle on pin 2.4 to 7.5%
    TA2CCTL1 = OUTMOD_7 ;    // Macro which is equal to 0x00e0, defined in msp432p401r.h

    // Enable NVIC
    Interrupt_enableMaster();

    // Infinite loop
    while(1){


        angle = 0;
        for (i = 0; i < 200000; i++) {};
        dutyCycle = ((float) -1*angle/1982) + 0.131;
        //dutyCycle = ((float) angle/1982) + 0.0175;
        PWM = 60000*dutyCycle;
        TA2CCR1 = PWM; // Save new PWM value to register
        printf("PWM Value: %i \r\n\n", PWM);

        angle = 45;
        for (i = 0; i < 200000; i++) {};
        dutyCycle = ((float) -1*angle/1982) + 0.131;
        //dutyCycle = ((float) angle/1982) + 0.0175;
        PWM = 60000*dutyCycle;
        TA2CCR1 = PWM; // Save new PWM value to register
        printf("PWM Value: %i \r\n\n", PWM);

        angle = 90;
        for (i = 0; i < 200000; i++) {};
        dutyCycle = ((float) -1*angle/1982) + 0.131;
        //dutyCycle = ((float) angle/1982) + 0.0175;
        PWM = 60000*dutyCycle;
        TA2CCR1 = PWM; // Save new PWM value to register
        printf("PWM Value: %i \r\n\n", PWM);

        angle = 112;
        for (i = 0; i < 200000; i++) {};
        dutyCycle = ((float) -1*angle/1982) + 0.131;
        //dutyCycle = ((float) angle/1982) + 0.0175;
        PWM = 60000*dutyCycle;
        TA2CCR1 = PWM; // Save new PWM value to register
        printf("PWM Value: %i \r\n\n", PWM);

        angle = 135;
        for (i = 0; i < 200000; i++) {};
        dutyCycle = ((float) -1*angle/1982) + 0.131;
        //dutyCycle = ((float) angle/1982) + 0.0175;
        PWM = 60000*dutyCycle;
        TA2CCR1 = PWM; // Save new PWM value to register
        printf("PWM Value: %i \r\n\n", PWM);

        angle = 180;
        for (i = 0; i < 200000; i++) {};
        dutyCycle = ((float) -1*angle/1982) + 0.131;
        //dutyCycle = ((float) angle/1982) + 0.0175;
        PWM = 60000*dutyCycle;
        TA2CCR1 = PWM; // Save new PWM value to register
        printf("PWM Value: %i \r\n\n", PWM);

        angle = 225;
        for (i = 0; i < 200000; i++) {};
        dutyCycle = ((float) -1*angle/1982) + 0.131;
        //dutyCycle = ((float) angle/1982) + 0.0175;
        PWM = 60000*dutyCycle;
        TA2CCR1 = PWM; // Save new PWM value to register
        printf("PWM Value: %i \r\n\n", PWM);

//        for (i = 0; i < 200000; i++) {};
//        PWM = 60000*.12;
//        TA0CCR1 = PWM; // Save new PWM value to register
//        printf("PWM Value: %i \r\n\n", PWM);

//        for (i = 0; i < 200000; i++) {};
//        PWM = 60000*.131;
//        TA2CCR1 = PWM; // Save new PWM value to register
//        printf("PWM Value: %i \r\n\n", PWM);
//        angle = 45;
//        for (i = 0; i < 100000; i++) {};
//        dutyCycle = ((float) angle/3600) + 0.0175;
//        PWM = 60000*dutyCycle;
//        TA0CCR1 = PWM; // Save new PWM value to register
//        printf("PWM Value: %i \r\n\n", PWM);
//
//        angle = 90;
//        for (i = 0; i < 100000; i++) {};
//        dutyCycle = ((float) angle/3600) + 0.0175;
//        PWM = 60000*dutyCycle;
//        TA0CCR1 = PWM; // Save new PWM value to register
//        printf("PWM Value: %i \r\n\n", PWM);
//
//        angle = 135;
//        for (i = 0; i < 100000; i++) {};
//        dutyCycle = ((float) angle/3600) + 0.025;
//        PWM = 60000*dutyCycle;
//        TA0CCR1 = PWM; // Save new PWM value to register
//        printf("PWM Value: %i \r\n\n", PWM);
//
//        angle = 180;
//        for (i = 0; i < 100000; i++) {};
//        dutyCycle = ((float) angle/3600) + 0.025;
//        PWM = 60000*dutyCycle;
//        TA0CCR1 = PWM; // Save new PWM value to register
//        printf("PWM Value: %i \r\n\n", PWM);
//
//
//        angle = 225;
//        for (i = 0; i < 100000; i++) {};
//        dutyCycle = ((float) angle/3600) + 0.025;
//        PWM = 60000*dutyCycle;
//        TA0CCR1 = PWM; // Save new PWM value to register
//        printf("PWM Value: %i \r\n\n", PWM);
    }
}

////////////////// *** Timer A Interrupt Service Routine *** ///////////////////
//void TA2_0_IRQHandler(void){
//
//    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0); // Clear Timer A Interrupt Flag
//}

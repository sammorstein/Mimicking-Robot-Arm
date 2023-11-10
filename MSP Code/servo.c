#include "msp.h"
#include <driverlib.h>
#include <stdio.h>
#include "msp432p401r.h"

volatile int PWM = 0;

// Configure Timer A for UpMode operation
const Timer_A_UpModeConfig upConfig_0 =
{    TIMER_A_CLOCKSOURCE_SMCLK,
     TIMER_A_CLOCKSOURCE_DIVIDER_1,
     20000,
     TIMER_A_TAIE_INTERRUPT_DISABLE,
     TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
     TIMER_A_DO_CLEAR
};

// Main Loop
void main(){

    // Disable the Watchdog Timer
    WDT_A_holdTimer() ;

    // Set DCO Frequency to 3 Mhz
    CS_setDCOFrequency(1E+6);

    // Set up Sub-Master Clock
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    ///// *** Configure Timer A and its interrupt *** /////
    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig_0);
    Interrupt_enableInterrupt(INT_TA0_0);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    ///// *** Setup Pin 2.4 as PWM output signal *** /////
    P2->SEL0 |= 0xF0 ;    // Set bits 4-7 of P2SEL0 to enable TA0.1 functionality on P2.4
    P2->SEL1 &= ~0xF0 ;   // Clear bit 4-7 of P2SEL1 to enable TA0.1 functionality on P2.4
    P2->DIR |= 0xF0 ;     // Set pins 2.4-2.7 as an output pin
    TA0CCR1 = 0 ;    // Set duty cycle on pin 2.4 to 7.5%
    TA0CCTL1 = OUTMOD_7 ;    // Macro which is equal to 0x00e0, defined in msp432p401r.h

    // Enable NVIC
    Interrupt_enableMaster();

    // Infinite loop
    while(1){

    }
}

//////////////// *** Timer A Interrupt Service Routine *** ///////////////////
void TA0_0_IRQHandler(void){
    PWM = 1500; // Update PWM Value
    TA0CCR1 = PWM; // Save new PWM value to register
//    printf("PWM Value: %i \r\n\n", PWM);
//    PWM = PWM + 50; // this should increase the duty cycle, thus increasing the speed
    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0); // Clear Timer A Interrupt Flag
}

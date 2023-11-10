#include "msp.h"
#include "driverlib.h"
#include "msp432p401r.h"

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>
//#include <string.h>

volatile int count = -720; //encoder counter
volatile int Astat; //A channel
volatile int Bstat; //B channel
volatile int prevA; //prev A val
volatile int prevB; //prev B val
volatile char direction; //direction
volatile float angle = -90; //encoder angle

void main(void)
{

        CS_setDCOFrequency(3E+6);                       // Sets clock speed
        WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

        GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN0);  // Encoder A
        GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN1);   //Encoder B

        // 1. Disable All Interrrupts
       Interrupt_disableMaster();

       // 2. Edge select
       GPIO_interruptEdgeSelect(GPIO_PORT_P5,GPIO_PIN0,GPIO_LOW_TO_HIGH_TRANSITION);
       GPIO_interruptEdgeSelect(GPIO_PORT_P5,GPIO_PIN1,GPIO_LOW_TO_HIGH_TRANSITION);
       //GPIO_interruptEdgeSelect(GPIO_PORT_P5,GPIO_PIN0,GPIO_HIGH_TO_LOW_TRANSITION); //can any edge transition work?
       //GPIO_interruptEdgeSelect(GPIO_PORT_P5,GPIO_PIN1,GPIO_HIGH_TO_LOW_TRANSITION);

       // 3.Clear interrupt flags
       GPIO_clearInterruptFlag(GPIO_PORT_P5,GPIO_PIN0);
       GPIO_clearInterruptFlag(GPIO_PORT_P5,GPIO_PIN1);

       // 4. Arm interrupts on those pins
       GPIO_enableInterrupt(GPIO_PORT_P5,GPIO_PIN0);
       GPIO_enableInterrupt(GPIO_PORT_P5,GPIO_PIN1);

       // 5. Set priority
       Interrupt_setPriority(INT_PORT5,1);

       // 6. Enable interrupts on those ports
       Interrupt_enableInterrupt(INT_PORT5);

       // 7. Enable NVIC
       Interrupt_enableMaster();  // Un-comment this line to use interrupts

       while(1){}

}

// Interrupt Service Routine for Port 6
void PORT5_IRQHandler(void){
    //count++;
    //count = 0;                // Reset counter
    //P6->IFG &= ~0x02;         // clear interrupt flag (“acknowledge”)

    Astat = GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN0);
    Bstat = GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1);

    //Astat = GPIO_getInterruptStatus(GPIO_PORT_P5, GPIO_PIN0);
    //Bstat = GPIO_getInterruptStatus(GPIO_PORT_P5, GPIO_PIN1);

    if(Astat == 1 && Bstat==0 && prevA == 1 && prevB == 1){ //cw check
        direction = 1; //cw
        count--; //decreases angle read
    }
    else if(Astat == 1 && Bstat==1 && prevA == 1 && prevB == 0){ //other possibility cw check
        direction = 1; //cw
        count--; //decreases angle read
    }
    else if(Astat == 0 && Bstat==1 && prevA == 1 && prevB == 1){ //ccw check
        direction = 0; //ccw
        count++; //increases angle read
    }
    else if(Astat == 1 && Bstat==1 && prevA == 0 && prevB == 1){ //other possibility ccw check
        direction = 0; //ccw
        count++; //increases angle read
    }

    if(abs(count)==2880){ //angle overflow reset
        count = 0;
    }


    /*if(Astat == 0){
        printf("A=0");
    }
    if (Bstat == 0){
        printf("B=0");
    }*/


    GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN0);  //A Channel
    GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN1);   //B Channel
    //printf("%d",count);
    //printf("\n");

    prevA = Astat;
    prevB = Bstat;

    angle = 360*(float)count/2880; //tick to angle conversion
}

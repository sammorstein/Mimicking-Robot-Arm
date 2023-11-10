#include "msp.h"
#include "driverlib.h"
#include "msp432p401r.h"

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//UART configuration
const eUSCI_UART_Config UART_init =
{
 EUSCI_A_UART_CLOCKSOURCE_SMCLK,
 19,
 8,
 85,
 EUSCI_A_UART_NO_PARITY,
 EUSCI_A_UART_LSB_FIRST,
 EUSCI_A_UART_ONE_STOP_BIT,
 EUSCI_A_UART_MODE,
 EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};


// Configure Timer A for UpMode operation
const Timer_A_UpModeConfig upConfig_0 =
{    TIMER_A_CLOCKSOURCE_SMCLK,
     TIMER_A_CLOCKSOURCE_DIVIDER_1,
     60000, //0.02 sec
     TIMER_A_TAIE_INTERRUPT_DISABLE,
     TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
     TIMER_A_DO_CLEAR
};

//Timer Configuration
const Timer_A_UpModeConfig upConfig_1 =
{    TIMER_A_CLOCKSOURCE_SMCLK,
     TIMER_A_CLOCKSOURCE_DIVIDER_64,
     1563, //0.1 sec
     TIMER_A_TAIE_INTERRUPT_DISABLE,
     TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
     TIMER_A_DO_CLEAR
};



volatile uint8_t letter; //current letter
volatile char buffer[16]; //char buffer
volatile int strcount = 0; //current pos in buffer
volatile char string; //check to fill buffer
volatile float shoulderAng; //shoulder angle
volatile float elbowAng; //elbow angle

volatile int count = -720; //encoder counter (start at -90 deg)
volatile int Astat; //A channel
volatile int Bstat; //B channel
volatile int prevA; //prev A val
volatile int prevB; //prev B val
volatile char encDirection; //direction
volatile float angle = -90; //encoder angle

volatile int PWM; //PWM val
volatile char direction; //motor direction
volatile char running1 = 0; //direction running check
volatile char running2 = 0; //other direction check
volatile float angle; //encoder angle
//volatile float shoulderAng = 90; //target angle
volatile float kp = 0.05; //proportional gain
volatile float ki = 0.2; //integral gain
volatile float error;//error
volatile float accError; //integral error


void main(void)
{
    CS_setDCOFrequency(3E+6);                       // Sets clock speed
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer


    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN0);  // Encoder A
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN1);   //Encoder B

    //red led setup
    GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);


    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig_0);
    Interrupt_enableInterrupt(INT_TA0_0);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    TA0CCR1 = 0;    // Set duty cycle on pin 2.4 to 0%
    TA0CCR2 = 0;     //Set duty cycle on pin 2.5 to 0%
    TA0CCR3 = 0;    //Set duty cycle on pin 2.6 to 0%
    TA0CCR4 = 0;    // Set duty cycle on pin 2.7 to 0%
    TA0CCTL1 = OUTMOD_3;    //Set/Reset Mode
    TA0CCTL2 = OUTMOD_7;    //Reset/Set Mode
    TA0CCTL3 = OUTMOD_3;    //Set/Reset Mode
    TA0CCTL4 = OUTMOD_7;    //Reset/Set Mode
    P2SEL0 |= 0xF0;
    P2SEL1 &= ~0xF0;
    P2DIR |= 0xF0;

    ///// *** Configure Timer A and its interrupt *** /////
    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig_1);
    Interrupt_enableInterrupt(INT_TA1_0);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    Interrupt_disableMaster(); //step 1

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
    Interrupt_setPriority(INT_PORT5,1); //encoder priority important
    Interrupt_setPriority(INT_TA0_0,5); //Timer A0 not as important
    Interrupt_setPriority(INT_TA1_0,4);


    // 6. Enable interrupts on those ports
    Interrupt_enableInterrupt(INT_PORT5);



    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
                                               GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION); /* Selecting P1.2 and P1.3 in UART mode */
    UART_initModule(EUSCI_A0_BASE,&UART_init); //Initialize UART Module 0

    UART_enableModule(EUSCI_A0_BASE); //Enable UART module

    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT); //enable UART interrupts

    //Interrupt_enableInterrupt(INT_EUSCIA0); //step 6

    Interrupt_enableMaster(); //step 7

    while(1){}


}

void TA0_0_IRQHandler(void){
    error = shoulderAng-angle; //error
    accError = accError + error*.02; //accel error
    //deltaError = (error-prevError)/.02;
    //prevError = error;

    //PWM = 60000*(kp*error+ki*accError+kd*deltaError);
    PWM = abs(60000*(kp*error+ki*accError)); //PWM value

    if(PWM>60000){ //if value is larger than possible limit it to max
        PWM = 60000;
    }


    //printf("%d    %f     %f\n",PWM,error,accError);

    if(error > 0){
        direction = 1; //cw
    }
    if(error < 0){
        direction = 0; //ccw
    }

    if(direction == 0){ //ccw direction
        Interrupt_disableInterrupt(INT_PORT5);
        Interrupt_disableInterrupt(INT_TA1_0);
        TA0CCR2 = 0; //disable cw pin
        TA0CCR3 = 0;
        /*int i = 0;
            while(i < 50){
                i++;
            }*/
        TA0CCR1 = PWM; //enable ccw pin
        TA0CCR4 = PWM;
        Interrupt_enableInterrupt(INT_PORT5);
        Interrupt_enableInterrupt(INT_TA1_0);
    }

    if (direction ==1){
        Interrupt_disableInterrupt(INT_PORT5);
        Interrupt_disableInterrupt(INT_TA1_0);
        TA0CCR1 = 0; //disable ccw pin
        TA0CCR4 = 0;
        /*int i = 0;
                    while(i < 50){
                        i++;
                    }*/
        TA0CCR2 = PWM; //enable cw pin
        TA0CCR3 = PWM;
        Interrupt_enableInterrupt(INT_PORT5);
        Interrupt_enableInterrupt(INT_TA1_0);
    }

    /*if (direction == 0){
        Interrupt_disableInterrupt(INT_PORT5);
        Interrupt_disableInterrupt(INT_TA1_0);
        TA0CCR2 = 0; //disable cw pin
        TA0CCR3 = 0;
        /*int i = 0;
            while(i < 50){
                i++;
            }*/
        /*TA0CCR1 = 0; //disable ccw pin
        TA0CCR4 = 0;
        Interrupt_enableInterrupt(INT_PORT5);
        Interrupt_enableInterrupt(INT_TA1_0);
    }*/

    /*if (direction == 1){
        Interrupt_disableInterrupt(INT_PORT5);
        Interrupt_disableInterrupt(INT_TA1_0);
        TA0CCR2 = 0; //disable cw pin
        TA0CCR3 = 0;
        /*int i = 0;
            while(i < 50){
                i++;
            }*/
        /*TA0CCR1 = 0; //disable ccw pin
        TA0CCR4 = 0;
        Interrupt_enableInterrupt(INT_PORT5);
        Interrupt_enableInterrupt(INT_TA1_0);
    }*/
    // Clear Timer A Interrupt Flag
    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
}





//UART interrupt - receives data and sends when enter key is pressed
void EUSCIA0_IRQHandler(void){
    Timer_A_disableInterrupt(TIMER_A1_BASE);

    //char buffer[16]; //char buffer

    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A0_BASE); //UART interrupt status

    UART_clearInterruptFlag(EUSCI_A0_BASE, status); //clears UART flag

    letter = UART_receiveData(EUSCI_A0_BASE); //specific letter received

    if(letter == 115){ //'s' check for string start
        string = 1; //sets check so buffer can be filled
    }


    if(string == 1){ //lets buffer fill once 's' is received
        buffer[strcount] = letter; //adds letter to char buffer

        strcount = strcount+1; //increases count of letters
    }


    if(strcount == 16) //once buffer is filled
    {
        /*char *string = buffer;
        char *e = strchr(string,' ');
        int delim = (int)(e-string);
        printf("%d\n",delim);*/
        char shoulderChar[7]; //string of shoulder angle
        char elbowChar[7]; //string of elbow angle
        strncpy(shoulderChar,buffer+1,7); //fills shoulderChar
        strncpy(elbowChar,buffer+8,7); //fills elbowChar
        shoulderAng = atoi(shoulderChar); //converts string to int
        elbowAng = atoi(elbowChar); //converts string to int
        //printf(buffer); //print check
        //printf("\n");
        /*printf(shoulderChar);
        printf("\n");
        printf(elbowChar);
        printf("\n");*/

        int i;
        for (i = 0; i < strcount; ++i) //loops through buffer
        {
            buffer[i] = '\0'; //empties buffer after it transmits the character
        }
        strcount = 0; //reset count
        string = 0; //reset string

        error = 0;
        accError = 0;

        Interrupt_disableInterrupt(INT_EUSCIA0); //disables UART
        Timer_A_enableInterrupt(TIMER_A1_BASE); //enables timer
    }

    GPIO_toggleOutputOnPin(GPIO_PORT_P1,GPIO_PIN0); //flashes LED for debug
}


//////////////// *** Timer A Interrupt Service Routine *** ///////////////////
void TA1_0_IRQHandler(void){
    Interrupt_enableInterrupt(INT_EUSCIA0); //enables UART
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
    //printf("1 sec\n");
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
        encDirection = 1; //cw
        count--; //decreases angle read
    }
    else if(Astat == 1 && Bstat==1 && prevA == 1 && prevB == 0){ //other possibility cw check
        encDirection = 1; //cw
        count--; //decreases angle read
    }
    else if(Astat == 0 && Bstat==1 && prevA == 1 && prevB == 1){ //ccw check
        encDirection = 0; //ccw
        count++; //increases angle read
    }
    else if(Astat == 1 && Bstat==1 && prevA == 0 && prevB == 1){ //other possibility ccw check
        encDirection = 0; //ccw
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

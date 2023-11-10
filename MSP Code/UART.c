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
volatile int count = 0; //current pos in buffer
volatile char string; //check to fill buffer
volatile int shoulderAng; //shoulder angle
volatile int elbowAng; //elbow angle


void main(void)
{
    CS_setDCOFrequency(3E+6);                       // Sets clock speed
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    //red led setup
    GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    ///// *** Configure Timer A and its interrupt *** /////
        Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig_1);
        Interrupt_enableInterrupt(INT_TA1_0);
        Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    Interrupt_disableMaster(); //step 1

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
                    GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION); /* Selecting P1.2 and P1.3 in UART mode */
        UART_initModule(EUSCI_A0_BASE,&UART_init); //Initialize UART Module 0

        UART_enableModule(EUSCI_A0_BASE); //Enable UART module

        UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT); //enable UART interrupts

        //Interrupt_enableInterrupt(INT_EUSCIA0); //step 6

        Interrupt_enableMaster(); //step 7

        while(1){}


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
    buffer[count] = letter; //adds letter to char buffer

    count = count+1; //increases count of letters
    }


    if(count == 16) //once buffer is filled
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
        printf(buffer); //print check
        printf("\n");
        /*printf(shoulderChar);
        printf("\n");
        printf(elbowChar);
        printf("\n");*/

        int i;
        for (i = 0; i < count; ++i) //loops through buffer
        {
            buffer[i] = '\0'; //empties buffer after it transmits the character
        }
        count = 0; //reset count
        string = 0; //reset string
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


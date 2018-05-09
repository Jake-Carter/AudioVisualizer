/*
 * periph_config.c
 *
 *  Created on: Apr 25, 2018
 *      Author: Andrew Teta
 */


#include "driverlib.h"
#include "periph_config.h"
#include "circBuf.h"

CircBuf_t temp_data;

void configureClocks(void) {
    /* Set to Vcore1 */
    PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set to use DCDC */
    PCM_setPowerState(PCM_AM_DCDC_VCORE1);

    /* Set wait state */
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_64 );
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_4 );
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    return;
}

void configureGPIO(void) {
    /* Terminating all remaining pins to minimize power consumption. This is
        done by register accesses for simplicity and to minimize branching API
        calls */
    //GPIO_setAsOutputPin(GPIO_PORT_P1, PIN_ALL16);
    GPIO_setAsOutputPin(GPIO_PORT_P2, PIN_ALL16);
    GPIO_setAsOutputPin(GPIO_PORT_PB, PIN_ALL16);
    GPIO_setAsOutputPin(GPIO_PORT_PC, PIN_ALL16);
    GPIO_setAsOutputPin(GPIO_PORT_PD, PIN_ALL16);
    GPIO_setAsOutputPin(GPIO_PORT_PE, PIN_ALL16);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, PIN_ALL16);
    GPIO_setOutputLowOnPin(GPIO_PORT_PB, PIN_ALL16);
    GPIO_setOutputLowOnPin(GPIO_PORT_PC, PIN_ALL16);
    GPIO_setOutputLowOnPin(GPIO_PORT_PD, PIN_ALL16);
    GPIO_setOutputLowOnPin(GPIO_PORT_PE, PIN_ALL16);

    /* Setting up GPIO pins as analog inputs (and references) */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,
            GPIO_PIN7 | GPIO_PIN6 | GPIO_PIN5 | GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);

    return;
}

void configureADC(void) {
/* Initializing ADC (MCLK/1/4) */
    MAP_ADC14_enableModule();
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_HSMCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_2,
            0);

    /* Configuring ADC Memory */
    MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);
    MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
    ADC_INPUT_A0, false);

    /* Configuring Sample Timer */
    MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    /* Enabling/Toggling Conversion */
    MAP_ADC14_enableConversion();
    MAP_ADC14_toggleConversionTrigger();

    /* Enabling interrupts */
    MAP_ADC14_enableInterrupt(ADC_INT0);
    MAP_Interrupt_enableInterrupt(INT_ADC14);
    MAP_Interrupt_enableMaster();
}

void configureUART(void) {

    // initialize circular buffer for storing temp data from ADC
    CB_initialize_buffer(&temp_data, 60);

    // P1.2, P1.3 correspond to Rx and Tx pins
    // configure pins to secondary function for UART config
    P1->SEL0 |= BIT2 | BIT3;
    P1->SEL1 &= ~BIT2 & ~BIT3;


    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST;         // put uart into reset state before config
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SSEL__SMCLK;   // select SMCLK as source
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_MODE_0;        // UART async mode

/****************************************************************/
// Configure UART Baud rate

#ifdef BAUD9600
    EUSCI_A0->MCTLW |= EUSCI_A_MCTLW_OS16;
    EUSCI_A0->BRW = 78;
    EUSCI_A0->MCTLW |= 2 << EUSCI_A_MCTLW_BRF_OFS;
    EUSCI_A0->MCTLW |= 0 << EUSCI_A_MCTLW_BRS_OFS;
#endif

#ifdef BAUD19200
    EUSCI_A0->MCTLW |= EUSCI_A_MCTLW_OS16;
    EUSCI_A0->BRW = 39;
    EUSCI_A0->MCTLW |= 1 + EUSCI_A_MCTLW_BRF_OFS;
    EUSCI_A0->MCTLW |= 0 + EUSCI_A_MCTLW_BRS_OFS;
#endif

#ifdef BAUD38400
    EUSCI_A0->MCTLW |= EUSCI_A_MCTLW_OS16;
    EUSCI_A0->BRW = 19;
    EUSCI_A0->MCTLW |= 8 << EUSCI_A_MCTLW_BRF_OFS;
    EUSCI_A0->MCTLW |= 0x65 << EUSCI_A_MCTLW_BRS_OFS;
#endif

#ifdef BAUD115200
    EUSCI_A0->MCTLW |= EUSCI_A_MCTLW_OS16;
    EUSCI_A0->BRW = 6;
    EUSCI_A0->MCTLW |= 8 << EUSCI_A_MCTLW_BRF_OFS;
    EUSCI_A0->MCTLW |= 0x20 << EUSCI_A_MCTLW_BRS_OFS;
#endif

    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST;          // take uart out of reset, done with config
    //return;
}

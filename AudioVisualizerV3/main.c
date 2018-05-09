/* --COPYRIGHT--,BSD
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//****************************************************************************
//
// main.c - MSP-EXP432P401R + Educational Boosterpack MkII - Microphone FFT
//
//          CMSIS DSP Software Library is used to perform 512-point FFT on
//          the audio samples collected with MSP432's ADC14 from the Education
//          Boosterpack's onboard microhpone. The resulting frequency bin data
//          is displayed on the BoosterPack's 128x128 LCD.
//
//****************************************************************************
//Jake Carter
//Audio Visualizer
//Modified the above example project to receive input from an auxiliary audio input
//and send fourier data over UART to a processing window that handles visualization

#include "driverlib.h"
//#include "msp.h"
#include "stdio.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "periph_config.h"
#include "circBuf.h"
#include "uart.h"

#define SAMPLE_LENGTH 512

/* ------------------------------------------------------------------
* Global variables for FFT Bin Example
* ------------------------------------------------------------------- */
uint32_t fftSize = SAMPLE_LENGTH;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
volatile arm_status status;

/* Statics */
static volatile uint16_t curADCResult;
static volatile float normalizedADCRes;

/* FFT data/processing buffers*/
float hann[SAMPLE_LENGTH];
int16_t adc_inputarray[SAMPLE_LENGTH];
int16_t data_array1[SAMPLE_LENGTH];
int16_t data_array2[SAMPLE_LENGTH];
int16_t data_input[SAMPLE_LENGTH*2];
int16_t data_output[SAMPLE_LENGTH];

int16_t samplePointer = 0;
bool sampleComplete = false;
uint8_t val[2];

/* SYSTICK variables */
uint32_t overflow = 0;
volatile uint32_t totalCycles = 0;

//volatile int switch_data = 0;

const eUSCI_UART_Config uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        6,                                       // BRDIV = 78
        8,                                       // UCxBRF = 2
        32,                                      // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};

void main(void)
{
    /* Halting the Watchdog  */
    MAP_WDT_A_holdTimer();

    /* Initializing Variables */
    curADCResult = 0;

    configureClocks();

    /* Enabling the FPU for floating point operation */
    MAP_FPU_enableModule();
    MAP_FPU_enableLazyStacking();

    //configureGPIO();

    /* Setting up GPIO pins as analog inputs (and references) */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,
            GPIO_PIN7 | GPIO_PIN6 | GPIO_PIN5 | GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
                GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    configureADC();

    //configure_uart();

    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_BASE);

    // Configure SysTick, module not started yet
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

    // Set SysTick period = 0x1000000000000 = 2^24
    SysTick->LOAD = 0xFFFFFF;

    // Enable global interrupt
//    __enable_irq();

    // Enable SysTick interrupt
//    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

//    /* Enabling interrupts */
//    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
//    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
//    MAP_Interrupt_enableSleepOnIsrExit();
//    MAP_Interrupt_enableMaster();

    // Initialize Hann Window
    int n;
    for (n = 0; n < SAMPLE_LENGTH; n++)
    {
        hann[n] = 0.5 - 0.5 * cosf((2*PI*n)/(SAMPLE_LENGTH-1));
    }

    while(1)
    {
        MAP_PCM_gotoLPM0();

        // Start SysTick Timer
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

        int i = 0;


        /* Computer real FFT using the completed data buffer */
        if (sampleComplete)
        {
            //Hann array * adc input...
            for (i=0; i<SAMPLE_LENGTH; i++)
            {
                data_array1[i] = (int16_t)(hann[i]*adc_inputarray[i]);
            }

            //Fourier transform init
            arm_rfft_instance_q15 instance;
            status = arm_rfft_init_q15(&instance, fftSize, ifftFlag, doBitReverse);

            arm_rfft_q15(&instance, data_array1, data_input);//Do the fourier transform -> data_input

            //Calculate magnitude of fft -> data_output
            for (i = 0; i < 2*SAMPLE_LENGTH; i+=2)
            {
                data_output[i/2] = (int32_t)(sqrtf((data_input[i] * data_input[i]) + (data_input[i+1] * data_input[i+1])));
            }

            q15_t maxValue;
            uint32_t maxIndex = 0;

            //Scale data...
            arm_max_q15(data_output, fftSize, &maxValue, &maxIndex);

            //Transmit data
            //Only transmit first half = positive fourier transforms to prevent wrap-around
            int c, i, sum;
            for (c = 0; c < SAMPLE_LENGTH / 2; c ++){
                if (!(c == 0 || c == 1 || c == 511)){
                    val[0] = (uint8_t)((data_output[c] & 0xFF00) >> 8);//most significant
                    val[1] = (uint8_t)(data_output[c] & 0x00FF);//least significant
                    MAP_UART_transmitData(EUSCI_A0_BASE, (uint8_t)(c));
                    MAP_UART_transmitData(EUSCI_A0_BASE, val[0]);//most first
                    MAP_UART_transmitData(EUSCI_A0_BASE, val[1]);//least next
                    MAP_UART_transmitData(EUSCI_A0_BASE, (uint8_t)0xFF);
                }
            }

            // Disable SysTick Timer
            SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

            // Calculate the # of cycles
            //totalCycles = (0x1000000 - SysTick->VAL) + 0x1000000 * overflow ;

            sampleComplete = 0;
        }
    }
}

void ADC14_IRQHandler(void)
{
    uint64_t status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);

    if (ADC_INT0 & status & !sampleComplete)//Interrupt pending and sample is not currently completed/being processed
    {
        curADCResult = MAP_ADC14_getResult(ADC_MEM0);//Get ADC result
        normalizedADCRes = (curADCResult * 3.3) / 16384;//Normalized result
        //Data buffer...
        if (samplePointer == SAMPLE_LENGTH - 1){
            //Buffer is about to be full
            //Wrap around and tell the main loop that the data is ready to be transformed...
            adc_inputarray[samplePointer] = curADCResult;
            samplePointer = 0;
            sampleComplete = true;
        } else {
            adc_inputarray[samplePointer++] = curADCResult;
        }
    }
}

void SysTick_Handler(void)
{
    /* This should never happen unless the function takes more > 2^24 cycles
     * to complete.
     */
    overflow++;                             // Increment # of systick timer overlows
}

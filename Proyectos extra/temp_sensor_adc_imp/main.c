//*****************************************************************************
//
// temperature_sensor.c - Example demonstrating the internal ADC temperature
// sensor.
//
// Copyright (c) 2010-2012 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 9453 of the Stellaris Firmware Development Package.
//
//*****************************************************************************

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"

//*****************************************************************************
//
//! \addtogroup adc_examples_list
//! <h1>ADC Temperature Sensor (temperature_sensor)</h1>
//!
//! This example shows how to setup ADC0 to read the internal temperature
//! sensor.
//!
//! NOTE: The internal temperature sensor is not calibrated.  This example
//! just takes the raw temperature sensor sample and converts it using the
//! equation found in the LM3S9B96 datasheet.
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - ADC0 peripheral
//!
//! The following UART signals are configured only for displaying console
//! messages for this example.  These are not required for operation of the
//! ADC.
//! - UART0 peripheral
//! - GPIO Port A peripheral (for UART0 pins)
//! - UART0RX - PA0
//! - UART0TX - PA1
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - None.
//
//*****************************************************************************

//*****************************************************************************
//
// This function sets up UART0 to be used for a console to display information
// as the example is running.
//
//*****************************************************************************
void
InitConsole(void)
{
    //
    // Enable GPIO port A which is used for UART0 pins.
    // TODO: change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Configure the pin muxing for UART0 functions on port A0 and A1.
    // This step is not necessary if your part does not support pin muxing.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    //
    // Select the alternate (UART) function for these pins.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioInit(0);
}

unsigned long ulADC0Value[1];
volatile float ulmV;
volatile float ulV;

//*****************************************************************************
//
// Configure ADC0 for the temperature sensor input with a single sample.  Once
// the sample is done, an interrupt flag will be set, and the data will be
// read then displayed on the console via UART0.
//
//*****************************************************************************
int
main(void)
{
    /*//
    // This array is used for storing the data read from the ADC FIFO. It
    // must be as large as the FIFO for the sequencer in use.  This example
    // uses sequence 3 which has a FIFO depth of 1.  If another sequence
    // was used with a deeper FIFO, then the array size must be changed.
    //
    unsigned long ulADC0_Value[1];

    //
    // These variables are used to store the temperature conversions for
    // Celsius and Fahrenheit.
    //
    unsigned long ulTemp_ValueC;
    unsigned long ulTemp_ValueF;*/

    //unsigned long ulADC0Value[4];
	//unsigned long ulADC0Value[1];
/*    volatile unsigned long ulTempAvg;
    volatile unsigned long ulTempValueC;
    volatile unsigned long ulTempValueF;
    volatile unsigned long inputValue;*/
    //volatile unsigned long ulmV;
    //volatile unsigned long ulV;

    //
    // Set the clocking to run at 20 MHz (200 MHz / 10) using the PLL.  When
    // using the ADC, you must either use the PLL or supply a 16 MHz clock
    // source.
    // TODO: The SYSCTL_XTAL_ value must be changed to match the value of the
    // crystal on your board.
    //
    //SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
    //               SYSCTL_XTAL_16MHZ);

    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

    //
    // Set up the serial console to use for displaying messages.  This is just
    // for this example program and is not needed for ADC operation.
    //
    InitConsole();

    //
    // Display the setup on the console.
    //
    UARTprintf("ADC ->\n");
    UARTprintf("  Type: Internal Temperature Sensor\n");
    //UARTprintf("  Samples: One\n");
    //UARTprintf("  Update Rate: 250ms\n");
    UARTprintf("  Input Pin: Internal temperature sensor\n\n");

    /*//
    // The ADC0 peripheral must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);*/

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    //GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_2);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlADCSpeedSet(SYSCTL_ADCSPEED_250KSPS);
    //ADCSequenceDisable(ADC0_BASE, 1);

    //
    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a singal to start the
    // conversion.  Each ADC module has 4 programmable sequences, sequence 0
    // to sequence 3.  This example is arbitrarily using sequence 3.
    //
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    //
    // Configure step 0 on sequence 3.  Sample the temperature sensor
    // (ADC_CTL_TS) and configure the interrupt flag (ADC_CTL_IE) to be set
    // when the sample is done.  Tell the ADC logic that this is the last
    // conversion on sequence 3 (ADC_CTL_END).  Sequence 3 has only one
    // programmable step.  Sequence 1 and 2 have 4 steps, and sequence 0 has
    // 8 programmable steps.  Since we are only doing a single conversion using
    // sequence 3 we will only configure step 0.  For more information on the
    // ADC sequences and steps, reference the datasheet.
    //
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                             ADC_CTL_END);

    //
    // Since sample sequence 3 is now configured, it must be enabled.
    //
    ADCSequenceEnable(ADC0_BASE, 3);

    //
    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    //
    ADCIntClear(ADC0_BASE, 3);

    /*ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 1);*/

    //
    // Sample the temperature sensor forever.  Display the value on the
    // console.
    //
    while(1)
    {
        /*//
        // Trigger the ADC conversion.
        //
        ADCProcessorTrigger(ADC0_BASE, 3);

        //
        // Wait for conversion to be completed.
        //
        while(!ADCIntStatus(ADC0_BASE, 3, false))
        {
        }

        //
        // Clear the ADC interrupt flag.
        //
        ADCIntClear(ADC0_BASE, 3);

        //
        // Read ADC Value.
        //
        ADCSequenceDataGet(ADC0_BASE, 3, ulADC0_Value);

        //
        // Use non-calibrated conversion provided in the data sheet.  Make
        // sure you divide last to avoid dropout.
        //
        ulTemp_ValueC = ((1475 * 1023) - (2250 * ulADC0_Value[0])) / 10230;

        //
        // Get fahrenheit value.  Make sure you divide last to avoid dropout.
        //
        ulTemp_ValueF = ((ulTemp_ValueC * 9) + 160) / 5;

        //
        // Display the temperature value on the console.
        //*/

    	//inputValue = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_2);
    	//UARTprintf("Input Value = %3d Volts\n", inputValue);

    	ADCProcessorTrigger(ADC0_BASE, 3);
    	while(!ADCIntStatus(ADC0_BASE, 3, false))
    	        {
    	        }
    	ADCIntClear(ADC0_BASE, 3);
    	ADCSequenceDataGet(ADC0_BASE, 3, ulADC0Value);

    	ulmV = ((ulADC0Value[0] * 3.3)/4095)*1000;
    	ulV = ulmV/1000;

    	char ulmVstr[100];
    	char ulVstr[100];

    	int ulmVEnt1 = ulmV;            // Get the integer part (678).
    	float ulmVFrac = ulmV - ulmVEnt1;     // Get fractional part (678.0123 - 678 = 0.0123).
    	int ulmVEnt2 = trunc(ulmVFrac * 10000);   // Turn into integer (123).
    	//int d2 = (int)(f2 * 10000); // Or this one: Turn into integer.

    	int ulVEnt1 = ulV;            // Get the integer part (678).
    	float ulVFrac2 = ulV - ulVEnt1;     // Get fractional part (678.0123 - 678 = 0.0123).
    	int ulVEnt2 = trunc(ulVFrac2 * 10000);   // Turn into integer (123).
    	float ulVFrac3 = (ulVFrac2 * 10000) - ulVEnt2;   // Get next fractional part (0.4567).
    	int ulVEnt3 = trunc(ulVFrac3 * 10000);   // Turn into integer (4567).

    	UARTprintf("origValue = %3d \n miliVolts = %d.%03d mV\n Volts = %d.%03d%03d V\n",ulADC0Value[0], ulmVEnt1, ulmVEnt2, ulVEnt1, ulVEnt2, ulVEnt3);

    	/*ADCIntClear(ADC0_BASE, 1);
    	ADCProcessorTrigger(ADC0_BASE, 1);
    	while(!ADCIntStatus(ADC0_BASE, 1, false))
    	{}
    	ADCSequenceDataGet(ADC0_BASE, 1, ulADC0Value);
    	ulTempAvg = (ulADC0Value[0] + ulADC0Value[1] + ulADC0Value[2] + ulADC0Value[3] + 2)/4;
    	ulTempValueC = (1475 - ((2475 * ulTempAvg)) / 4096)/10;
    	ulTempValueF = ((ulTempValueC * 9) + 160) / 5;*/

        //UARTprintf("Temperature = %3d*C or %3d*F\r\n ADC0_Value: %3d*ADCV\n Averange:Temperature: %3d*AT\n", ulTempValueC,
        //		ulTempValueF, ulADC0Value, ulTempAvg);

        //
        // This function provides a means of generating a constant length
        // delay.  The function delay (in cycles) = 3 * parameter.  Delay
        // 250ms arbitrarily.
        //
        SysCtlDelay(SysCtlClockGet() / 12);
    }
}

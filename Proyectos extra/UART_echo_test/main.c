#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

unsigned char dataFromUart[500];
int i;

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

void
InitUARTComm(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);

	GPIOPinConfigure(GPIO_PE0_U7RX);
	GPIOPinConfigure(GPIO_PE1_U7TX);
	GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	UARTConfigSetExpClk(UART7_BASE, SysCtlClockGet(), 115200, UART_CONFIG_WLEN_8|UART_CONFIG_PAR_NONE|UART_CONFIG_STOP_ONE);

	IntEnable(INT_UART7);
	UARTIntEnable(UART7_BASE, UART_INT_RX | UART_INT_RT);
	IntMasterEnable();
	//UARTEnable(UART0_BASE);
}

void blinkled(void)
{
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2);
	SysCtlDelay(SysCtlClockGet()/30);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
}

void sendDataUART(unsigned char *data){
    	while(UARTBusy(UART7_BASE));
  		while(*data != '\0')
  			//Coloca un string el cual es enviado caracter por caracter a traves del FIFO.
  			UARTCharPut(UART7_BASE, *data++);
  		blinkled();//Parpadea el LED para indicar la transmición
}

void
UARTIntHandler(void)
{
	unsigned long ulStatus;
	unsigned char *bufEcho = dataFromUart;

    // Obtiene el estado de la interrupción.
    ulStatus = UARTIntStatus(UART7_BASE, true);

    // Limpia la interrupción.
    UARTIntClear(UART7_BASE, ulStatus);

    // Loop while aqui estan los caracteres recibidos.
    while(UARTCharsAvail(UART7_BASE))
    {
        // Lee el caracter de la UART y lo almacena en la variable temp y esta en el arreglo recibe.
    	*bufEcho = UARTCharGetNonBlocking(UART7_BASE);
    	*bufEcho++;
        // Parpadea el led para indicar que un caracter se esta transfiriendo.
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        // retardo de 1 ms.
        SysCtlDelay(SysCtlClockGet() / (1000 * 3));
        // Apaga el LED.
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
    }

    sendDataUART(bufEcho);
    UARTprintf("PC say: %s\n", bufEcho);
}

int main(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_1);

	InitConsole();
	InitUARTComm();
	UARTprintf("UARTPrintf initialized!!!\n");

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
    SysCtlDelay(SysCtlClockGet() / (1000 * 3));
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

	unsigned char *buf = "Datas send from LM4F120 to PC\n\r";
	sendDataUART(buf);

	while(UARTBusy(UART7_BASE));

	buf = "Send me something please\n\r";
	sendDataUART(buf);

	UARTprintf("Program started succesfully, wait for response from PC\n");
    while (1) {
    	/*unsigned char *buf = "Envio de datos de LM4F120 a PC\n\r";
    	UARTprintf("Envio de datos de LM4F120 a PC\n\r");

    	sendDataUART(buf);

  		SysCtlDelay(SysCtlClockGet()/6);*///Espera por un while
    }
}

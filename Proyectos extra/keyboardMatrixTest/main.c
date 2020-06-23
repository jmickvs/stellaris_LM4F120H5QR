/*
 * main.c
 */
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#include "keypad.h"

//volatile unsigned int key_get;
unsigned long key_get;
unsigned char dataFromUart[4];

void InitConsole(void)
{
  //enable portA
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  //configure the pin multiplexing
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  //configure the type of the pins for uart tx/rx
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  //init the console
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
	//IntMasterEnable();
	//UARTEnable(UART0_BASE);
}

void initIntPortD(void){
	GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_FALLING_EDGE);
	IntEnable(INT_GPIOD);
	GPIOPinIntEnable(GPIO_PORTD_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
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
	int i = 0;
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

        i++;
    }
}

void GPIOIntHandler (void){
	//key_get = keypad_wait();
	//key_get = keypad_wait2();
	key_get = getKey();
	if(key_get != 0xff){
		UARTprintf("Key pressed: %x\n", key_get);
	}

	GPIOPinIntDisable(GPIO_PORTD_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	SysCtlDelay(SysCtlClockGet() / 3000);
	GPIOPinIntEnable(GPIO_PORTD_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
}

int main(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_1);

	InitConsole();
	UARTprintf("UARTPrintf initialized ...\n");
	InitUARTComm();
	UARTprintf("UART initialized ...\n");
	initPorts();
	UARTprintf("Port D initialized ...\n");
	initIntPortD();
	UARTprintf("Port D interrupt initialized ...\n");

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
	SysCtlDelay(SysCtlClockGet() / (1000 * 3));
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

	UARTprintf("Led indicator in LM4F120 configured ...\n");

	unsigned char *buf = "Data send from LM4F120 to PC\n\r";
	sendDataUART(buf);

	while(UARTBusy(UART7_BASE));

	UARTprintf("Program started succesfully\n");

	IntMasterEnable();

	while (1) {
	}
}

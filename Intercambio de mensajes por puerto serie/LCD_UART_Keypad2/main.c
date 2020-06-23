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
#include "driverlib/timer.h"
#include "utils/uartstdio.h"
#include "Lcd.h"
#include "keypad.h"

/*#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif*/

unsigned char dataFromUart[32];
int key_get;

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
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);

	GPIOPinConfigure(GPIO_PE4_U5RX);
	GPIOPinConfigure(GPIO_PE5_U5TX);
	GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);

	UARTConfigSetExpClk(UART5_BASE, SysCtlClockGet(), 115200, UART_CONFIG_WLEN_8|UART_CONFIG_PAR_NONE|UART_CONFIG_STOP_ONE);

	IntEnable(INT_UART5);
	UARTIntEnable(UART5_BASE, UART_INT_RX | UART_INT_RT);
	//IntMasterEnable();
	//UARTEnable(UART0_BASE);
}

void initIntPortE(void){
	GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_LOW_LEVEL);
	IntEnable(INT_GPIOE);
	GPIOPinIntEnable(GPIO_PORTE_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
}

void blinkled(void)
{
	/*The LED blinking once*/
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2);
	/*Sets delay 100ms*/
	SysCtlDelay(SysCtlClockGet()/75);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
}

void sendDataUART(unsigned char *data){
    	while(UARTBusy(UART5_BASE));
  		while(*data != '\0')
  			//Coloca un string el cual es enviado caracter por caracter a traves del FIFO.
  			UARTCharPut(UART5_BASE, *data++);
  		blinkled();//Parpadea el LED para indicar la transmición
}

void
UARTIntHandler(void)
{
	unsigned long ulStatus;
	unsigned char *recibe = dataFromUart;

    // Obtiene el estado de la interrupción.
    ulStatus = UARTIntStatus(UART5_BASE, true);

    // Limpia la interrupción.
    UARTIntClear(UART5_BASE, ulStatus);

    // Loop while aqui estan los caracteres recibidos.
    while(UARTCharsAvail(UART5_BASE))
    {
        // Lee el caracter de la UART y lo almacena en la variable temp y esta en el arreglo recibe.
    	*recibe = UARTCharGetNonBlocking(UART5_BASE);
    	*recibe++;
        // Parpadea el led para indicar que un caracter se esta transfiriendo.
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        // retardo de 1 ms.
        SysCtlDelay(SysCtlClockGet() / 150);
        // Apaga el LED.
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
    }
    // Limpia pantalla del LCD y escribe el contenido del arreglo recibe.
    Lcd_borrar();
    Lcd_Puts(recibe);
}

void GPIOIntHandler (void){
	unsigned long ulStatus;
	unsigned char *key_char;

	ulStatus = GPIOPinIntStatus(GPIO_PORTE_BASE, true);
	GPIOPinIntClear(GPIO_PORTE_BASE, ulStatus);

	key_get = getKey();
	if(key_get != 0xff){
		switch(key_get) {

		   case 0  :
			  key_char = "0";
		      break; /* optional */

		   case 1  :
			  key_char = "1";
		      break; /* optional */

		   case 2  :
			  key_char = "2";
		      break; /* optional */

		   case 3  :
			  key_char = "3";
		      break; /* optional */

		   case 4  :
			  key_char = "4";
		      break; /* optional */

		   case 5  :
			  key_char = "5";
		      break; /* optional */

		   case 6  :
			  key_char = "6";
		      break; /* optional */

		   case 7  :
			  key_char = "7";
		      break; /* optional */

		   case 8  :
			   key_char = "8";
		      break; /* optional */

		   case 9  :
			   key_char = "9";
		      break; /* optional */

		   case 10  :
			  key_char = "10";
		      break; /* optional */

		   case 11 :
			  key_char = "11";
		      break; /* optional */

		   case 12  :
			  key_char = "12";
		      break; /* optional */

		   case 13  :
			  key_char = "13";
		      break; /* optional */

		   case 14  :
			  key_char = "14";
		      break; /* optional */

		   case 15  :
			  key_char = "15";
		      break; /* optional */
		}

	    Lcd_borrar();
	    Lcd_Puts(key_char);
		UARTprintf("Key pressed: %x\n", key_get);
	}

	//GPIOPinIntDisable(GPIO_PORTE_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	/*Sets delay 100 ms*/
	/*SysCtlDelay(SysCtlClockGet() / 30);
	GPIOPinIntEnable(GPIO_PORTE_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);*/
}

int main(void) {
	/* El relojdel sistema esta corriendo utilizando un oscilador de 16 Mhz conectado al oscilador principal del microcontrolador
	 * Este genera una señal de reloj interna de 400Mhz utilizando un PLL
	 * La señal es preescalada por el sistema por 2
	 * En la sigiente configuracion se esta preescalando el reloj del sistema por 5 para que el oscilador corra a 40 Mhz
	 * La frecuencia del reloj del sistema debe ser menor o igual a 80 Mhz
	 */
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_1);

	InitConsole();
	UARTprintf("UARTPrintf initialized ...\n");
	InitUARTComm();
	UARTprintf("UART initialized ...\n");
	initPorts();
	UARTprintf("Port E initialized ...\n");
	initIntPortE();
	UARTprintf("Port E interrupt initialized ...\n");
	Lcd_init();
	UARTprintf("LCD initialized ...\n");

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
	/*Sets delay 50ms*/
	SysCtlDelay(SysCtlClockGet() / 150);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

	IntMasterEnable();

	UARTprintf("Led indicator in LM4F120 configured ...\n");
	UARTprintf("Program started succesfully\n");

	unsigned char *buf = "Data send from LM4F120 to PC\n\r";
	sendDataUART(buf);
	while(UARTBusy(UART5_BASE));
	buf = "Type something and you send it for show it in the LCD, also you can press some button from keypad 4x4 and see result in the LCD.";
	sendDataUART(buf);

  while (1) {
  }
}

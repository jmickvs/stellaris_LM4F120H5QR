#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

// Número de bytes para enviar y recibir.
#define NUM_SSI_DATA 3

// Variables globales a utilizar en el sistema.
unsigned long ulDataTx[NUM_SSI_DATA];
unsigned long ulDataRx[NUM_SSI_DATA];
unsigned long ulindex1;
int ulindex2;

// Configuración de la consola serial.
void InitConsole(void)
{
	// Habilita el puerto A
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	// Configura los pines de lectura y escritura de la consola serial.
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	// Configura el tipo de los pines para la UART tx/rx.
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	// Inicializa la consola
	UARTStdioInit(0);
}

// Configuración del módulo UART para la comunicación serial.
void
InitUARTComm(void)
{
	// Habilita el puerto E.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	//Habilita el módulo UART7.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);

	// Configura los pines de lectura y escritura para la comunicación serial.
	GPIOPinConfigure(GPIO_PE0_U7RX);
	GPIOPinConfigure(GPIO_PE1_U7TX);

	// Configura el tipo de los pines para la UART tx/rx.
	GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Establece la configuración de la comunicación serial.
	// baud rate: 115200.
	// data size: 8 bytes.
	// Parity: none.
	// Un bit de paro.
	UARTConfigSetExpClk(UART7_BASE, SysCtlClockGet(), 115200, UART_CONFIG_WLEN_8|UART_CONFIG_PAR_NONE|UART_CONFIG_STOP_ONE);

	// Habilita interrupción del módulo UART7.
	IntEnable(INT_UART7);
	// Habilita y configura la interrupción para el módulo UART7.
	// UART_INT_RX: Interrupción de recepción.
	// UART_INT_RT: Interrupción de tiempo de espera de la recepción.
	UARTIntEnable(UART7_BASE, UART_INT_RX | UART_INT_RT);
}

// Configuración del módulo SSI para la comunicación SPI.
void initSSI(void){
	// Se habilita el módulo SSI2.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);

	// Se habilita el puerto B en el cual se encuentran los pines MOSI, SIMO, CLK Y FSS para el módulo SSI2.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	// Se configuran los pines 4, 6 y 7 del puerto B, el pin 5 no se está usando en esta aplicación.
	GPIOPinConfigure(GPIO_PB4_SSI2CLK);
	//GPIOPinConfigure(GPIO_PB5_SSI2FSS);
	GPIOPinConfigure(GPIO_PB6_SSI2RX);
	GPIOPinConfigure(GPIO_PB7_SSI2TX);

	// Configura el tipo de los pines 4, 6 y 7.
	GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_4);

	// Configura el puerto SSI para funcionar maestro SPI.
	// Establece la frecuencia de salida del CLK de 1.2 MHz.
	// Tamaño de dato a transmitir de 8 bytes.
	// Establece polaridad 1 y fase 1.

	SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_2,
			SSI_MODE_MASTER, 1200000, 8);

	// Habilita el módulo SSI2.
	SSIEnable(SSI2_BASE);

	// Habilita la interrupción para el módulo SSI2.
	IntEnable(INT_SSI2);
	// Habilita y configura la interrupción para el módulo SSI2.
	SSIIntEnable(SSI2_BASE, SSI_RXFF | SSI_RXTO | SSI_RXOR);
}

// Función para encender y apagar el led rojo.
void blinkled(void)
{
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2);
	SysCtlDelay(SysCtlClockGet()/30);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
}

// Función para enviar un cadena de caracteres a través del módulo UART.
void sendDataUART(unsigned char *data){
	while(UARTBusy(UART7_BASE));
	while(*data != '\0')
		//Coloca un string el cual es enviado caracter por caracter a traves del FIFO.
		UARTCharPut(UART7_BASE, *data++);
	blinkled();//Parpadea el LED para indicar la transmisión
}

void sendDataSSI (unsigned long data2ssi, int size){

	// Espera si el bus está ocupado.
	while(SSIDataGetNonBlocking(SSI2_BASE, &ulDataRx[0]))
	{
	}

	SSIDataPut(SSI2_BASE, data2ssi*2);

	UARTprintf("Dato enviado:\n  ");
	UARTprintf("'%c' \n ", data2ssi);
	UARTprintf("Data transfer complete!!! \n");
}

// Función para recibir datos a través del módulo UART cuando se genera un interrupción.
void
UARTIntHandler(void)
{

	int i = 0;
	unsigned long ulStatus;
	unsigned long data2Send;

	// Obtiene el estado de la interrupción.
	ulStatus = UARTIntStatus(UART7_BASE, true);

	// Limpia la interrupción.
	UARTIntClear(UART7_BASE, ulStatus);

	// Loop while aqui estan los caracteres recibidos.
	while(UARTCharsAvail(UART7_BASE))
	{
		// Lee el carácter de la UART y lo almacena en la variable temp y esta en el arreglo recibe.
		//*bufEcho = UARTCharGetNonBlocking(UART7_BASE);
		//*bufEcho++;
		data2Send = UARTCharGetNonBlocking(UART7_BASE);
		// Parpadea el led para indicar que un carácter se está transmitiendo.
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
		// retardo de 1 ms.
		SysCtlDelay(SysCtlClockGet() / (1000 * 3));
		// Apaga el LED.
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

		i++;
	}

	UARTprintf("Message recived from uart: %c\n", data2Send);
	sendDataSSI(data2Send, 0);
}

// Función para recibir los datos a través del módulo SSI2 cuando se genera un interrupción.
void ssirxhandler(){

	// Limpiar la interrupción.
	SSIIntClear(SSI2_BASE, SSI_RXFF | SSI_RXTO | SSI_RXOR);

	// Obtiene el dato enviado desde el esclavo
	SSIDataGet(SSI2_BASE, &ulDataRx[ulindex2]);

    //
    // Since we are using 8-bit data, mask off the MSB.
    //
    ulDataRx[ulindex2] &= 0x00FF;

	UARTprintf("Recibido:\n  ");
	UARTprintf("'%c' \n", ulDataRx[ulindex2]);

	UARTprintf("Data receive complete!!!! \n");
}

int
main(void)
	{
	ulindex1 = 0;
	ulindex2 = 0;

	// Configura el reloj para que sea ejecutado directamente desde el cristal/oscilador externo, el cual tiene una frecuencia de 16 MHz.
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
			SYSCTL_XTAL_16MHZ);

	// Inicializa el puerto y pines de salida para usar el led RGB de la tarjeta.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_1);

	// Configura la consola serial para desplegar mensajes de informacion sobre la ejecución del sistema.
	InitConsole();
	UARTprintf("UARTPrintf inicializada.\n");

	// Configura el módulo UART para la comunicación serial.
	InitUARTComm();
	UARTprintf("Módulo UART inicializado.\n");

	// Inicializa el módulo SSI.
	initSSI();
	UARTprintf("Módulo SSI inicializado.\n");

	// Habilita el interruptor maestro.
	IntMasterEnable();

	// Inicializa los pines 1 y 2 del led RGB de la tarjeta.
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

	// Enciende y apaga el led Azul como indicador de que el sistema se ha iniciado correctamente.
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
	SysCtlDelay(SysCtlClockGet() / (1000 * 3));
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
	UARTprintf("Led indicador en la tarjeta LM4F120 configurado.\n");

	// Display the setup on the console.
	UARTprintf("  SSI ->\n");
	UARTprintf("  Modo: SPI\n");
	UARTprintf("  Dato: 8-bit\n\n");

	UARTprintf("Sistema iniciado con éxito.\n");

	// Inicializa el dato enviar por SPI.
	/*ulDataTx[0] = (unsigned long)'s';
	ulDataTx[1] = (unsigned long)'p';
	ulDataTx[2] = (unsigned long)'i';*/

	sendDataSSI(0, 0);

	while (1){
		// Envía los caracteres uno a uno de la variable ulDataTx.
		/*if (ulindex1 < 3) {
			// Espera si el bus está ocupado.
			while(SSIDataGetNonBlocking(SSI2_BASE, &ulDataRx[0]))
			{
			}

			UARTprintf("Enviado:\n  ");
			UARTprintf("'%c' \n ", ulDataTx[ulindex1]);

			// Inserta el dato al bus de salida.
			SSIDataPut(SSI2_BASE, ulDataTx[ulindex1]);

			ulindex1 ++;

			if(ulindex1 == 3){
				UARTprintf("Data transfer complete!!! \n");
			}
		}*/
	}
}

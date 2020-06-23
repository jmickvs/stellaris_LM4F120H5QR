#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"

// Direcci�n del esclavo.
#define SLAVE_ADDRESS 0x48

unsigned char tx;
unsigned char rx;
unsigned char dataFromUart[4];
int i;

// Configuraci�n de la consola serial.
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

// Configuraci�n del m�dulo UART para la comunicaci�n serial.
void
InitUARTComm(void)
{
	// Habilita el puerto E.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	//Habilita el m�dulo UART7.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);

	// Configura los pines de lectura y escritura para la comunicaci�n serial.
	GPIOPinConfigure(GPIO_PE0_U7RX);
	GPIOPinConfigure(GPIO_PE1_U7TX);

	// Configura el tipo de los pines para la UART tx/rx.
	GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Establece la configuraci�n de la comunicaci�n serial.
	// baud rate: 115200.
	// data size: 8 bytes.
	// Parity: none.
	// Un bit de paro.
	UARTConfigSetExpClk(UART7_BASE, SysCtlClockGet(), 115200, UART_CONFIG_WLEN_8|UART_CONFIG_PAR_NONE|UART_CONFIG_STOP_ONE);

	// Habilita interrupci�n del m�dulo UART7.
	IntEnable(INT_UART7);
	// Habilita y configura la interrupci�n para el m�dulo UART7.
	// UART_INT_RX: Interrupci�n de recepci�n.
	// UART_INT_RT: Interrupcion de tiempo de espera de la recepci�n.
	UARTIntEnable(UART7_BASE, UART_INT_RX | UART_INT_RT);
}

// Configura el m�dulo i2c2.
void initI2C(void)
{
	// Habilita el m�dulo I2C2.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);
	// Habilita el puerto E.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	// Configura los pines como i2c.
	GPIOPinTypeI2CSCL(GPIO_PORTE_BASE, GPIO_PIN_4);
	GPIOPinTypeI2C(GPIO_PORTE_BASE, GPIO_PIN_5);
	GPIOPinConfigure(GPIO_PE4_I2C2SCL);
	GPIOPinConfigure(GPIO_PE5_I2C2SDA);

	// Inicializa el reloj a 100 KHz.
	I2CMasterInitExpClk(I2C2_MASTER_BASE, SysCtlClockGet(), false);//false para usar el modo 100kHz.
	// Habilita i2c como maestro.
	I2CMasterEnable(I2C2_MASTER_BASE);

	// Habilita la interrupci�n del m�dulo I2C2
	IntEnable(INT_I2C2);
	// Configura la interrupci�n para el maestro.
	I2CMasterIntEnable(I2C2_MASTER_BASE);
}

// Funci�n para transmitir datos del maestro al esclavo.
void I2C_write(unsigned char slave_address, unsigned char *data, unsigned long length)
{
	volatile int i = 0;

	// Inicia la transmisi�n i2c.
	I2CMasterSlaveAddrSet(I2C2_MASTER_BASE, slave_address, false);
	I2CMasterDataPut(I2C2_MASTER_BASE, *(data));
	while(I2CMasterBusy(I2C2_MASTER_BASE)) {} // Check, the bus isn't busy (low?)

	// Se configura para la transmisi�n de m�ltiples bytes.
	I2CMasterControl(I2C2_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);

	// Espera mientras el bus est� ocupado.
	while(I2CMasterBusy(I2C2_MASTER_BASE)) {}

	// Hace la transmisi�n de los datos.
	for(i=1; i < length; i++) {
		I2CMasterDataPut(I2C2_MASTER_BASE, *(data+i));
		I2CMasterControl(I2C2_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
		while(I2CMasterBusy(I2C2_MASTER_BASE)) {}
	}

	// Termina la transmisi�n de datos.
	I2CMasterControl(I2C2_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(I2C2_MASTER_BASE)) {}
}

// Funci�n para encender y apagar el led rojo.
void blinkled(void)
{
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2);
	SysCtlDelay(SysCtlClockGet()/30);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
}

// Funci�n para enviar un cadena de caracteres a trav�s del m�dulo UART.
void sendDataUART(unsigned char *data){
	while(UARTBusy(UART7_BASE));
	while(*data != '\0')
		//Coloca un string el cual es enviado caracter por caracter a traves del FIFO.
		UARTCharPut(UART7_BASE, *data++);
	blinkled();//Parpadea el LED para indicar la transmisi�n
}

void
UARTIntHandler(void)
{
	// Deshabilita la interrupci�n i2c del Maestro.
	I2CMasterIntDisable(I2C2_MASTER_BASE);

	int i = 0;
	unsigned long ulStatus;
	unsigned char *bufEcho = dataFromUart;

	// Obtiene el estado de la interrupci�n.
	ulStatus = UARTIntStatus(UART7_BASE, true);

	// Limpia la interrupci�n.
	UARTIntClear(UART7_BASE, ulStatus);

	// Loop while aqui estan los caracteres recibidos.
	while(UARTCharsAvail(UART7_BASE))
	{
		// Lee el car�cter de la UART y lo almacena en la variable temp y esta en el arreglo recibe.
		*bufEcho = UARTCharGetNonBlocking(UART7_BASE);
		*bufEcho++;
		// Parpadea el led para indicar que un car�cter se est� transfiriendo.
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
		// retardo de 1 ms.
		SysCtlDelay(SysCtlClockGet() / (1000 * 3));
		// Apaga el LED.
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

		i++;
	}

	//sendDataUART(bufEcho);
	// Env�a los los datos a trav�s de i2c desde el maestro al esclavo.
	I2C_write(SLAVE_ADDRESS, dataFromUart, i);

	// Habilita la interrupci�n  i2c del maestro.
	I2CMasterIntEnable(I2C2_MASTER_BASE);

	UARTprintf("Message sent: %s\n", dataFromUart);
}

void I2C_read(unsigned char slave_address, unsigned char *dataRx, unsigned long length)
{
	volatile int i = 0;

	// Configura la direcci�n del esclavo desde el que espera recibir un dato.
	I2CMasterSlaveAddrSet(I2C2_MASTER_BASE, slave_address, true); // set to true since it's a read

	// Configura la recepci�n de m�ltiples bytes desde el esclavo.
	I2CMasterControl(I2C2_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
	while(I2CMasterBusy(I2C2_MASTER_BASE)) {}

	// Obtiene los datos enviados por el esclavo.
	*dataRx = I2CMasterDataGet(I2C2_MASTER_BASE);

	for(i = 1; i < (length-2); i--)	{
		I2CMasterControl(I2C2_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
		while(I2CMasterBusy(I2C2_MASTER_BASE)){}
		// get further bytes
		*dataRx = I2CMasterDataGet(I2C2_MASTER_BASE);
	}

	// Termina la recepci�n de datos enviados por el esclavo.
	I2CMasterControl(I2C2_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
	while(I2CMasterBusBusy(I2C2_MASTER_BASE)) {}

	*dataRx = I2CMasterDataGet(I2C2_MASTER_BASE);

	if(rx == '1'){
		UARTprintf("Dato recibido desde el esclavo: %c", dataRx);
	}
}

void i2c_rx_handler (void) {
	// Limpia la interrupci�n del maestro.
	I2CMasterIntClear(I2C2_MASTER_BASE);
	// Lee el bus para recibir datos enviados por el esclavo.
	I2C_read(SLAVE_ADDRESS, &rx, 1);
}

int main(void){

	// Configura el reloj para que sea ejecutado directamente desde el cristal/oscilador externo, el cual tiene una frecuencia de 16 MHz.
	SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	// Inicializa el puerto y pines de salida para usar el led RGB de la tarjeta.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_1);

	// Configura la consola serial para desplegar mensajes de informaci�n sobre la ejecuci�n del sistema.
	InitConsole();
	UARTprintf("UARTPrintf inicializada.\n");
	// Configura el m�dulo UART para la comunicaci�n serial.
	InitUARTComm();
	UARTprintf("M�dulo UART inicializado.\n");
	// Inicializa el modulo SSI.
	initI2C();
	UARTprintf("M�dulo i2c inicializado.\n");

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

	unsigned char *buf = "Dato enviado desde LM4F120 a la PC\n\r";
	sendDataUART(buf);

	// Espera mientras el bus est� ocupado.
	while(UARTBusy(UART7_BASE));

	buf = "Debe escribir \"on\" para encender el led rojo de la tarjeta MSP430 y \"off\" para apagarlo\n";
	sendDataUART(buf);

	UARTprintf("Sistema iniciado con �xito.\n");

	while(1)
	{

	}
}

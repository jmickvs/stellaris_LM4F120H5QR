#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "utils/uartstdio.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"

#define RED 0

unsigned long ulADC0Value[1];
volatile float ulmV;
volatile float ulV;

unsigned long ulPeriod, ulDutyR, ulDutyExt;
unsigned int uiOn = RED;
unsigned char charGet;
int uartEnable_ = 0;
int UARTFire = 0;
float dutyValue;

/*#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif*/

void InitConsole(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	UARTStdioInit(0);
}

void
InitUARTComm(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

	GPIOPinConfigure(GPIO_PC4_U1RX);
	GPIOPinConfigure(GPIO_PC5_U1TX);
	GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200, UART_CONFIG_WLEN_8|UART_CONFIG_PAR_NONE|UART_CONFIG_STOP_ONE);

	IntEnable(INT_UART1);
	UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
}

void initPWMTimer(void){
	//
	// Habilita los periféricos puerto B y el timer 2 respectivamente.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	// Timer2
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

	//
	// Configura el PIN 0 del puerto B como salida y se inicializa
	//
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0,0x00);

	//
	// Se le asigna un valor de configuración al PIN 0 del puerto B, el cual indica que trabajara con el PWM.
	//
	GPIOPinConfigure(GPIO_PB0_T2CCP0); // B
	//
	// Se configura el pin a usar por el periférico del timer 2.
	//
	GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_0);

	//
	// Configura el modo de operación del timer 2
	// - TIMER_CFG_SPLIT_PAIR: Dos timers de media onda cada uno.
	// - TIMER_CFG_A_PWM: Salida PWM de media onda.
	//
	TimerConfigure(TIMER2_BASE,TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM);

	//
	// Se define una configuración del periodo de 1ms.
	//
	ulPeriod = (SysCtlClockGet() / 500)/2;

	//
	// Se define un tiempo de trabajo inicial de 50%
	//
	ulDutyExt = (unsigned long)(ulPeriod-1)*0.5;

	//
	// Configura el valor del periodo de la señal de salida.
	//
	TimerLoadSet(TIMER2_BASE,TIMER_A,ulPeriod-1);
	//
	// Configura el valor inicial del ciclo de trabajo para la onda de salida
	//
	TimerMatchSet(TIMER2_BASE,TIMER_A,ulDutyExt);
	//
	// Habilita la operación del timer 2.
	//
	TimerEnable(TIMER2_BASE,TIMER_A);
}

void initADCModule (void){
    //
    // Se habilita el módulo ADC.
    //
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    //
    // Se configura una tasa de muestreo de 250 Kilo-Muestras por segundo (Kilo-samples per second).
    // Otras opciones para la tasa de muestreo son: 125KSPS, 500KSPS and 1MSPS.
    //
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_250KSPS);

	//
	// Se habilita el periférico E para usar uno de sus pines como entrada analogica.
	// Usando el método GPIOPinTypeADC se especifica el paquete de pines a usar como entrada/s analogica, en este caso se usa el PIN3.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

	//
	// Se configura la secuencia de muestras como:
	// 		3: captura una muestra simple.
	//		ADC_TRIGGER_PROCESSOR: al ejecutar ADCProcessorTrigger() se inicia la conversión del dato/s capturado.
	// 		0: Se establece una prioridad alta con respecto a otras secuencias de muestras configuradas en caso de que las hubiera.
	//
	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

	//
	// Se configura un canal de muestra 0 (ADC_CTL_CH0) en modo de una sola terminación o single-ended (default)
	// y configura la bandera de interrupcion (ADC_CTL_IE) para ser agregada cuando la muestra esté completa.
	// Con ADC_CTL_END la lógica del ADC sabe que esta es la ultima conversión en secuencia 3.
	//
	ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

    //
    // Se habilita la secuencia de muestras 3.
    //
	ADCSequenceEnable(ADC0_BASE, 3);

    //
    // Se limpia el estado de la bandera de interrupción para asegurar que la bandera de la interrupción esté limpia antes de empezar el muestreo.
    //
	ADCIntClear(ADC0_BASE, 3);
}

void blinkled(void)
{
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
	SysCtlDelay(SysCtlClockGet()/75);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
}

void sendDataUART(unsigned char *data){
    	while(UARTBusy(UART1_BASE));
  		while(*data != '\0')
  			//Coloca un string el cual es enviado caracter por caracter a traves del FIFO.
  			UARTCharPut(UART1_BASE, *data++);
  		blinkled();//Parpadea el LED para indicar la transmisión
}

void
UARTIntHandler(void)
{
	unsigned long ulStatus;
	int i = 0;
	int j = 0;
    char c = 0;
    int x = 0;
	int dutyValueTemp = 0;
	unsigned char uartDuty[3] = {'\0'};
	UARTFire ++;

    // Obtiene el estado de la interrupción.
    ulStatus = UARTIntStatus(UART1_BASE, true);

    // Limpia la interrupción.
    UARTIntClear(UART1_BASE, ulStatus);

    // Loop while donde obtiene los caracteres recibidos.
    while(UARTCharsAvail(UART1_BASE))
    {
        // Lee el carácter de la UART y lo almacena en la variable charGet y está en el arreglo uartDuty.
    	charGet = UARTCharGetNonBlocking(UART1_BASE);
    	uartDuty[i]= charGet;
        i++;

        blinkled();
    }

    // Convierte el valor almacenado en uartDuty a un valor entero.
    for(j = i; j > 0; j--){
    	if(j == 3){
    		c = uartDuty[i-j];
    		x = (c - '0')*100;
    	}
    	if(j == 2){
    		c = uartDuty[i-j];
    		x = (c - '0')*10;
    	}
    	if(j == 1){
    		c = uartDuty[i-j];
    		x = c - '0';
    	}

    	dutyValueTemp += x;
    }

    // El valor obtenido desde la UART se convierte a un valor porcentual entre 0 y 1 para definir el ciclo de trabajo para el PWM.
   	dutyValue = (99 - dutyValueTemp) * 0.01;

    uartEnable_ = 1;

    // Se obtiene el valor del ciclo de trabajo para el PWM.
    ulDutyExt = (unsigned long)(ulPeriod-1)*dutyValue;

    // Se valida la variable UARTFire para el ciclo de trabajo sea configurado una vez.
    // esto debido a que la configuración actual de la interrupción provoca que la funcion se dispare dos veces.
    if(UARTFire == 1){
    	// Se establece en ciclo de trabajo actual para el PWM.
    	TimerMatchSet(TIMER2_BASE,TIMER_A,ulDutyExt);
    }else if(UARTFire == 2){
    	UARTFire = 0;
    }
}

void getDataAdc ()
{
	int dutyValueTemp = 0;
	int dutyValuePrint = 0;

	// dispara el inicio para obtener la muestra.
	ADCProcessorTrigger(ADC0_BASE, 3);

	// Obtiene el estado de la interrupción.
   	while(!ADCIntStatus(ADC0_BASE, 3, false))
    {
    }

   	// Limpia la interrupción del ADC.
   	ADCIntClear(ADC0_BASE, 3);

   	// Obtiene el dato capturado por la secuencia de muestreo y la almacena en la variable ulADC0Value.
   	ADCSequenceDataGet(ADC0_BASE, 3, ulADC0Value);

   	// Obtiene valor de entrada dados en mV
   	ulmV = ((ulADC0Value[0] * 3.3)/4095)*1000;
   	// Obtiene el valor de entrada dados en V
   	ulV = ulmV/1000;

  	int ulmVEnt1 = ulmV;            // Get the integer part (678).
   	float ulmVFrac = ulmV - ulmVEnt1;     // Get fractional part (678.0123 - 678 = 0.0123).
   	int ulmVEnt2 = ulmVFrac * 10000; // Turn into integer (123).
   	//int ulmVEnt2 = trunc(ulmVFrac * 10000);   // Turn into integer (123).

   	int ulVEnt1 = ulV;            // Get the integer part (678).
   	float ulVFrac2 = ulV - ulVEnt1;     // Get fractional part (678.0123 - 678 = 0.0123).
   	//int ulVEnt2 = trunc(ulVFrac2 * 10000);   // Turn into integer (123).
   	int ulVEnt2 = ulVFrac2 * 10000;   // Turn into integer (123).
  	float ulVFrac3 = (ulVFrac2 * 10000) - ulVEnt2;   // Get next fractional part (0.4567).
   	//int ulVEnt3 = trunc(ulVFrac3 * 10000);   // Turn into integer (4567).
   	int ulVEnt3 = ulVFrac3 * 10000;   // Turn into integer (4567).

   	dutyValuePrint = ((100 * ulADC0Value[0])/4096);

   	// El valor obtenido desde la el ADC se convierte a un valor porcentual entre 0 y 1 para definir el ciclo de trabajo para el PWM.
   	dutyValueTemp = 99 - ((100 * ulADC0Value[0])/4096);
   	dutyValue = dutyValueTemp * 0.01;

   	// Se obtiene el valor del ciclo de trabajo para el PWM.
   	ulDutyExt = (unsigned long)(ulPeriod-1)*dutyValue;

   	// Se establece en ciclo de trabajo actual para el PWM.
	TimerMatchSet(TIMER2_BASE,TIMER_A,ulDutyExt);

   	UARTprintf("origValue = %3d \n miliVolts = %d.%03d mV\n Volts = %d.%03d%03d V\n Duty = %3d %%\n",ulADC0Value[0], ulmVEnt1, ulmVEnt2, ulVEnt1, ulVEnt2, ulVEnt3, dutyValuePrint);
}

int main(void) {

	// Configuración de reloj a 40MHz
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

	// Inicializa el puerto y pines de salida para usar el led RGB de la tarjeta.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_1);

	// Configura la consola serial para desplegar mensajes de información sobre la ejecución del sistema.
	InitConsole();
	UARTprintf("UARTPrintf inicializada.\n");
	// Configura el módulo UART para la comunicación serial.
	InitUARTComm();
	UARTprintf("Módulo UART inicializado.\n");
	// Configura el PWM.
	initPWMTimer();
	UARTprintf("PWM inicializado.");
	// Configura el módulo ADC.
	initADCModule();
	UARTprintf("ADC incializado.");

	UARTprintf("Prueba de PWM y ADC ->\n");
	UARTprintf("  Tipo: Pwm controlado por una entrada adc\n");
	UARTprintf("  Entrada: Senal analogica de 3.3 V\n\n");

	// Habilita el interruptor maestro.
	IntMasterEnable();

	// Inicializa los pines 1 y 2 del led RGB de la tarjeta.
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

	// Enciende y apaga el led Azul como indicador de que el sistema se ha iniciado correctamente.
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
	SysCtlDelay(SysCtlClockGet() / 150);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
	UARTprintf("Led indicador en la tarjeta LM4F120 configurado.\n");

	UARTprintf("Sistema iniciado con éxito.\n");

	unsigned char *buf = "Dato enviado desde LM4F120 a la PC\n\r";
    sendDataUART(buf);
    while(UARTBusy(UART1_BASE));
    buf = "Envie un numero del 0 al 100 para establecer el ciclo de trabajo del PWM\n";
    sendDataUART(buf);

	while(1)
	{
		if(uartEnable_ == 1)
		{
			UARTprintf("Ciclo de trabajo establecido desde la PC. \n");
		}
		else
		{
			getDataAdc();
		}
		SysCtlDelay(SysCtlClockGet() / 12);
	}
}

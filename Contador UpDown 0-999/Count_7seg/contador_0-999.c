/*
 * Stellaris LAUNCHPAD LM4F120H5QR
 * EL siguiente codigo sirve para un contador el cual se desplegara en displays de 7 segmentos anodo comun
 * Utilizando un decodificador BCD SN74LS47N y un boton externo
 * Las salidas son entradas de los habilitadores de los displays de 7 segmentos
 * (PB1-PRIMER DISPLAY, PB2-SEGUNDO DISPLAY, PB3-TERCER DISPLAY)
 * y el decodificador BCD SN74LS47N (PD0-A,PD1-B,PD2-C,PD3-D)
 * Las entradas son el boton externo (PB0) y los PUSH1, PUSH2 (PF4, PF0) de la tarjeta
 * En este trabajo se utilizaron las interrupciones externas, los Timer0 y Timer1 y entrada y salida de perifericos
 */

#include "utils/ustdlib.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/hibernate.h"
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include <inc/hw_gpio.h>

#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

volatile unsigned int cont = 0;
volatile unsigned int unidad = 0;
volatile unsigned int decena = 0;
volatile unsigned int centena = 0;

//volatile unsigned int miliS = 0;
//volatile unsigned int seg = 0;

volatile unsigned int band = 0;
volatile unsigned int band1 = 0;
volatile unsigned int band3 = 0;


int main(void)
{
	unsigned long ulPeriod;
	unsigned long ulPeriod2;

	/* El relojdel sistema esta corriendo utilizando un oscilador de 16 Mhz conectado al oscilador principal del microcontrolador
	 * Este genera una señal de reloj interna de 400Mhz utilizando un PLL
	 * La señal es preescalada por el sistema por 2
	 * En la sigiente configuracion se esta preescalando el reloj del sistema por 5 para que el oscilador corra a 40 Mhz
	 * La frecuencia del reloj del sistema debe ser menor o igual a 80 Mhz
	 */
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	// Se habilitan los perifericos de los puertos F, B y D
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

	// Se configuran los puertos como entrada
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0);

	// Se configuran los puertos de salida
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	// Se libera el boton PUSH2 para que trabaje como un periferico normal de entrada
	HWREG(GPIO_PORTF_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
	HWREG(GPIO_PORTF_BASE+GPIO_O_CR) = 0x01;

	// Se configuran los botones PUSH1,PUSH2 y boton externo
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);

	// Se habilitan los perifericos del Timer0 y Timer1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

	// Se configuran los Timer0 y Timer1 para que trabajen con 32 bits por periodo
	TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);
	TimerConfigure(TIMER1_BASE, TIMER_CFG_32_BIT_PER);

	// La variable ulPeriod toma un valor del reloj del sistema
	ulPeriod = (SysCtlClockGet() / 1) / 1;
	ulPeriod2 = (SysCtlClockGet() / 150) / 1;

	// Se establece el tiempo cada cuanto se activaran las interrupciones

	// Timer0 se activara cada 1 segundo
	TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod -1);
	// Timer1 se activara cada 10 milisegundos
	TimerLoadSet(TIMER1_BASE, TIMER_A, ulPeriod2 -1);

	//Habilita las interrupciones del sistema
	IntEnable(INT_TIMER0A);
	IntEnable(INT_TIMER1A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	// Habilita el control maestro de interrupciones
	IntMasterEnable();

	while(1)
	{
		// Si el boton PUSH1 de la tarjeta es presionado entonces ingresa a la siguiente instrucción
		if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0){
			// El led rgb de la tarjeta enciende de color rojo como señal de que se presiono el boton de aumento
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2);
			// Se incian las interrupciones del Timer0 y el Timer1
			TimerEnable(TIMER0_BASE, TIMER_A);
			TimerEnable(TIMER1_BASE, TIMER_A);
			band = 1;
		}
		// Si el boton PUSH2 de la tarjeta es presionado entonces ingresa a la siguiente instrucción
		else if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) == 0){
			// El led rgb de la tarjeta enciende de color azul como señal de que se presiono el boton de disminución
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);
			// Se incian las interrupciones del Timer0 y el Timer1
			TimerEnable(TIMER0_BASE, TIMER_A);
			TimerEnable(TIMER1_BASE, TIMER_A);
			band = 2;
		}
		// Si el boton externo es presionado entonces ingresa a la siguiente instrucción
		else if(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0) == 0){
			// El led rgb de la tarjeta enciende de color verde como señal de que se presiono el boton de disminución
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 8);
			// Se incian las interrupciones del Timer0 y el Timer1
			TimerEnable(TIMER0_BASE, TIMER_A);
			TimerEnable(TIMER1_BASE, TIMER_A);
			band = 3;
			// La bandera band3 cambia de valor dependiendo de los puslsos que reciba y el tiempo
			// que se mantenga presionado el boton externo (1 pulso = inicio, 2 pulsos = pausa y presionado por 3 seg = reinicio)
			band3++;
		}
	}
}

// Interrupcion para aumentar, disminuir, iniciar/pausar/reinicio el conteo
void Timer0IntHandler(void)
{
	// Limpia la interrupcion del Timer0
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	// Lee el estado actual del los pines GPIO y los regresa al estado inicial
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);

	// Si la bandera band = 1 entonces el contador aumenta 1 unidad
	if(band == 1){
		if(cont != 999){
			if(unidad == 9)
			{
				if(decena == 9){
					centena++;
					decena = 0;
				}
				else{
					decena++;
					unidad = 0;
				}
				unidad = 0;
			}
			else
			{
				unidad++;
			}
			cont++;
			}
		else{
			cont = 0;
			unidad = 0;
			decena = 0;
			centena = 0;
			}
		band = 0;
	}
	// Si la bandera band = 2 entonces el contador dismunuye 1 unidad
	else if(band == 2){
		if(cont!= 0){
			if(unidad != 0)
			{
				unidad --;
			}
			else
			{
				if(decena != 0){
					decena --;
				}
				else{
					centena--;
					decena = 9;
				}
				unidad = 9;
			}
			cont--;
			}
			else{
			unidad = 9;
			decena = 9;
			centena = 9;
			cont = 999;
			}
		band = 0;
	}
	// Si la bandera band = 3 entonces el contador inicia/pausa/reinicio
	else if(band == 3){
		// Si la band3 es menor a 70000 y menor a 140000 entonce el contador se pausa (dos pulsos del boton externo)
		if(band3 > 70000 && band3 < 140000){
			TimerDisable(TIMER0_BASE, TIMER_A);
			band3 = 0;
		}
		// Si la bandera ban3 es mayor a band3 entonces el sistema reinicializa sus contadores y banderas a cero
		// (mantener presionado el boton esterno durante 3 segundos aproximadamente)
		else if(band3 > 600000){
			cont = 0;
			unidad = 0;
			decena = 0;
			centena= 0;
			band = 0;
			band1 = 0;
			band3 = 0;
		}
		// Si no se cumplen ni una de las condiciones anteriores el contador incia el conteo
		else{
			band = 0;
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);

				if(cont != 999){
				if(unidad == 9)
				{
					if(decena == 9){
						GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);
						centena++;
						decena = 0;
						unidad = 0;
					}
					else{
						GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 4);
						decena++;
						unidad = 0;
					}
				}
				else
				{
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);
					unidad++;
				}
				cont++;
				}
				else{
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
					cont = 0;
					unidad = 0;
					decena = 0;
					centena = 0;
				}
				band = 3;
			}
	}
	else
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
}

// Interrupcion para mostrar los datos y habilitar los displays de 7 segmentos
void Timer1IntHandler(void)
{
	// Limpia la interrupcion del Timer1
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
/*	if(miliS == 99){
		seg++;
		miliS = 0;
	}

	else{
		miliS++;
	}*/
	if(band1 == 0){
		// Se inicializa la bandera que controla los habilitadores de los displays de 7 segmentos
		band1 = 1;
	}
	else if(band1 == 1){
		// Habilita el primer display de 7 segmentos
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);
		// Los pines del PIND toman el valor de la variable unidad y es mostrado en el display de 7 segmentos
		GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, unidad);
		// Cambia el valor de la bandera para habilitar el segundo display de 7 segmentos
		band1 = 2;
	}
	else if(band1 == 2){
		// Habilita el segundo display de 7 segmentos
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 4);
		// Los pines del PIND toman el valor de la variable decena y es mostrado en el display de 7 segmentos
		GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, decena);
		// Cambia el valor de la bandera para habilitar el tercer display de 7 segmentos
		band1 = 3;
	}
	else{
		// Habilita el tercer display de 7 segmentos
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);
		// Los pines del PIND toman el valor de la variable centena y es mostrado en el display de 7 segmentos
		GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, centena);
		// regresa al valor inicial a la bandera band1
		band1 = 0;
	}
}





/*
* PWM example
*/

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"

#define RED 0
#define BLUE 1
#define GREEN 2

int main(void)
{
	unsigned long ulPeriod, ulDutyR, ulDutyG, ulDutyB;
	unsigned int uiOn = RED;

	// Set the clock to 40 MHz
	SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_5);

	// Enable clocks for peripherals
	// GPIOF > LED
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// Timer0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	// Timer1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

	// Configure GPIO
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,0x00);
	GPIOPinConfigure(GPIO_PF1_T0CCP1); // R
	GPIOPinConfigure(GPIO_PF2_T1CCP0); // B
	GPIOPinConfigure(GPIO_PF3_T1CCP1); // G
	GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

	// Congiure timer0
	ulPeriod = 1000;

	// Red LED > PF1 > T0CCP1
	TimerConfigure(TIMER0_BASE,TIMER_CFG_SPLIT_PAIR|TIMER_CFG_B_PWM);
	ulDutyR = 0;
	TimerLoadSet(TIMER0_BASE,TIMER_B,ulPeriod - 1);
	TimerMatchSet(TIMER0_BASE,TIMER_B,ulDutyR);
	TimerEnable(TIMER0_BASE,TIMER_B);

	TimerConfigure(TIMER1_BASE,TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM);
	// Blue LED > PF2 > T1CCP0
	ulDutyB = 0;
	TimerLoadSet(TIMER1_BASE,TIMER_A,ulPeriod-1);
	TimerMatchSet(TIMER1_BASE,TIMER_A,ulDutyB);
	TimerEnable(TIMER1_BASE,TIMER_A);

	// Green LED > PF3 > T1CCP1
	ulDutyG = 0;
	TimerLoadSet(TIMER1_BASE,TIMER_B,ulPeriod-1);
	TimerMatchSet(TIMER1_BASE,TIMER_B,ulDutyG);
	TimerEnable(TIMER1_BASE,TIMER_B);

	while(1)
	{
		if(uiOn == RED)
		{
			TimerMatchSet(TIMER0_BASE,TIMER_B,ulDutyR++);
			if(ulDutyR == ulPeriod - 1)
			{
				ulDutyR = 900;
				TimerMatchSet(TIMER0_BASE,TIMER_B,ulDutyR);
				ulDutyB = 0;
				uiOn = BLUE;
			}
		}else if(uiOn == BLUE)
		{
			TimerMatchSet(TIMER1_BASE,TIMER_A,ulDutyB++);
			if(ulDutyB == ulPeriod - 1)
			{
				ulDutyB = 900;
				TimerMatchSet(TIMER1_BASE,TIMER_A,ulDutyB);
				ulDutyG = 0;
				uiOn = GREEN;
			}
		}else if(uiOn == GREEN)
		{
			TimerMatchSet(TIMER1_BASE,TIMER_B,ulDutyG++);
			if(ulDutyG == ulPeriod - 1)
			{
				ulDutyG = 900;
				TimerMatchSet(TIMER1_BASE,TIMER_B,ulDutyG);
				ulDutyR= 0;
				uiOn = RED;
			}
		}

		SysCtlDelay(10000);
	}
}

/*
 * keypad.h
 *
 *  Created on: 28/02/2016
 *      Author: Juan Miguel Vargas Sánchez
 */
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

#ifndef KEYPAD_H_
#define KEYPAD_H_

#define ROWPORT GPIO_PORTC_BASE
#define COLPORT GPIO_PORTE_BASE
#define ROWPORTENABLE SYSCTL_PERIPH_GPIOC
#define COLPORTENABLE SYSCTL_PERIPH_GPIOE
#define ROW1 GPIO_PIN_4
#define ROW2 GPIO_PIN_5
#define ROW3 GPIO_PIN_6
#define ROW4 GPIO_PIN_7
#define COL1 GPIO_PIN_0
#define COL2 GPIO_PIN_1
#define COL3 GPIO_PIN_2
#define COL4 GPIO_PIN_3
#define OUTP1 0xE0
#define OUTP2 0xD0
#define OUTP3 0xB0
#define OUTP4 0x70
#define INP1 0x0E
#define INP2 0x0D
#define INP3 0x0B
#define INP4 0x07

extern unsigned long getKey(void);
extern void initPorts(void);

#endif /* KEYPAD_H_ */

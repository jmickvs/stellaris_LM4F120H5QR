/*
 * keypad.c
 *
 *  Created on: 28/02/2016
 *      Author: Juan Miguel Vargas Sánchez
 */

#include "keypad.h"

void initPorts (void){
	// Habilitación de preifericos y configuracion de los puertos C, D como salida y entrada respectivamente
	SysCtlPeripheralEnable(ROWPORTENABLE);
	SysCtlPeripheralEnable(COLPORTENABLE);
	GPIOPinTypeGPIOOutput(ROWPORT, ROW1 | ROW2 | ROW3 | ROW4);
	GPIOPinTypeGPIOInput(COLPORT, COL1 | COL2 | COL3 | COL4);
}

unsigned long getKey(void) {
	unsigned int secuence[4] = {OUTP1, OUTP2, OUTP3, OUTP4};
	int i, row, data;

	row = 0;

	for(i = 0; i < 4; i++){
		GPIOPinWrite(ROWPORT, ROW1 | ROW2 | ROW3 | ROW4, secuence[i]);
		asm(" nop");
		data = GPIOPinRead(COLPORT, COL1 | COL2 | COL3 | COL4);

		if(data != 0xFF){
			SysCtlDelay(SysCtlClockGet()/25);
			switch(data){
			case INP1:
				return row;
			case INP2:
				return row + 1;
			case INP3:
				return row + 2;
			case INP4:
				return row + 3;
			}
		}
		row += 4;
	}

	return 0xFF;
}

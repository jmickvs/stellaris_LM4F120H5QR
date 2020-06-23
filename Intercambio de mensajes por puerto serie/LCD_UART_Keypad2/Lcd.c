/*
 * Stellaris LaunchPad Lcd.c
 *
 * 16x2 Lcd
 *
 * Juan Miguel Vargas Sánchez
 */
 
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "Lcd.h"
 
void Lcd_init() {
 
		// Habilitación y configuraciín del puerto B como salida
        SysCtlPeripheralEnable(LCDPORTENABLE);
        GPIOPinTypeGPIOOutput(LCDPORT, 0xFF);
 
        SysCtlDelay(50000);
 
        // Inicia proceso de inicialiazacion del LCD
        GPIOPinWrite(LCDPORT, RS,  0x00 );
 
        GPIOPinWrite(LCDPORT, D4 | D5 | D6 | D7,  0x30 );
        GPIOPinWrite(LCDPORT, E, 0x02);
        SysCtlDelay(10);
        GPIOPinWrite(LCDPORT, E, 0x00);
 
        SysCtlDelay(50000);
 
        GPIOPinWrite(LCDPORT, D4 | D5 | D6 | D7,  0x30 );
        GPIOPinWrite(LCDPORT, E, 0x02);
        SysCtlDelay(10);
        GPIOPinWrite(LCDPORT, E, 0x00);
 
        SysCtlDelay(50000);
 
        GPIOPinWrite(LCDPORT, D4 | D5 | D6 | D7,  0x30 );
        GPIOPinWrite(LCDPORT, E, 0x02);
        SysCtlDelay(10);
        GPIOPinWrite(LCDPORT, E, 0x00);
 
        SysCtlDelay(50000);
 
        GPIOPinWrite(LCDPORT, D4 | D5 | D6 | D7,  0x20 );
        GPIOPinWrite(LCDPORT, E, 0x02);
        SysCtlDelay(10);
        GPIOPinWrite(LCDPORT, E, 0x00);
 
        SysCtlDelay(50000);
 
        Lcd_comando(0x28);
        Lcd_comando(0xC0);
        Lcd_comando(0x06);
        Lcd_comando(0x80);
        Lcd_comando(0x28);
        Lcd_comando(0x0f);
        Lcd_borrar();
 
}
void Lcd_comando(unsigned char c) {
 
        GPIOPinWrite(LCDPORT, D4 | D5 | D6 | D7, (c & 0xf0) );
        GPIOPinWrite(LCDPORT, RS, 0x00);
        GPIOPinWrite(LCDPORT, E, 0x02);
        SysCtlDelay(50000);
        GPIOPinWrite(LCDPORT, E, 0x00);
 
        SysCtlDelay(50000);
 
        GPIOPinWrite(LCDPORT, D4 | D5 | D6 | D7, (c & 0x0f) << 4 );
        GPIOPinWrite(LCDPORT, RS, 0x00);
        GPIOPinWrite(LCDPORT, E, 0x02);
        SysCtlDelay(10);
        GPIOPinWrite(LCDPORT, E, 0x00);
 
        SysCtlDelay(50000);
 
}
 
void Lcd_Putch(unsigned char d) {
 
        GPIOPinWrite(LCDPORT, D4 | D5 | D6 | D7, (d & 0xf0) );
        GPIOPinWrite(LCDPORT, RS, 0x01);
        GPIOPinWrite(LCDPORT, E, 0x02);
        SysCtlDelay(10);
        GPIOPinWrite(LCDPORT, E, 0x00);
 
        SysCtlDelay(50000);
 
        GPIOPinWrite(LCDPORT, D4 | D5 | D6 | D7, (d & 0x0f) << 4 );
        GPIOPinWrite(LCDPORT, RS, 0x01);
        GPIOPinWrite(LCDPORT, E, 0x02);
        SysCtlDelay(10);
        GPIOPinWrite(LCDPORT, E, 0x00);
 
        SysCtlDelay(50000);
 
}
void Lcd_Goto(char x, char y){
 
        if(x==1)
                Lcd_comando(0x80+((y-1)%16));
        else
                Lcd_comando(0xC0+((y-1)%16));
}
 
void Lcd_borrar(void){
        Lcd_comando(0x01);
        SysCtlDelay(10);
}
 
void Lcd_Puts(unsigned char* s){
 
        while(*s)
                Lcd_Putch(*s++);
}

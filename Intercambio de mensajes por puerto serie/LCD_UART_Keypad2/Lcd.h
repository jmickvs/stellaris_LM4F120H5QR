/*
 * Stellaris LaunchPad 
 *
 * 16x2 Lcd
 *  
 * Juan Miguel Vargas Sánchez
 */
 
#ifndef LCD_H_
#define LCD_H_
 
#define LCDPORT                  GPIO_PORTB_BASE
#define LCDPORTENABLE    SYSCTL_PERIPH_GPIOB
#define RS                               GPIO_PIN_0
#define E                                GPIO_PIN_1
#define D7                               GPIO_PIN_4
#define D6                               GPIO_PIN_5
#define D5                               GPIO_PIN_6
#define D4                               GPIO_PIN_7
 
void Lcd_comando(unsigned char);  //comando LCD
void Lcd_borrar(void);                 //Limpia el Lcd
void Lcd_Puts(unsigned char*);                   //Despliega datos en el LCD
void Lcd_Goto(char,char);               //Mueve el cursor a las cordenadas que se le indiquen
void Lcd_init(void);                    //Inicializa el LCD
void Lcd_Putch(unsigned char);  //Envia caracter a caracter al LCD
 
#endif /* LCD_H_ */

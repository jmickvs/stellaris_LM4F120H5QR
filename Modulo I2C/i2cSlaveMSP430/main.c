//******************************************************************************
//  MSP430G2xx3 Demo - USCI_B0 I2C Slave RX multiple bytes from MSP430 Master
//
//  Description: This demo connects two MSP430's via the I2C bus. The master
//  transmits to the slave. This is the slave code. The interrupt driven
//  data receiption is demonstrated using the USCIB0 RX interrupt.
//  ACLK = n/a, MCLK = SMCLK = default DCO = ~1.2MHz
//
//                                /|\  /|\
//               MSP430G2xx3      10k  10k     MSP430G2xx3
//                   slave         |    |        master
//             -----------------   |    |  -----------------
//           -|XIN  P3.1/UCB0SDA|<-|---+->|P3.1/UCB0SDA  XIN|-
//            |                 |  |      |                 |
//           -|XOUT             |  |      |             XOUT|-
//            |     P3.2/UCB0SCL|<-+----->|P3.2/UCB0SCL     |
//            |                 |         |                 |
//
//  D. Dang
//  Texas Instruments Inc.
//  February 2011
//   Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
#include "msp430g2553.h"

void receive_cb(unsigned char receive);
void transmit_cb();
void start_cb();

unsigned char flag=0;
unsigned int flag1=0;

unsigned char PRxData[3];                     // Pointer to RX data
unsigned char RXByteCtr;
volatile unsigned char RxBuffer[128];       // Allocate 128 byte of RAM

unsigned char TXData = 0;
unsigned char TXByteCtr;

unsigned char data;

unsigned char ok_success[2] = {'0','k'};


void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P1DIR |= BIT0;
  //P1OUT = 0;

  P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
  P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0

  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
  UCB0I2COA = 0x48;                         // Own Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  //IE2 |= UCB0RXIE;                          // Enable RX interrupt
  UCB0I2CIE |= UCSTPIE + UCSTTIE;           // Enable STT and STP interrupt
  IE2 |= UCB0RXIE + UCB0TXIE;              //Enable RX and TX interrupt
  //UCB0I2CIE |= UCSTTIE;           // Enable STT and STP interrupt
  //UCB0I2CIE |= UCSTPIE + UCSTTIE;           // Enable STT and STP interrupt

  TXData = 0x00;                            // Holds TX data

  _EINT();
  BCSCTL1 = CALBC1_16MHZ;
  DCOCTL = CALDCO_16MHZ;
  LPM4;                                       // read out the RxData buffer
}

void start_cb(){
	PRxData[0] = 0;
	flag1 = 0;
}

void receive_cb(unsigned char receive){
	PRxData[flag1] = receive;
    flag1++;

    if(strncmp(PRxData, "on", 2) == 0){
    	//P1OUT = ~P1OUT;
    	P1OUT = 1;
    	//flag1 = 0;
    	data = '1';
    	//IFG2 &= ~UCB0RXIFG;
    	//UCB0STAT &= ~UCSTTIFG;
    	//transmit_cb();
    }else if (strncmp(PRxData, "off", 3) == 0) {
    	//P1OUT = ~P1OUT;
    	P1OUT = 0;
    	//flag1 = 0;
    	data = '1';
    	//IFG2 &= ~UCB0RXIFG;
    	//UCB0STAT &= ~UCSTTIFG;
    	//transmit_cb();
    }

    /*if(flag1 >= 3){
    	flag1 = 0;
    }*/
}

void transmit_cb(){

	//UCB0TXBUF = TXData++;
	//flag++;
	if (data == '1') {

		UCB0TXBUF = data;
		data = '0';
	}else{
		//UCB0TXBUF = TXData++;
		UCB0TXBUF = data;
		flag++;
	}
}

// USCI_B0 Data ISR
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  if (IFG2 & UCB0TXIFG){
	  transmit_cb();
  }
  else{
	  //UCB0CTL1 |= UCTXSTP;
	  receive_cb(UCB0RXBUF);
  }
}

// USCI_B0 State ISR
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
    //UCB0STAT &= ~UCSTTIFG;                    // Clear start condition int flag
	UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);
	start_cb();
}

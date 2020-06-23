//******************************************************************************
//   MSP430G2xx3 Demo - USCI_A0, SPI 3-Wire Slave Data Echo
//
//   Description: SPI slave talks to SPI master using 3-wire mode. Data received
//   from master is echoed back.  USCI RX ISR is used to handle communication,
//   CPU normally in LPM4.  Prior to initial data exchange, master pulses
//   slaves RST for complete reset.
//   ACLK = n/a, MCLK = SMCLK = DCO ~1.2MHz
//
//   Use with SPI Master Incremented Data code example.  If the slave is in
//   debug mode, the reset signal from the master will conflict with slave's
//   JTAG; to work around, use IAR's "Release JTAG on Go" on slave device.  If
//   breakpoints are set in slave RX ISR, master must stopped also to avoid
//   overrunning slave RXBUF.
//
//                MSP430G2xx3
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          | |             XOUT|-
// Master---+-|RST              |
//            |             P1.2|<- Data Out (UCA0SOMI)
//            |                 |
//            |             P1.1|-> Data In (UCA0SIMO)
//            |                 |
//            |             P1.4|<- Serial Clock In (UCA0CLK)
//
//   D. Dang
//   Texas Instruments Inc.
//   February 2011
//   Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
#include "msp430g2553.h"
#include <string.h>

unsigned long cmdbuf[3];
unsigned int cmd_index=0;
char respbuff[2];

/** Delay function. **/
void delay(unsigned int d) {
  int i;
  for (i = 0; i<d; i++) {
    //nop();
  }
}

void flash_spi_detected(void) {
    int i=0;
    P1OUT = 0;
    for (i=0; i < 6; ++i) {
        P1OUT = ~P1OUT;
        delay(0x4fff);
        delay(0x4fff);
    }
}

void main(void)
{
	respbuff[0] = 'o';
	respbuff[1] = 'k';

	  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
	  /* led */
	  //P1DIR |= BIT0 + BIT5;
	  P1DIR |= BIT0;
	  while (P1IN & BIT4);                   // If clock sig from mstr stays low,
	                                            // it is not yet in SPI mode

	  flash_spi_detected();                 // Blink 3 times

	  P1SEL = BIT1 + BIT2 + BIT4;
	  P1SEL2 = BIT1 + BIT2 + BIT4;
	  UCA0CTL1 = UCSWRST;                       // **Put state machine in reset**
	  UCA0CTL0 |= UCMSB + UCMODE_2 + UCSYNC;               // 3-pin, 8-bit SPI slave
	  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

	  IE2 |= UCA0RXIE;                          // Enable USCI0 RX interrupt

	  __bis_SR_register(LPM4_bits + GIE);       // Enter LPM4, enable interrupts

	  while(1){

	  }
}

__attribute__((interrupt(USCIAB0RX_VECTOR))) void USCI0RX_ISR (void)
{

	unsigned long dataReceived;

  if (cmd_index == 3) {//(strncmp(cmdbuf, "spi", 3) == 0) {
    P1OUT |= BIT0;
    cmd_index = 0;
  } else {
    P1OUT &= ~BIT0;

    cmdbuf[cmd_index] = UCA0RXBUF;
    dataReceived = UCA0RXBUF;

    UCA0TXBUF = dataReceived;
    cmd_index++;
  }
}

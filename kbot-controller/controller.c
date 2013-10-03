#include <stdint.h>
#include <msp430.h>

#include "msprf24.h"
#include "nrf_userconfig.h"

#define PROTO_TOKEN    0x4b
#define PROTO_STOP     0
#define PROTO_FORWARD  1
#define PROTO_BACKWARD 2
#define PROTO_RIGHT    4
#define PROTO_LEFT     8

#define PROTO_PACKET_SIZE 4

volatile unsigned int user;

unsigned char pressed_buttons;

int poll_buttons(void)
{
	pressed_buttons = PROTO_STOP;
	if((P1IN & BIT7))
		pressed_buttons |= PROTO_FORWARD;
#if 1
	if((P2IN & BIT5))
		pressed_buttons |= PROTO_BACKWARD;
	if((P2IN & BIT4))
		pressed_buttons |= PROTO_LEFT;
	if((P2IN & BIT3))
		pressed_buttons |= PROTO_RIGHT;
#endif
	if(pressed_buttons)
		return 1;
	return 0;
}

int main(void)
{
	unsigned char seq_num = 0;
	char buf[PROTO_PACKET_SIZE];
	char addr0[5] = {0xaa, 0xbb, 0xcc, 0xdd, 0x00};
	//char addr1[5] = {0xde, 0xee, 0xde, 0xee, 0x11};
	//char addr2[5] = {0xde, 0xee, 0xde, 0xee, 0x22};
	//char addr3[5] = {0xde, 0xee, 0xde, 0xee, 0x33};
	//char addr4[5] = {0xde, 0xee, 0xde, 0xee, 0x44};
	//char addr5[5] = {0xde, 0xee, 0xde, 0xee, 0x55};

	WDTCTL = WDTHOLD | WDTPW;
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 = CALBC1_16MHZ;
	BCSCTL2 = DIVS_1;  // SMCLK = DCOCLK/2
	// SPI (USCI) uses SMCLK, prefer SMCLK < 10MHz (SPI speed limit for nRF24 = 10MHz)

	// Red, Green LED used for status
	P1DIR |= (BIT0 + BIT6);
	P1OUT &= ~(BIT0 + BIT6);

	user = 0xFE;

	// Initial values for nRF24L01+ library config variables 
	rf_crc = RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
	rf_addr_width      = 5;
	rf_speed_power     = RF24_SPEED_1MBPS | RF24_POWER_0DBM;
	rf_channel         = 120;

	msprf24_init();  // All RX pipes closed by default
	msprf24_set_pipe_packetsize(0, PROTO_PACKET_SIZE);
	msprf24_open_pipe(0, 1);  // Open pipe#0 with Enhanced ShockBurst enabled for receiving Auto-ACKs

	msprf24_standby();
	user = msprf24_current_state();

	w_tx_addr(addr0);
	w_rx_addr(0, addr0);  // Pipe 0 receives auto-ack's, autoacks are sent back to the TX addr so the PTX node
			             // needs to listen to the TX addr on pipe#0 to receive them.

	buf[0] = PROTO_TOKEN;

	while(1) {
		__delay_cycles(DELAY_CYCLES_5MS);
		P1OUT &= ~BIT0; // Red LED off
		P1OUT &= ~BIT6; // Green LED off

		if(!poll_buttons())
			continue;

		buf[1] = seq_num++;
		buf[2] = pressed_buttons;
		buf[3] = 0;

		w_tx_payload(PROTO_PACKET_SIZE, buf);
		msprf24_activate_tx();
		LPM4;

		if (rf_irq & RF24_IRQ_FLAGGED) {
			rf_irq &= ~RF24_IRQ_FLAGGED;

			msprf24_get_irq_reason();
			if (rf_irq & RF24_IRQ_TX){
				P1OUT &= ~BIT0; // Red LED off
				P1OUT |= BIT6;  // Green LED on
			}
			if (rf_irq & RF24_IRQ_TXFAILED){
				P1OUT &= ~BIT6; // Green LED off
				P1OUT |= BIT0;  // Red LED on
			}

			msprf24_irq_clear(rf_irq);
			user = msprf24_get_last_retransmits();
		}
	}
}

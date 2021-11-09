#include <atmel_start.h>
#include "addio/Embedded/IO/Atmel Start/usb_cdc_stdio/usb_cdc_stdio.h"

//Add 1 character for string termination.
uint8_t rx_buffer[USB_CDC_RX_BUF_SIZE+1];
uint8_t tx_buffer[USB_CDC_TX_BUF_SIZE+1];

bool dataReceived;

void data_rx_callback(const uint16_t length);

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	//Initialize and redirect.
	cdc_stdio_init();
	
	//Register callback to see when data is receieved.
	usb_cdc_stdio_register_callback(USB_CDC_RX_DATA, (FUNC_PTR)data_rx_callback);	
	
	/* Replace with your application code */
	while (1) {
		
		if(dataReceived)
		{
			printf("Data Received");
			dataReceived = false;
		}
	}
}


void data_rx_callback(const uint16_t length)
{
	dataReceived = true;
	
	uint32_t rxCount = stdio_io_read(&rx_buffer, USB_CDC_RX_BUF_SIZE);

	//Wait for USB to finish the last registered transfer.
	while(!cdc_tx_ready(true));
	
	//Copy from RX buffer to TX buffer.
	memcpy(tx_buffer, rx_buffer, rxCount);

	//Echo
	stdio_io_write(&tx_buffer, rxCount);
}
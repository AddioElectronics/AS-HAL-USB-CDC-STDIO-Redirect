#include <atmel_start.h>
#include "Addio/Embedded/IO/Atmel Start/usb_cdc_stdio/usb_cdc_stdio.h"
#include "Addio/Embedded/Time/System_Timer/system_timer.h"

//Add 1 character for string termination.
uint8_t rx_buffer[USB_CDC_RX_BUF_SIZE+1] = {0};
uint8_t tx_buffer[USB_CDC_TX_BUF_SIZE+1] = {0};

int dataReceived;

void data_rx_callback(const uint16_t length);

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	//Initialize system timer for millis().
	//If you are using the system timer, you will need to create your own millis() function.
	//millis() prototype located in "Addio\Embedded\Time\Timing\timing.h"
	system_timer_init();
	
	//Initialize and redirect.
	cdc_stdio_init();
	
	//Register callback to see when data is receieved.
	usb_cdc_stdio_register_callback(USB_CDC_RX_DATA, (FUNC_PTR)data_rx_callback);	
	
	/* Replace with your application code */
	while (1) {
		
		if(dataReceived)
		{
			printf("Data Received");
			
			////Echo, if rx larger than 64 bytes, data loss will occur.
			//stdio_io_write(&tx_buffer, dataReceived);
			
			dataReceived = 0;
		}
	}
}


void data_rx_callback(const uint16_t length)
{

	dataReceived = stdio_io_read(&rx_buffer, USB_CDC_RX_BUF_SIZE);
	
	//Copy from RX buffer to TX buffer.
	memcpy(tx_buffer, rx_buffer, dataReceived);


	//Echo, if rx larger than 64 bytes, com port will freeze.
	stdio_io_write(&tx_buffer, dataReceived);
}
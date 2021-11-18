#include <atmel_start.h>
#include "Addio/Embedded/IO/Atmel Start/usb_cdc_stdio/usb_cdc_stdio.h"
#include "Addio/Embedded/Time/Timing/timing.h"
#include "Addio/Embedded/Time/Timing/System_Timer/system_timer.h"

#include "Addio/Embedded/IO/Atmel Start/usb_cdc_stdio/flow_control.h"

//Add 1 character for string termination.
uint8_t rx_buffer[USB_CDC_RX_BUF_SIZE+1] = {0};
uint8_t tx_buffer[USB_CDC_TX_BUF_SIZE+1] = {0};

volatile size_t rx_length;

void data_rx_callback(const uint16_t length);

unsigned long start_time;

bool dsr = true; //cd holding
bool dcd = false;
bool breakstate = false; //dsr?


//dsr=t,dcd=f,bs=f CD holding
//dsr=f,dcd=t,bs=f dsr holding, sometimes also cts
//dsr=f,dcd=f,bs=t 

int main(void)
{
	/* Initializes MCU, drivers and middle ware */
	atmel_start_init();
	
	//Initialize system timer for millis().
	//If you are using the system timer for something else, you will need to create your own millis() function.
	//millis() prototype located in "Addio\Embedded\Time\Timing\timing.h"
	system_timer_init();
	
	//Initialize and redirect.
	cdc_stdio_init();
	
	test_flow_control_init();
	
	//Register callback to see when data is received.
	usb_cdc_stdio_register_callback(USB_CDC_RX_DATA, (FUNC_PTR)data_rx_callback);	
	
	//Wait until serial terminal has connected and raised the DTR signal
	while(!cdc_data_terminal_ready()){}
	
	printf("Connected\n");

	start_time = millis();
	
	/* Replace with your application code */
	while (1) {
		
		//Instead of using callback, this is another option.
		//Just remove "rx_length -= length;" below.
		//rx_length = cdc_get_rx_length();
		
		if(rx_length)
		{
			size_t length = stdio_io_read(&rx_buffer, rx_length);

			//Echo
			stdio_io_write(&rx_buffer, length);
			rx_length -= length;
			
			test_send_call_state_change(false);
			test_send_serial_state(false, false, false);
			//test_send_call_state_change(true);
			
			//Reset timer. Transfer from host may not have finished,
			//and this will give us time to echo more data, 
			//without having the Elapsed Time inserted in the middle.
			start_time = millis();
		}
		
		if(has_time_elapsed_ms(2500, start_time))
		{
			printf("Elapsed Time : %d\n", start_time);
			start_time = millis();
		}
	}
}


void data_rx_callback(const uint16_t length)
{
	/*
		If a TX is inside the RX callback, and more than 64 bytes are sent from the host,
		the com port will freeze. 
		The host will not allow data to be sent to it while it is still trying to send data,
		and we can not receive data while in the callback.
		This could be fixed with client side flow control, but unfortunately the HAL CDC ACM does not support it.
	*/
	
	rx_length += length;
	
	
	test_send_serial_state(dcd, dsr, breakstate);
	test_send_call_state_change(true);
}
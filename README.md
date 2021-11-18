# AS-HAL-USB-CDC-STDIO-Redirect
#### `V0.2.0`

C Library for use with Atmel Start's USB CDC ACM driver(HAL). 

Redirect STDIO to USB, and handles transfers to and from USB.



## Usage

- Create a project with Atmel Start.
  - Add the `Middleware/USB Stack/USB Device CDC ACM` component.
  - Add the `Middleware/Utilities/STDIO Redirect` component.
  - Generate your project
- Import the Addio folder and its contents to your project.
- _(Optional)_ Edit the [config file](https://github.com/AddioElectronics/AS-HAL-USB-CDC-STDIO-Redirect/blob/master/Addio/Embedded/IO/Atmel%20Start/usb_cdc_stdio/usb_cdc_stdio_config.h) to your liking.
- In `atmel_start.c`, comment out `stdio_redirect_init();`
- `#include "Addio/Embedded/IO/Atmel Start/cdc_stdio_redirect/usb_cdc_stdio.h"`
- `#include "Addio/Embedded/Time/System_Timer/system_timer.h"`
- Call `cdc_stdio_init()` and `system_timer_init()` in your initialization routine. *1 *2
- Register your desired callbacks using `usb_cdc_stdio_register_callback`
- `printf`, `stdio_io_write` and `stdio_io_read`  away!

`*1 : If the system timer is not available. You will need to write your own millis() function, which returns how many milliseconds have elapsed since program start.
Create a source file and include "Addio\Embedded\Time\Timing\timing.h", which contains the millis() prototype.`

*2 : If you are not using the Samd21, you will need to add includes to your IC in the `system_timer_atmelstart_cm0plus.c,` like the example below. In future versions more MCU's will be supported out of the box.

*3 : If you are not using a cm0plus, you will need to add an include for that. As long as they use the same macros/functions/registers as cm0plus, if you choose to you can modify `system_timer_atmelstart_cm0plus.c,` or better yet create a new file for the CPU type.  In future versions more CPU's will be supported out of the box.

``` C
/*
*	Include IC header
*/
#if defined(__SAMD21J18A__)
#include <samd21j18a.h>

#elif //Add your IC here.

#else
#error unsupported
#endif

#include <core_cm0plus.h> //Requires IC header.
```

### Example (Echo/printf)

``` C
#include <atmel_start.h>
#include "Addio/Embedded/IO/Atmel Start/usb_cdc_stdio/usb_cdc_stdio.h"
#include "Addio/Embedded/Time/Timing/timing.h"
#include "Addio/Embedded/Time/Timing/System_Timer/system_timer.h"

//Add 1 character for string termination.
uint8_t rx_buffer[USB_CDC_RX_BUF_SIZE+1] = {0};
uint8_t tx_buffer[USB_CDC_TX_BUF_SIZE+1] = {0};

volatile size_t rx_length;

void data_rx_callback(const uint16_t length);

unsigned long start_time;

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
	
	//Register callback to see when data is received.
	usb_cdc_stdio_register_callback(USB_CDC_RX_DATA, (FUNC_PTR)data_rx_callback);	
	
	//Wait until serial terminal has connected and raised the DTR signal
	while(!cdc_data_terminal_ready()){}
	
	printf("Connected\n");

	start_time = millis();
	
	/* Replace with your application code */
	while (1) {
		

		rx_length = cdc_get_rx_length();
		
		if(rx_length)
		{
			size_t length = stdio_io_read(&rx_buffer, rx_length);

			//Echo
			stdio_io_write(&rx_buffer, length);
			rx_length -= length;
		
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
}
```

## License

AS-HAL-USB-CDC-STDIO-Redirect is released under the [MIT License](http://www.opensource.org/licenses/MIT).

## Author

- Author : Addio from Addio Electronics (Canada)
- Website : www.Addio.io

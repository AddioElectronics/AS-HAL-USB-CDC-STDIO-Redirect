# AS-HAL-USB-CDC-STDIO-Redirect
#### `V0.2.0`

`Notice: In the echo example(main.c), if you receieve a USB transfer larger than 64 bytes, the com port will freeze as the host will not allow transfers to it until it has finished. By moving the write out of the callback freezing will not occur, but data loss will occur due to it not getting sent fast enough before it is overwritten. This does not apply for non-echo situations. I am currently working on a fix.`

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
Create a source file and include "Addio\Embedded\Time\Timing\timing.h", which contains the millis() prototype.

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
```

## License

AS-HAL-USB-CDC-STDIO-Redirect is released under the [MIT License](http://www.opensource.org/licenses/MIT).

## Author

- Author : Addio from Addio Electronics (Canada)
- Website : www.Addio.io

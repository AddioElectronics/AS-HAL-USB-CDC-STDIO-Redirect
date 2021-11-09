# AS-HAL-USB-CDC-STDIO-Redirect
C Library for use with Atmel Start's USB CDC driver(HAL). 

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
- Call `cdc_stdio_init()` in your initialization routine.
- Register your desired callbacks using `usb_cdc_stdio_register_callback`
- `printf` away!

### Example (Echo/printf)

``` C

#include <atmel_start.h>
#include "addio/Embedded/IO/Atmel Start/cdc_stdio_redirect/usb_cdc_stdio.h"

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

```

## License

AS-HAL-USB-CDC-STDIO-Redirect is released under the [MIT License](http://www.opensource.org/licenses/MIT).

## Author

- Author : Addio from Addio Electronics (Canada)
- Website : www.Addio.io

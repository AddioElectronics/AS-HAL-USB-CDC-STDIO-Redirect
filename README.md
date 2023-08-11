# AS-HAL-USB-CDC-STDIO-Redirect
#### `V0.2.8`

C Library for use with Atmel Start's USB CDC ACM driver(HAL). 
Redirects STDIO to USB, and handles transfers to and from USB.
Also fixes the issue that the Atmel Start example had, where data was consistantly garbage, or just plain non-existent.
## Usage

##### Device
- Create a project with Atmel Start.
  - Add the `Middleware/USB Stack/USB Device CDC ACM` component.
  - Add the `Middleware/Utilities/STDIO Redirect` component.
  - Generate your project
- Import the Addio folder and its contents to your project.
- _(Optional)_ Edit the [config file](https://github.com/AddioElectronics/AS-HAL-USB-CDC-STDIO-Redirect/blob/master/Addio/Embedded/IO/Atmel%20Start/usb_cdc_stdio/usb_cdc_stdio_config.h).
- In "atmel_start.c," comment out `stdio_redirect_init();`
- #include "Addio/Embedded/IO/Atmel Start/cdc_stdio_redirect/usb_cdc_stdio.h"
- #include "Addio/Embedded/Time/System_Timer/system_timer.h"
- Call `cdc_stdio_init()` and `system_timer_init()` in your initialization routine. ***1**
- Register your desired callbacks using `usb_cdc_stdio_register_callback`
- Wait for a connection ***2**
- `printf`, `stdio_io_write` and `stdio_io_read`  away!

##### PC
- Connect to the COM port of the device.
- Set the DTR line ***2**

1. If the system timer is not available. You will need to write your own millis() function, which returns how many milliseconds have elapsed since program start.
 Create a source file and include "Addio\Embedded\Time\Timing\timing.h", which contains the millis() prototype.
2. Data cannot be transferred or received until DTR is set.

### Supported Devices
`Recently added devices have not been tested. If there are any problems please raise an Issue.`
Should work with all Arm Cortex devices currently supported by Atmel Start.
- Cortex M0+
    - SAMD21
    - SAMDA1
    - SAML21
    - SAML22
- Cortex M3 `(Untested)`
    - SAM3A
    - SAM3N
    - SAM3S
    - SAM3U
    - SAM3X
    - SAM3SD
- Cortex M4 `(Untested)`
    - SAM4C
    - SAM4E
    - SAM4L
- Cortex M4F `(Untested)`
    - SAMD51
    - SAME51
    - SAME53
    - SAMD54
- Cortex M7 `(Untested)`
    - SAME70
    - SAMS70
    - SAMV70
    - SAMV71

#### How to Add Support for a Device
- Navigate to `Addio\Embedded\Time\Timing\System_Timer\mcu` folder
- Open source file of your cpu type ***1** 
- Add an #elif directive checking if your device is defined `#elif defined(__SAMD21J18A__)` ***2**
- Under #elif include your device `#include <samd21j18a.h>` ***3**
- Open `check_mcu_core.h`
- Add your device like `|| defined(__SAM****__)` to the macro for your cpu type.

1. For M0+ that would be `system_timer_atmelstart_cm0plus.c`
2. To get device definition go to "Project Properties\Toolchain\ARM/GNU Common" and copy value after -D
3. For SAMD21J18A that would be `#include "samd21j18a.h"`

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

#### How to Add Support for a CPU
- Navigate to `Addio\Embedded\Time\Timing\System_Timer\mcu` folder
- Duplicate one of the source files and rename to your cpu type
- On line `#if __has_include("RTE_Components.h") && __has_include("core_cmx.h")`, replace "core_cmx.h" with your CPU's core header.
- Remove all devices under /\*Include IC header\*/
- Add your device as seen [here](#####How-to-Add-Support-for-a-Device). 

Note : *system_timer_init()*, *millis()*, and *micros()* may need to be modified. I am unsure if all Atmel Start Devices share Macros and Structures for the System Timer.

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
			uint32_t hr, min, sec, ms;
			convert_ms_to_time(start_time, &hr, &min, &sec, &ms);
			start_time = millis();
			printf("Elapsed Time : %d:%d:%02d.%03d\t-\t%d Total Milliseconds\n\r", hr, min, sec, ms);
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

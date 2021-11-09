#include "usb_cdc_stdio.h"

#include <err_codes.h>
#include <hal_io.h>
#include <hal_atomic.h>

#include "../../../../Universal/IO/buffer/ring_buffer.h"



#pragma region Variables

/*
*	The IO descriptor which holds the pointers to the read/write functions for the STDIO redirect.
*/
struct io_descriptor USB_CDC_IO;


/* 
*	Buffers to receive and transfer data to and from the USB interface. 
	One byte added for null termination character.
*/
static			uint8_t cdc_tx_buffer[USB_CDC_TX_BUF_SIZE + 1];
static volatile uint8_t cdc_rx_buffer[USB_CDC_RX_BUF_SIZE + 1];

/*
*	We must still use the standard rx_buffer, as the HAL USB handles populating the buffer.
*/
#if CDC_MULTI_BUFFER == true
static volatile uint8_t cdc_secondary_rx_buffer[CDC_SECONDARY_BUFFER_SIZE];
static volatile ring_buffer_t rx_ring_buffer;
#else

#endif

/*
*	Is there a write currently waiting or being transferred?
*/
volatile bool cdc_data_transfering;

/*
*	If true, the data in the TX buffer will not be sent.
*	This allows the tx buffer to be filled with multiple write operations before sending.
*/
bool tx_wait_to_fill;

/*
*	The size of the last chunk of data received.
*/
volatile uint16_t cdc_rx_length;

/* 
*	Used for keeping track of the last writes length,
*	for the event it gets stuck and needs to be resent.
*/
volatile uint16_t cdc_tx_length;



/*
*	The internal structure holding the callbacks.
*/
struct usb_cdc_callbacks callbacks;

#pragma endregion Variables

#pragma region Function Headers

/*
*	Function headers for static methods.
*/
static bool cdc_usb_device_cb_state_c(usb_cdc_control_signal_t state);
static int32_t cdc_stdio_write(struct io_descriptor* const io_descr, const uint8_t* buf, const uint16_t length);
static cdc_stdio_read(struct io_descriptor* const io_descr, const uint8_t* buf, const uint16_t length);
static bool cdc_cb_bulk_out(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count);
static bool cdc_cb_bulk_in(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count);

#pragma endregion Function Headers

#pragma region Initialization Functions

/*
 *	Initialize redirect of stdio to USB CDC.
 */
void cdc_stdio_init()
{
	cdc_data_transfering = false;
	
	#if CDC_MULTI_BUFFER == true
	ring_buffer_init(&cdc_secondary_rx_buffer, CDC_SECONDARY_BUFFER_SIZE, 1, &rx_ring_buffer);
	#endif
	
	////Set callbacks as dummy for the event a polling implementation is used.
	callbacks.dataReceieved = NULL;
	callbacks.txComplete = NULL;
	callbacks.blockTx = NULL;
	callbacks.allBlocksTx = NULL;

	//Set IO functions for STDIO redirection
	USB_CDC_IO.read  = cdc_stdio_read;
	USB_CDC_IO.write = cdc_stdio_write;

	//Initialize STDIO redirection.
	stdio_io_init(&USB_CDC_IO);
	
	//Register USB callbacks.
	cdcdf_acm_register_callback(CDCDF_ACM_CB_READ, (FUNC_PTR)cdc_cb_bulk_in);
	cdcdf_acm_register_callback(CDCDF_ACM_CB_WRITE, (FUNC_PTR)cdc_cb_bulk_out);
	cdcdf_acm_register_callback(CDCDF_ACM_CB_STATE_C, (FUNC_PTR)cdc_usb_device_cb_state_c);
	
	//Wait for USB to fully initialize.
	//usb_init() must be called before cdc_stdio_init()
	while(usbdc_get_state() != USBD_S_CONFIG);
	
	//Dummy write and read needed to stop locking of comms.
	cdcdf_acm_write(cdc_tx_buffer, 0); 
	
	//Dummy read sets read buffer destination. DMA will automatically put data from out endpoint in to buffer.
	//Still needs to be called every read to prevent locking.
	cdcdf_acm_read(cdc_rx_buffer, sizeof(cdc_rx_buffer));
	
	return ERR_NONE;
}

/*
 *	Register a callback function.
 *
 * \param[in] cb_type	The callback type.
 * \param[in] func		The callback function.
 *
 * \return Exit code.
 */
int32_t usb_cdc_stdio_register_callback(enum usb_cdc_cb_type cb_type, FUNC_PTR func)
{	
	switch (cb_type) {
		case USB_CDC_RX_DATA:
		callbacks.dataReceieved = (cdc_rx_data)func;
		break;
		case USB_CDC_TX_COMPLETE:
		callbacks.txComplete = (cdc_tx_complete)func;
		break;
		case USB_CDC_BLOCK_TX:
		callbacks.blockTx = (cdc_block_tx)func;
		break;
		case USB_CDC_ALL_BLOCKS_TX:
		callbacks.allBlocksTx = (cdc_all_blocks_tx)func;
		break;
		default:
		return ERR_INVALID_ARG;
	}
	return ERR_NONE;
}

#pragma endregion Initialization Functions

#pragma region Status Functions


size_t __attribute__((__always_inline__)) cdc_tx_capacity()
{
	return USB_CDC_RX_BUF_SIZE - cdc_tx_length;
}


#if CDC_TX_RETRY == false

/*
*	Amount of times cdc_tx_ready has been called, and returned false.
*/
static uint16_t tx_attempts;

/*
*	Has the last write to the TX buffer been transmitted?
*	When tx_attempts reaches CDC_TX_MAX_RETRIES, it will either give up on the transfer,
*	or resend the transfer.

*	/param		retry	Should the last transfer be resent if it reaches a timeout state?
*
*	/returns	True	If the last write has been transferred or reached the timeout.
*				False	If the last write is still waiting to be transferred.
*/
bool cdc_tx_ready(bool retry)
{
	if(cdc_data_transfering)
	{
		if(++tx_attempts > CDC_TX_MAX_RETRIES)
		{
			if(retry)
			{
				cdc_retry_last_tx();				
			}
			cdc_data_transfering = false;
			tx_attempts = 0;
			return true;
		}
		return false;
	}
	
	tx_attempts = 0;
	return true;
}

/*
*	Has the last write to the TX buffer been transmitted?
*	Will wait for transfer or to reach the timeout.
*	When tx_attempts reaches CDC_TX_MAX_RETRIES, it will either give up on the transfer,
*	or resend the transfer.
*
*	/param		retry	Should the last transfer be resent if it reaches a timeout state?
*
*	/returns			True when TX is ready, False if the connection is not accepting transfers.
*/
bool cdc_tx_ready_wait(bool retry)
{
	//if(!control_state.rs232.DTR) return false;
	
	//if(!cdc_connected()) return false;
	while(!cdc_tx_ready(retry));
	
	return true;
}

/*
 * Attempt to resend the last write, if it was not transfered.
 *
 * /returns		True if the last transfer was attempted to be resent.
 *				False if there were no transfers in a stuck state.
 */
bool cdc_retry_last_tx()
{
	//Last transfer was sent successfully,
	//will not resend.
	if(!cdc_data_transfering) return false;
	
	cdcdf_acm_write(cdc_tx_buffer, cdc_tx_length);	
	return true;
}

#endif

/*
 *	Have any new bytes been received?
 *
 *	/returns	If there are new bytes in the cdc_rx_buffer.
 */
bool cdc_rx_ready(void)
{
	return cdc_rx_length > 0;
}

/*
 *	Get the count of bytes received.
 *
 *	/returns	The number of new bytes in the cdc_rx_buffer
 */
uint16_t cdc_get_rx_size()
{
	return cdc_rx_length;
}

#pragma endregion Status Functions

#pragma region Functions

/*
*	If enabled, any write will be stored in the TX buffer,
*	and not transferred until either the buffer is full,
*	or until cdc_disable_tx_wait_to_fill() is called.
*
*	*The standard windows driver tends to lose a lot of data when using this.
*	*I added this feature to try and circumvent the loss of data when sending 
*	large amounts of data, 1 byte at a time, but it only made the problem worse.
*	The best way to stop the loss of data from MCU to PC is to use/increase CDC_TRANSFER_DELAY.
*/
void cdc_enable_tx_wait_to_fill()
{
	tx_wait_to_fill = true;
}

/*
*	If disabled, a future write of any size will be sent via USB immediately.
*	If the buffer currently has any thing inside, they will be sent.
*/
void cdc_disable_tx_wait_to_fill()
{
	tx_wait_to_fill = false;
	
	if(cdc_tx_length > 0)
	{
		cdcdf_acm_write(cdc_tx_buffer, cdc_tx_length);
	}
}


#pragma endregion Functions

#pragma region STDIO Functions


/*
 * \internal Write the given data from the STDIO redirect, to the USB interface.
 *
 * \param[in] descr		The pointer to an io descriptor
 * \param[in] buf		Data to write to usart
 * \param[in] length	The number of bytes to write
 *
 * \return The number of bytes written.
 */
 static int32_t cdc_stdio_write(struct io_descriptor *const io_descr, const uint8_t* buf, const uint16_t length)
{	
	#if CDC_TX_USE_CRITICAL == true
	volatile hal_atomic_t flags;
	#endif
	
	uint16_t txed = 0;
	static CDC_TX_TIMEOUT_TYPE	timeoutCounter = 0;
	static CDC_TX_RETRY_TYPE	retryCount = 0;
	
	
	#if CDC_TX_RETRY == false
	//If a previous write is not complete. Wait for it.
	//If CDC_TX_RETRY is true, this function will not exit until a transfer has completed, and will always be true.
	while(!cdc_tx_ready(true));
	#endif
	
	
	
	while(txed < length)
	{
		
		//Get pointer to unused section of buffer.
		uint8_t* tx_buffer = cdc_tx_buffer + cdc_tx_length;
		
		//How many bytes will be transferred.
		size_t tx_size =  length < USB_CDC_TX_BUF_SIZE - cdc_tx_length ? length : USB_CDC_TX_BUF_SIZE - cdc_tx_length;
		
		memcpy(tx_buffer, buf, tx_size);	//Copy data into TX buffer.
		cdc_tx_length += tx_size;			//Mark how many bytes are currently in the TX buffer.
		txed += tx_size;					//Mark how many bytes for the current write have been sent.
		
		RetryTransfer:
		
		//Send data in buffer.
		if(!tx_wait_to_fill || (tx_wait_to_fill && cdc_tx_length == USB_CDC_TX_BUF_SIZE))
		{
			//Mark that a transfer is underway.
			cdc_data_transfering = true;
			
			cdcdf_acm_write(cdc_tx_buffer, cdc_tx_length);
			
			//#if CDC_TX_RETRY_ == false
			//cdc_tx_length = txed;
			//#endif
		}

		
		//If there are still bytes waiting to be transferred, wait for the tx to complete.
		//If CDC_TX_RETRY is true, it will always wait until the last transfer has finished, and try to resend failed transfers.
		while(cdc_data_transfering && (txed < length || CDC_TX_RETRY))
		{
			if(++timeoutCounter > CDC_TX_TIMEOUT)
			{
				//Skip transmitting block.
				if(!CDC_TX_RETRY || ++retryCount > CDC_TX_MAX_RETRIES)
				{
					cdc_data_transfering = false;
					cdc_tx_length = 0;
					break;
				}
				
				//Retry sending block.
				timeoutCounter = 0;
				goto RetryTransfer;
			}
		}
		
		// Trigger the cdc_data_sent callback to signify a block has been sent to USB
		if(callbacks.blockTx != NULL)
		(*callbacks.blockTx)(tx_size);
		
	}
	
	// Trigger the cdc_data_sent callback to signify all blocks have been sent to USB
	if(callbacks.allBlocksTx != NULL)
	(*callbacks.allBlocksTx)(length);
	
	return (int32_t)length;
}

/*
 * \internal Read string from the buffer that was received from the USB interface.
 *
 * \param[in] descr		The pointer to an io descriptor
 * \param[in] buf		A buffer to read data to
 * \param[in] length	The size of a buffer, or how many bytes to read.
 *
 * \return The number of bytes read.
 */
static cdc_stdio_read(struct io_descriptor* const io_descr, const uint8_t*  buf, const uint16_t length)
{
	if(cdc_rx_length == 0) return 0;
	

	#if CDC_MULTI_BUFFER == true
	{
		cdc_rx_length -= ring_buffer_read(&rx_ring_buffer, buf, length);
	}
	#else
	{

		uint16_t rx_count = length;
		
		if(rx_count > USB_CDC_RX_BUF_SIZE)
		rx_count = USB_CDC_RX_BUF_SIZE;
		
		memcpy(buf, &cdc_rx_buffer, rx_count);
		
		cdc_rx_length = 0;
	}
	#endif
		
	
	
	return (int32_t)strlen(buf);
}

#pragma endregion STDIO Functions

#pragma region USB Callback Functions

/*
 * brief Callback invoked when Line State Change
 */
 static bool cdc_usb_device_cb_state_c(usb_cdc_control_signal_t state)
{
	
	if (state.rs232.DTR) {
		/* Callbacks must be registered after endpoint allocation */
		cdcdf_acm_register_callback(CDCDF_ACM_CB_READ, (FUNC_PTR)cdc_cb_bulk_in);
		cdcdf_acm_register_callback(CDCDF_ACM_CB_WRITE, (FUNC_PTR)cdc_cb_bulk_out);
		
		/* Start Rx */
		cdcdf_acm_read((uint8_t*)cdc_rx_buffer, sizeof(cdc_rx_buffer));
	}

	/* No error. */
	return false;
}



/*
 * \internal	Callback invoked when bulk OUT data received
 *
 * \param[in] ep	The USB endpoint.
 * \param[in] rc	The USB transfer.
 * \param[in] count The amount of bytes being sent.
 */
static bool cdc_cb_bulk_out(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
	#if CDC_TRANSFER_DELAY > 0
	delay_ms(CDC_TRANSFER_DELAY);
	#endif
	
	cdc_data_transfering = false;
	
	//#if CDC_TX_RETRY_ == false
	cdc_tx_length = 0;
	//#endif
	
	// Trigger the cdc_data_sent callback to signify a transfer has finished.
	if(callbacks.txComplete != NULL)
	(*callbacks.txComplete)(count);
		
	
	
	return false;
}

/*
 * \internal	Callback invoked when the USB interface and buffer has received data.
 *				Handles setting up next transfer, and changing status.
 *
 * \param[in] ep	The USB endpoint.
 * \param[in] rc	The USB transfer.
 * \param[in] count The amount of bytes received.
 */
static bool cdc_cb_bulk_in(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{	
	//Data is already in the buffer, USB used DMA to copy data from endpoint to buffer.
	//All we need to do is set some variables to signify that we received data, call a callback,
	//and then setup the USB module for its next read transfer (by calling cdcdf_acm_read).
	
	//Transfer data from the rx_buffer to the ring buffer.
	#if CDC_MULTI_BUFFER == true
	{
		cdc_rx_length += ring_buffer_write(&rx_ring_buffer, cdc_rx_buffer, count);
		
		if(cdc_rx_length > 0)
		{
			if(callbacks.dataReceieved != NULL)
			(*callbacks.dataReceieved)(cdc_rx_length); //Pass the current length of the buffer, not how many bytes were received
		}
	}	
	#else
	{
		//Set all bytes after rx to 0 to allow string functions.
		//memset((cdc_rx_buffer + count), 0, USB_CDC_RX_BUF_SIZE - count);
		cdc_rx_buffer[count] = 0;
		
		//Signify that data has been received.
		cdc_rx_length = count;
		
		if(cdc_rx_length > 0)
		{
			if(callbacks.dataReceieved != NULL)
			(*callbacks.dataReceieved)(count);
		}
	}	
	
	#endif
	
	// Setup USB transfer for next rx.
	cdcdf_acm_read((uint8_t *)cdc_rx_buffer, sizeof(cdc_rx_buffer));
	
	return false;
}

#pragma endregion USB Callback Functions



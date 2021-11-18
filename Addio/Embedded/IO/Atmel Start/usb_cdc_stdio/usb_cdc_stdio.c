#include "usb_cdc_stdio.h"

#include <err_codes.h>
#include <hal_io.h>
#include <hal_usb_device.h>


#include "../../../Time/timing/timing.h"
#include "../../../../Universal/IO/buffers/ring_buffer.h"


#pragma region Variables

/*
*	The IO descriptor which holds the pointers to the read/write functions for the STDIO redirect.
*/
struct io_descriptor USB_CDC_IO;

union{
struct{
	uint8_t DTR : 1; //!< Data Terminal Ready
	uint8_t RTS : 1; //!< Request To Send
	};
	uint8_t value;
} rs232_control_state;

/* 
*	Buffers to receive and transfer data to and from the USB interface. 
*/
static			uint8_t cdc_tx_buffer[USB_CDC_TX_BUF_SIZE];

static volatile uint8_t cdc_rx_buffer[USB_CDC_RX_BUF_SIZE];

/*
*	We must still use the standard rx_buffer, as the USB interface handles populating the buffer.
*/
#if CDC_MULTI_BUFFER == true
static volatile uint8_t cdc_secondary_rx_buffer[CDC_SECONDARY_BUFFER_SIZE];
static volatile ring_buffer_t rx_ring_buffer;
#else
static volatile uint8_t cdc_rx_buffer_pos = 0;
#endif

/*
*	Is there a write currently waiting or being transferred?
*/
volatile bool cdc_data_transfering;

/*
*	If true, the data in the TX buffer will not be sent.
*	This allows the tx buffer to be filled with multiple write operations before sending.
*/
bool tx_hold_buffer;

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
*	The time at which the last transfer was sent. (milliseconds)
*	Used for limiting the amount of transfers to avoid data loss.
*/
unsigned long previous_write_timestamp;

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
static int32_t cdc_stdio_write(struct io_descriptor *const io_descr, const uint8_t* buf, const uint16_t length);
static int32_t cdc_stdio_read(struct io_descriptor *const io_descr, const uint8_t* buf, const uint16_t length);

#if __has_include("../../addio_io.h")
static int32_t cdc_stdio_peekMany(struct io_descriptor *const io_descr, const uint8_t* buf, const uint16_t length);
static int32_t cdc_stdio_peek(struct io_descriptor *const io_descr);
static int32_t cdc_stdio_rxReady(struct io_descriptor *const io_descr);
static int32_t  cdc_stdio_txReady(struct io_descriptor *const io_descr);
#endif

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
	
	//Use extended io, required for Addio/Embedded/IO/Serial/(print/reader)
	#if __has_include("../../addio_io.h")
	//USB_CDC_IO.get = cdc_stdio_get;
	USB_CDC_IO.peek = cdc_stdio_peek;
	USB_CDC_IO.peekMany = cdc_stdio_peekMany;
	USB_CDC_IO.rxReady = cdc_stdio_rxReady;
	USB_CDC_IO.txReady = cdc_stdio_txReady;
	
	USB_CDC_IO.flags.tx_wait_for_complete = false;
	USB_CDC_IO.flags.tx_check_previous_for_completion = false;
	USB_CDC_IO.flags.tx_min_interval = 0;// CDC_MIN_TX_INTERVAL; Handled internally
	USB_CDC_IO.flags.print_quick = true;
	#endif

	//Initialize STDIO redirection.
	stdio_io_init(&USB_CDC_IO);
	
	//Register USB callbacks.
	//cdcdf_acm_register_callback(CDCDF_ACM_CB_READ, (FUNC_PTR)cdc_cb_bulk_out);
	//cdcdf_acm_register_callback(CDCDF_ACM_CB_WRITE, (FUNC_PTR)cdc_cb_bulk_in);
	cdcdf_acm_register_callback(CDCDF_ACM_CB_STATE_C, (FUNC_PTR)cdc_usb_device_cb_state_c);
	
	//Wait for USB to fully initialize.
	//usb_init() must be called before cdc_stdio_init()
	while(usbdc_get_state() != USBD_S_CONFIG);
	
	////Dummy write and read needed to stop locking of comms.
	//cdcdf_acm_write(cdc_tx_buffer, 0); 
	
	//Register read. This tells the USB interface to place the next RX in the cdc_rx_buffer.
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


/*
*	Gets a count of how many empty bytes are in the TX buffer.
*	Unless "tx_wait_to_fill" is used, this will
*
*	/returns	How many empty bytes are in the TX buffer.
*/
size_t cdc_tx_capacity()
{
	if(tx_hold_buffer){
		return USB_CDC_TX_BUF_SIZE - cdc_tx_length;
	}else if(cdc_data_transfering){
		return 0;
	}else{
		return USB_CDC_TX_BUF_SIZE;
	}
}

/*
*	Has the host connection signaled it is accepting data?
*
*	/returns	True	If the host has raised the DTR signal.
*				False	If the host has stopped the DTR signal, or if no host is connected.
*/
bool __attribute__((__always_inline__)) cdc_data_terminal_ready()
{
	return rs232_control_state.DTR;
}


/*
*	Has the last write to the TX buffer been transmitted?
*
*	/returns	True	If the last write has been transferred or reached the timeout.
*				False	If the last write is still waiting to be transferred.
*/
bool __attribute__((__always_inline__)) cdc_tx_ready()
{
	return !cdc_data_transfering && cdc_data_terminal_ready() && cdcdf_acm_is_enabled();
}

/*
*	Amount of times cdc_retry_last_tx has been called from cdc_tx_ready_timeout.
*/
static uint8_t tx_attempts;


/*
*	Has the last write to the TX buffer been transmitted, and is the connection valid?
*
*	/param		retry	Should the last transfer be resent if it reaches a timeout state?
*
*	/returns	True	If the transfer has completed, or if the connection was regained.
*				False	If the transfer has timed out, or if the connection failed.
*/
bool cdc_tx_ready_timeout(bool retry)
{
	unsigned long start_time = millis();
	tx_attempts = 0;
	while(!cdc_tx_ready())
	{
		if(has_time_elapsed_ms(CDC_TX_TIMEOUT, start_time))
		{
			#if CDC_TX_MAX_RETRIES >= 0
			if(++tx_attempts < CDC_TX_MAX_RETRIES)
			{
				if(retry)
				{
					cdc_retry_last_tx();
				}		
			}	
			else
			{
				cdc_data_transfering = false;
				//cdc_abort_tx();
				return false;
			}
				
			#else
			cdc_data_transfering = false;
			//cdc_abort_tx();
			return false;
			#endif	
		}
	}
	
	return true;
}

/*
 *	Attempt to resend the last write, if it was not transfered.
 *	If the last transfer was aborted, it will not retry
 *
 * /returns		True if the last transfer was attempted to be resent.
 *				False if there were no transfers in a stuck state.
 */
bool cdc_retry_last_tx()
{
	//Last transfer was sent successfully, DTR signal is low, or .
	if(!cdc_data_transfering || !cdc_data_terminal_ready() || !cdcdf_acm_is_enabled()) return false;

	
	cdcdf_acm_write(cdc_tx_buffer, cdc_tx_length);	
	
	#if CDC_MIN_TX_INTERVAL > 100
	previous_write_timestamp = micros();
	#endif
	
	return true;
}

/*
 *	Get the count of bytes received.
 *
 *	/returns	The number of new bytes in the cdc_rx_buffer
 */
uint16_t __attribute__((__always_inline__)) cdc_get_rx_length()
{
	return cdc_rx_length;
}

#pragma endregion Status Functions

#pragma region Functions

void __attribute__((__always_inline__)) cdc_get_io_descriptor(struct io_descriptor **io)
{
	return &USB_CDC_IO;
}

//extern static struct cdcdf_acm_func_data _cdcdf_acm_funcd;
//
//void cdc_abort_tx()
//{
	//cdc_data_transfering = false;
	//usb_d_ep_abort(_cdcdf_acm_funcd.func_ep_in[1/*CDCDF_ACM_DATA_EP_INDEX*/]);
//}
//
//void cdc_abort_rx()
//{
	//usb_d_ep_abort(_cdcdf_acm_funcd.func_ep_out);
//}

/*
*	If enabled, any write will be stored in the TX buffer,
*	and will not transfer until either the buffer is full,
*	or until cdc_set_hold_buffer() is set to false.
*
*	/returns	bool	True if a transfer was sent when disabling. False if no transfer was sent, or if it was enabled.
*/
bool cdc_set_tx_hold_buffer(bool wait)
{
	tx_hold_buffer = wait;
	
	if(tx_hold_buffer == false && cdc_tx_length > 0)
	{
		cdc_data_transfering = true;
		cdcdf_acm_write(cdc_tx_buffer, cdc_tx_length);
		
		#if CDC_MIN_TX_INTERVAL > 100
		previous_write_timestamp = micros();
		#endif
		
		return true;
	}
	
	return false;
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
 * \return The number of bytes written, or -1 if the connection was lost.
 */
 static int32_t cdc_stdio_write(struct io_descriptor *const io_descr, const uint8_t* buf, const uint16_t length)
{	
	uint16_t txed = 0;
	uint16_t succesful = 0;
	static uint8_t	retryCount = 0;
	
	//If a previous write is not complete, or if the DTR signal is low, give it a chance.
	if(!cdc_tx_ready_timeout(true))
	{
		//No connection, give up.
		if(!cdc_data_terminal_ready())
		return -1;
		
		//Previous transfer canceled, continue with current.
	}
	

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
		if(!tx_hold_buffer || (tx_hold_buffer && cdc_tx_length == USB_CDC_TX_BUF_SIZE))
		{
			#if CDC_MIN_TX_INTERVAL > 250 //Taking a guess that any CPU will take longer than 250 microseconds to arrive back here.
			//Wait until the minimum amount of microseconds have elapsed since the last transfer.
			while(micros() - previous_write_timestamp < CDC_MIN_TX_INTERVAL){}
			#endif
				
			//Mark that a transfer is underway.
			cdc_data_transfering = true;
			
			//Initiate the USB transfer
			cdcdf_acm_write(cdc_tx_buffer, cdc_tx_length);
			
			#if CDC_MIN_TX_INTERVAL > 250
			previous_write_timestamp = micros();
			#endif
		}
		
		
		
		//If there are still bytes waiting to be transferred, wait for the tx to complete.
		//If CDC_TX_RETRY is true, it will always wait until the last transfer has finished, and try to resend failed transfers.
		unsigned long start_time = millis();
		while(cdc_data_transfering && (txed < length || CDC_WAIT_FOR_TX_COMPLETE))
		{
			
			if(has_time_elapsed_ms(CDC_TX_TIMEOUT, start_time))
			{

				#if CDC_TX_MAX_RETRIES > 0
				{
					//Skip transmitting block.
					if(++retryCount > CDC_TX_MAX_RETRIES)
					{
						//cdc_abort_tx();
						cdc_data_transfering = false;
						cdc_tx_length = 0;
						tx_size = 0;
						break;
					}
					//Transfer block failed. Retry.
					goto RetryTransfer;
				}
				#else
				{
					//Skip transmitting block.
					//cdc_abort_tx();
					cdc_data_transfering = false;
					cdc_tx_length = 0;
					tx_size = 0;
					break;
				}
				#endif
			}
		}
		
		
		//Transfer was a success
		if(tx_size > 0)
		{
			succesful += tx_size;
			
			// Trigger the cdc_data_sent callback to signify a block has been sent to USB
			if(callbacks.blockTx != NULL)
			(*callbacks.blockTx)(tx_size);
		}
	}
	
	// Trigger the cdc_data_sent callback to signify all blocks have been sent to USB
	if(callbacks.allBlocksTx != NULL)
	(*callbacks.allBlocksTx)(succesful);
	
	return (int32_t)succesful;
}


/*
 * \internal Read string from the buffer that was received from the USB interface.
 *
 * \param[in] buf		A buffer to read data to
 * \param[in] length	The size of a buffer, or how many bytes to read.
 *
 * \return The number of bytes read.
 */
static int32_t cdc_stdio_read(struct io_descriptor *const io_descr, const uint8_t*  buf, const uint16_t length)
{
	if(cdc_rx_length == 0) return 0;
	
	uint32_t rx_count;

	#if CDC_MULTI_BUFFER == true
	{
		rx_count = ring_buffer_read(&rx_ring_buffer, buf, length);
		cdc_rx_length -= rx_count;
	}
	#else
	{

		rx_count = length;
		
		if(rx_count > USB_CDC_RX_BUF_SIZE - cdc_rx_buffer_pos)
		rx_count = USB_CDC_RX_BUF_SIZE - cdc_rx_buffer_pos;
		
		memcpy(buf, cdc_rx_buffer + cdc_rx_buffer_pos, rx_count);
		
		cdc_rx_length -= rx_count;
		
		cdc_rx_buffer_pos += rx_count;
		
	}
	#endif
		
	
	
	return (int32_t)rx_count;
}

#if __has_include("../../addio_io.h") // required for Addio/Embedded/IO/Serial/(print/reader)

#if CDC_MULTI_BUFFER == true

static int32_t cdc_stdio_peek(struct io_descriptor *const io_descr)
{
	if(cdc_rx_length == 0) return -1;
	
	uint8_t data;
	ring_buffer_peek(&rx_ring_buffer, &data);
	return (int32_t)data;
}

static int32_t cdc_stdio_peekMany(struct io_descriptor *const io_descr, const uint8_t*  buf, const uint16_t length)
{
	if(cdc_rx_length == 0) return 0;
	
	return (int32_t)ring_buffer_peekMany(&rx_ring_buffer, buf, length);
}

#else

static int32_t cdc_stdio_peek(struct io_descriptor *const io_descr)
{
	if(cdc_rx_length == 0) return -1;
	return (int32_t) cdc_rx_buffer[0];
}

static int32_t cdc_stdio_peekMany(struct io_descriptor *const io_descr, const uint8_t*  buf, const uint16_t length)
{
	if(cdc_rx_length == 0) return 0;
	
	uint16_t peek_count = length < cdc_rx_length ? length : cdc_rx_length;
	
	memcpy(buf, cdc_rx_buffer, peek_count);
	return (int32_t)peek_count;
}

#endif //CDC_MULTI_BUFFER == true

static int32_t __attribute__((__always_inline__)) cdc_stdio_rxReady(struct io_descriptor *const io_descr)
{
	return cdc_rx_length;
}


static int32_t __attribute__((__always_inline__)) cdc_stdio_txReady(struct io_descriptor *const io_descr)
{
	if(cdc_data_terminal_ready())
	return cdc_tx_capacity();
	
	return false;
}

#endif //__has_include("../../addio_io.h")

#pragma endregion STDIO Functions

#pragma region USB Callback Functions

static bool cdc_cb_dummy(const uint8_t ep, const enum usb_xfer_code rc, const uint16_t count)
{
	
};

/*
 * brief Callback invoked when Line State Change
 */
 static bool cdc_usb_device_cb_state_c(usb_cdc_control_signal_t state)
{

	if (state.rs232.DTR) 
	{
		/* Callbacks must be registered after endpoint allocation */
		cdcdf_acm_register_callback(CDCDF_ACM_CB_READ, (FUNC_PTR)cdc_cb_bulk_out);
		cdcdf_acm_register_callback(CDCDF_ACM_CB_WRITE, (FUNC_PTR)cdc_cb_bulk_in);
		
		/* Start Rx */
		cdcdf_acm_read((uint8_t*)cdc_rx_buffer, sizeof(cdc_rx_buffer));
		
		rs232_control_state.DTR = state.rs232.DTR;
	}
	
	if(state.rs232.RTS)
	{
		rs232_control_state.RTS = state.rs232.RTS;
	}
	
	//USB device unconnected, or flow control used.
	if (state.rs232.DTR == 0 && state.rs232.RTS == 0) 
	{
		cdcdf_acm_stop_xfer();
		//Was causing the COM port to freeze if transfers were interrupted.
		
		cdcdf_acm_register_callback(CDCDF_ACM_CB_READ, (FUNC_PTR)cdc_cb_dummy);
		cdcdf_acm_register_callback(CDCDF_ACM_CB_WRITE, (FUNC_PTR)cdc_cb_dummy);
		
		rs232_control_state.value = 0;
	}

	/* No error. */
	return false;
}



/*
 * \internal	Callback invoked when a transfer to the host has finished. 
 *
 *				*Direction is from the host perspective, "in" means the host is receiving.
 *
 * \param[in] ep	The USB endpoint.
 * \param[in] rc	The USB transfer.
 * \param[in] count The amount of bytes being sent.
 */
static bool cdc_cb_bulk_in(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
	cdc_data_transfering = false;
	cdc_tx_length = 0;
	
	// Trigger the cdc_data_sent callback to signify a transfer has finished.
	if(callbacks.txComplete != NULL)
	(*callbacks.txComplete)(count);
		
	return false;
}

/*
 * \internal	Callback invoked when the USB interface has populated our buffer with received data.
 *				Handles setting up next transfer, and changing status.
 *
 *				*Direction is from the host perspective, "out" means the host is sending.
 *
 * \param[in] ep	The USB endpoint.
 * \param[in] rc	The USB transfer.
 * \param[in] count The amount of bytes received.
 */
static bool cdc_cb_bulk_out(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
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
		
		cdc_rx_buffer_pos = 0;
		
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



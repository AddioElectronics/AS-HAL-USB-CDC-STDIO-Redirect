#ifndef USB_CDC_STDIO_CONFIG_H_
#define USB_CDC_STDIO_CONFIG_H_


/*
*	If true, a secondary ring buffer will be used to store the received data.
*	This avoids having data overwritten if 2 transfers arrive before the first one is read,
*	and is especially useful when the host sends data 1 byte at a time.
*/
#define CDC_MULTI_BUFFER true

/*
*	The maximum amount of characters in the ring buffer.
*/
#define CDC_SECONDARY_BUFFER_SIZE 64


/*
*	Define buffer sizes.
*
*	*The only values that can be changed are the TX buffer sizes, 
*	and they must never be greater than the CONF_USB_CDCD_ACM_DATA_BULKOUT_MAXPKSZ(_HS).
*/
#if CONF_USBD_HS_SP
	#define USB_CDC_TX_BUF_SIZE CONF_USB_CDCD_ACM_DATA_BULKOUT_MAXPKSZ_HS
	#define USB_CDC_RX_BUF_SIZE CONF_USB_CDCD_ACM_DATA_BULKIN_MAXPKSZ_HS	//Must be same size as USB
	
	//Buffer larger than USB packet size.
	#if USB_CDC_TX_BUF_SIZE > CONF_USB_CDCD_ACM_DATA_BULKOUT_MAXPKSZ_HS
	#undef USB_CDC_TX_BUF_SIZE
	#define  USB_CDC_TX_BUF_SIZE CONF_USB_CDCD_ACM_DATA_BULKOUT_MAXPKSZ_HS
	#endif
	
	//Data loss will occur if smaller than USB,
	//anything larger will be unused.
	#if USB_CDC_RX_BUF_SIZE != CONF_USB_CDCD_ACM_DATA_BULKIN_MAXPKSZ_HS
	#undef USB_CDC_RX_BUF_SIZE
	#define  USB_CDC_RX_BUF_SIZE CONF_USB_CDCD_ACM_DATA_BULKIN_MAXPKSZ_HS
	#endif	
#else
	#define USB_CDC_TX_BUF_SIZE CONF_USB_CDCD_ACM_DATA_BULKOUT_MAXPKSZ		//Max is 64.
	#define USB_CDC_RX_BUF_SIZE CONF_USB_CDCD_ACM_DATA_BULKIN_MAXPKSZ		//Must be same size as USB

	//Buffer larger than USB packet size.
	#if USB_CDC_TX_BUF_SIZE > CONF_USB_CDCD_ACM_DATA_BULKOUT_MAXPKSZ
	#undef USB_CDC_TX_BUF_SIZE
	#define  USB_CDC_TX_BUF_SIZE CONF_USB_CDCD_ACM_DATA_BULKOUT_MAXPKSZ
	#endif
	
	//Data loss will occur if smaller than USB,
	//anything larger will be unused.
	#if USB_CDC_RX_BUF_SIZE != CONF_USB_CDCD_ACM_DATA_BULKIN_MAXPKSZ
	#undef USB_CDC_RX_BUF_SIZE
	#define  USB_CDC_RX_BUF_SIZE CONF_USB_CDCD_ACM_DATA_BULKIN_MAXPKSZ
	#endif
#endif

/*
*	Limit transfers to only occur every n microseconds.
*	After a transfer has been written, a timestamp is created.
*	If a succeeding transfer tries to start before the minimum interval has elapsed,
*	it will wait for the time to elapse.
*
*	*A transfer can be up to 64 bytes (Full speed), this does not mean 1 byte per n microseconds.
*
*	*If data loss is happening, increase the delay.
*	*Unfortunately the data loss is driver related,
*	*and is not able to be fixed from here.
*/
#define CDC_MIN_TX_INTERVAL 0

/*
*	Changes the way transfers are handled, and the methods available to transfer data.
*
* \ true	cdc_stdio_write() will wait for transfer to complete.
*
* \ false	cdc_stdio_write() will only wait for all but the last block in transfers which are larger than the TX buffer. (most transfers only have 1 block)
*/
#define CDC_WAIT_FOR_TX_COMPLETE false


/*
*	The maximum amount of loop ticks to happen until a transfer has been considered to have timed out.
*/
#define CDC_TX_TIMEOUT 1000

/*
*	The maximum amount times a write can have timed out, and have attempted to resend.
*
*	If 0, cdc_stdio_write will not retry on timeout.
*	If -1, both cdc_stdio_write and cdc_tx_ready_timeout will not retry on timeout.
*/
#define CDC_TX_MAX_RETRIES 5


#endif /* USB_CDC_STDIO_CONFIG_H_ */
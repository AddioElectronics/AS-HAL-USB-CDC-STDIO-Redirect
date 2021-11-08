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
*
*	*Does not use USB_CDC_RX_BUF_SIZE, as even if USB_CDC_RX_BUF_SIZE was increased when using regular buffer,
*	it will only ever fill up to the size of a USB packet.
*/
#define CDC_SECONDARY_BUFFER_SIZE 64

/*
*	Define buffer sizes.
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
	#define USB_CDC_TX_BUF_SIZE CONF_USB_CDCD_ACM_DATA_BULKOUT_MAXPKSZ -1	//Max is 64. Apparently windows 10 driver is more reliable when set to 63.
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
*	Delay after transfer complete (ms)
*
*	*If data loss is happening, increase the delay.
*	*Unfortunately the data loss is driver related,
*	*and is not able to be fixed from here.
*
*	*I have debugged with wire shark and all the bytes are getting to PC,
*	even though my terminal was missing the characters.
*/
#define CDC_TRANSFER_DELAY 0

/*
*	Changes the way transfers are handled, and the methods available to transfer data.
*
* \ true	cdc_stdio_write() will wait for transfer to complete, and retry to send if timeout limit is reached.
*			In some instances this will run slower, but data has lower chance of being lost.
*
* \ false	cdc_stdio_write() will only wait for all but the last block in transfers which are larger than the TX buffer. (most transfers only have 1 block)
*			cdc_tx_ready(bool retry) is added to avoid collisions, and to add the ability to resend a stuck transfer.
*			In some instances this will run a bit faster, but has a higher chance of losing data.
*			*Writing data, especially with printf, in a continuous loop with no delay, has a high chance of losing data.
*/
#define CDC_TX_RETRY false

/*
*	The maximum amount of loop ticks to happen until a transfer has been considered to have timed out.
*/
#define CDC_TX_TIMEOUT 10000
#define CDC_TX_TIMEOUT_TYPE uint32_t

/*
*	The maximum amount times a write can have timed out, and have attempted to resend.
*/
#define CDC_TX_MAX_RETRIES 1000
#define CDC_TX_RETRY_TYPE uint16_t



#endif /* USB_CDC_STDIO_CONFIG_H_ */
#ifndef USB_CDC_STDIO_DEFS_H_
#define USB_CDC_STDIO_DEFS_H_

#include <stdint.h>

/*
*	Used to determine which callback is being registered.
*/
enum usb_cdc_cb_type 
{ 
	USB_CDC_RX_DATA,		//USB received data, and is ready to read.
	USB_CDC_TX_COMPLETE,	//USB has finished transferring the most recent block.
	USB_CDC_BLOCK_TX,		//Block sent to USB and is waiting to be transferred.
	USB_CDC_ALL_BLOCKS_TX	//Last block has been sent to USB and is waiting to be transferred.
};

/* 
 *	Callback that is invoked when data has been received, and is waiting to be copied from its buffer.
 *  
 *  \param[in] length	The count of bytes received.
 */
typedef void (*cdc_rx_data)(const uint16_t length);

/* 
 *	Callback that is invoked when the last transfer has completed.
 *  
 *  \param[in] length	The count of bytes sent.
 */
typedef void (*cdc_tx_complete)(const uint16_t length);

/* 
 *	Callback that is invoked when a block of data has been sent to USB.
 *  
 *  \param[in] length	The count of bytes sent.
 */
typedef void (*cdc_block_tx)(const uint16_t length);

/* 
 *	Callback that is invoked when the last block of a write has been sent to USB.
 *  
 *  \param[in] length	The count of bytes sent.
 */
typedef void (*cdc_all_blocks_tx)(const size_t length);


/*
*	The structure definition which contains callbacks for usb_cdc.
*/
struct usb_cdc_callbacks
{
	cdc_rx_data				dataReceieved;		//Data has been received, and is ready to be read from buffer.
	cdc_tx_complete			txComplete;			//USB has finished transferring a block
	cdc_block_tx			blockTx;			//A block (Partial write) has been sent to USB
	cdc_all_blocks_tx		allBlocksTx;		//All blocks have been sent to USB
};


#endif /* USB_CDC_STDIO_DEFS_H_ */
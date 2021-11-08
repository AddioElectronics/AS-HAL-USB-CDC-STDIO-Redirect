#ifndef USB_CDC_STDIO_H
#define USB_CDC_STDIO_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include <usbd_config.h>
#include <usb_start.h>
#include <stdio_io.h>

#include "usb_cdc_stdio_config.h"
#include "usb_cdc_stdio_defs.h"



#pragma region Variables

/*
*	The IO descriptor which holds the pointers to the read/write functions for the STDIO redirect.
*	\Exists in usb_cdc_stdio.c
*/
extern struct io_descriptor USB_CDC_IO;

#pragma endregion Variables

#pragma region Initialization Functions

/*
 *	Initialize redirect of stdio to USB CDC.
 */
void cdc_stdio_init();

/*
 *	Register a callback function.
 *
 * \param[in] cb_type	The callback type.
 * \param[in] func		The callback function.
 *
 * \return Exit code.
 */
int32_t usb_cdc_stdio_register_callback(enum usb_cdc_cb_type cb_type, FUNC_PTR func);


#pragma endregion Initialization Functions

#pragma region Status Functions


/*
*	Includes functions used to recover from a stuck transfer.
*	Not needed when CDC_TX_RETRY == true, as transfers wait to be sent.
*/
//#if CDC_TX_RETRY_ == false

/*
* Has the last transmit finished?
* \param[in] retry When tx_attempts == CDC_TX_MAX_RETRIES, call cdc_retry_last_tx()
*/
bool cdc_tx_ready(bool retry);

/*
*	Has the last write to the TX buffer been transmitted?
*	Will wait for transfer or to reach the timeout.
*	When tx_attempts reaches CDC_TX_MAX_RETRIES, it will either give up on the transfer,
*	or resend the transfer.
* \param[in] retry	Should the last transfer be resent if it reaches a timeout state?
*/
bool cdc_tx_ready_wait(bool retry);

/*
 * Attempt to resend the last write, if it was not transfered.
 *
 * \return	True if the last transfer was attempted to be resent.
			False if there were no transfers in a stuck state.
 */
bool cdc_retry_last_tx();

//#endif

/*
 *	Have any new bytes been received?
 *
 * \return If there are new bytes in the cdc_rx_buffer.
 */
bool cdc_rx_ready(void);

/*
 *	Get the count of bytes received.
 *
 * \return The number of new bytes in the cdc_rx_buffer
 */
uint16_t cdc_get_rx_size();


#pragma endregion Status Functions


#pragma region Functions

void cdc_enable_tx_wait_to_fill();

void cdc_disable_tx_wait_to_fill();


#pragma endregion Functions

#endif
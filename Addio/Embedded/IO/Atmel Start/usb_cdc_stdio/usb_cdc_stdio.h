#ifndef USB_CDC_STDIO_H
#define USB_CDC_STDIO_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include <usbd_config.h>
#include <usb_start.h>

#if __has_include("../../stdio_redirect/addio_stdio_io.h")
#include "../../stdio_redirect/addio_stdio_io.h"
#else
#include <stdio_io.h>
#endif

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
*	If tx_wait_to_fill is enabled, this will return the amount of bytes still free in the buffer.
*/
size_t cdc_tx_capacity();

/*
*	Checks to see if the DTR signal is detected. If it is we can deem that we are connected.
*/
bool cdc_data_terminal_ready();

/*
* Has the last transmit finished?
*
* \return True if there are no active transfer. False if the last transfer has not been sent.
*/
bool cdc_tx_ready();

bool cdc_tx_ready_timeout(bool retry);
bool cdc_retry_last_tx();

/*
 *	Have any new bytes been received?
 *
 * \return If there are new bytes in the cdc_rx_buffer.
 */
bool cdc_rx_ready();

/*
 *	Get the count of bytes received.
 *
 * \return The number of new bytes in the cdc_rx_buffer
 */
uint16_t cdc_get_rx_length();


#pragma endregion Status Functions


#pragma region Functions

void cdc_get_io_descriptor(struct io_descriptor **io);

bool cdc_set_tx_hold_buffer(bool wait);

void cdc_disable_tx_wait_to_fill();


#pragma endregion Functions

#endif
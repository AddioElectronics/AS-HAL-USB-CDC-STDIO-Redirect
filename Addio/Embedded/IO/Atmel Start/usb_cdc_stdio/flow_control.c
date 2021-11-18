#include "flow_control.h"

#include <usbdc.h>
#include <usb_protocol_cdc.h>
#include <cdcdf_acm.h>

//
////Class notification
//typedef struct usb_cdc_notify_msg {
	//uint8_t bmRequestType;
	//uint8_t bNotification;
	//union {
		//le16_t wValue;
		//struct {
			//uint8_t low;
			//uint8_t high;
		//} wValueBytes;
	//};
	//union {
		//le16_t wIndex;
		//struct {
			//uint8_t low;
			//uint8_t high;
		//} wIndexBytes;
	//};
	//union {
		//le16_t wLength;
		//struct {
			//uint8_t low;
			//uint8_t high;
		//} wLengthBytes;
	//};
//} usb_cdc_notify_msg_t;////////! UART State Bitmap (cdc spec 1.1 chapter 6.3.5)
//typedef union usb_cdc_uart_state {
	//le16_t value;
	//struct {
		//uint8_t bDCD : 1;
		//uint8_t bDSR : 1;
		//uint8_t bBreak : 1;
		//uint8_t bRingSignal : 1;
		//uint8_t bFraming : 1;
		//uint8_t bParity : 1;
		//uint8_t bOverRun;
	//} rs232;
//} usb_cdc_uart_state_t;//
//
////! Hardware handshake support (cdc spec 1.1 chapter 6.3.5)
//typedef struct usb_cdc_notify_serial_state {
	//usb_cdc_notify_msg_t     header; //Same structure as a "Class Notification"
	//union usb_cdc_uart_state state;
//} usb_cdc_notify_serial_state_t;




usb_cdc_notify_serial_state_t serial_state;
usb_cdc_notify_msg_t call_state_change;




extern struct cdcdf_acm_func_data _cdcdf_acm_funcd;


typedef struct{
	uint8_t reserved;
	uint8_t
	}call_state_change_t;

void test_flow_control_init()
{
	serial_state.header.bmRequestType = 0b10100001;
	serial_state.header.bNotification = USB_REQ_CDC_NOTIFY_SERIAL_STATE; //0x20
	serial_state.header.wLength = 2;
	serial_state.header.wIndex = USB_DT_INTERFACE; //4, not sure if this is correct
	serial_state.header.wValue = 0;	
	
	serial_state.state.value = 0;
	
	call_state_change.bmRequestType = 0b110100001;
	call_state_change.bNotification = USB_REQ_CDC_NOTIFY_CALL_STATE_CHANGE;
	call_state_change.wLength = 0;
	call_state_change.wIndex = USB_DT_INTERFACE; //4, again not sure
	
}

void test_send_serial_state(bool dsr, bool dcd, bool breakstate)
{
	serial_state.state.rs232.bDSR = dsr;	
	serial_state.state.rs232.bDCD = dcd;
	serial_state.state.rs232.bBreak = breakstate;	
	
	
	//endpoint 0x82
	return usbdc_xfer(_cdcdf_acm_funcd.func_ep_in[0], &serial_state, sizeof(usb_cdc_notify_serial_state_t), false);
	
	
	//Tried this without sending "usb_cdc_uart_state_t" and same result.
	//header.wValue = serial_state.state.value;	
	//endpoint 0x82
	//return usbdc_xfer(_cdcdf_acm_funcd.func_ep_in[0], &serial_state.header, sizeof(usb_cdc_notify_msg_t), false);
}

//Call state change index
#define CSC_IDLE			0x01
#define CSC_DIALING			0x02
#define CSC_RINGBACK		0x03
#define CSC_CONNECTED		0x04
	#define CSC_CONNECTED_VOICECON			0x00
	#define CSC_CONNECTED_ANSWERINGMACHINE	0x01
	#define CSC_CONNECTED_FAXGMACHINE		0x02
	#define CSC_CONNECTED_DATAMODEM			0x03
	#define CSC_CONNECTED_UNKNOWN			0xFF

#define CSC_INCOMING_CALL	0x05

void test_send_call_state_change(bool idle)
{
	if(idle)
	{
		call_state_change.wValueBytes.high = CSC_IDLE;
		call_state_change.wValueBytes.low = 0;
	}
	else
	{
		call_state_change.wValueBytes.high = CSC_CONNECTED;
		call_state_change.wValueBytes.low = CSC_CONNECTED_DATAMODEM;
	}


	return usbdc_xfer(_cdcdf_acm_funcd.func_ep_in[0], &call_state_change, sizeof(usb_cdc_notify_msg_t), false);
}
/**
  * @file    tp_nbiot_interface.cpp
  * @version 0.1.0
  * @author  Adam Mitchell
  * @brief   C++ file of the Thingpilot NB-IoT interface. This interface is hardware agnostic
  *          and depends on the underlying modem drivers exposing an identical interface
  */

/** Includes
 */
#include "tp_nbiot_interface.h"


#if defined (BOARD) && (BOARD == WRIGHT_V1_0_0 || BOARD == DEVELOPMENT_BOARD_V1_1_0)
/** Constructor for the TP_NBIoT_Interface class, specifically when 
 *  using a ublox Sara N2xx. Instantiates an ATCmdParser object
 *  on the heap for comms between microcontroller and modem
 * 
 * @param txu Pin connected to SaraN2 TXD (This is MCU TXU)
 * @param rxu Pin connected to SaraN2 RXD (This is MCU RXU)
 * @param cts Pin connected to SaraN2 CTS
 * @param rst Pin connected to SaraN2 RST
 * @param vint Pin conencted to SaraN2 VINT
 * @param gpio Pin connected to SaraN2 GPIO1
 * @param baud Baud rate for UART between MCU and SaraN2
 */  
TP_NBIoT_Interface::TP_NBIoT_Interface(PinName txu, PinName rxu, PinName cts, PinName rst, 
					       			   PinName vint, PinName gpio, int baud) :
                                       _modem(txu, rxu, cts, rst, vint, gpio, baud) 
{

}
#endif /* #if defined (BOARD) && (BOARD == ...) */

/** Destructor for the TP_NBIoT_Interface class
 */
TP_NBIoT_Interface::~TP_NBIoT_Interface()
{
	#if defined (_COMMS_NBIOT_DRIVER) && (_COMMS_NBIOT_DRIVER == SARAN2)
    _modem.~SaraN2();
    #endif /* #if defined (_COMMS_NBIOT_DRIVER) && (_COMMS_NBIOT_DRIVER == SARAN2) */
}

/** Configure CoAP profile 0 with a given IP address, port and URI
 *
 * @param *ipv4 Pointer to a byte array storing the IPv4 address of the 
 *              destination server as a string, for example:
 *              char ipv4[] = "168.134.102.18"; 
 * @param port Destination server port
 * @param *uri Pointer to a byte array storing the URI, for example:
 *             char uri[] = "http://coap.me:5683/sink";
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::configure_coap(char *ipv4, uint16_t port, char *uri)
{
	int status = -1;

	status = _modem.select_profile(SaraN2::COAP_PROFILE_0);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.set_coap_ip_port(ipv4, port);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.set_coap_uri(uri);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.pdu_header_add_uri_path();
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.set_profile_validity(SaraN2::PROFILE_VALID);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.save_profile(SaraN2::COAP_PROFILE_0);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	return TP_NBIoT_Interface::NBIOT_OK;
}


int TP_NBIoT_Interface::coap_get(char *recv_data)
{
	int status = -1;

	status = _modem.load_profile(SaraN2::COAP_PROFILE_0);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.select_coap_at_interface();
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.coap_get(recv_data);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	return TP_NBIoT_Interface::NBIOT_OK;
}


int TP_NBIoT_Interface::coap_delete()
{
	int status = -1;

	status = _modem.load_profile(SaraN2::COAP_PROFILE_0);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.select_coap_at_interface();
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.coap_delete(recv_data);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	return TP_NBIoT_Interface::NBIOT_OK;
}


int TP_NBIoT_Interface::coap_put(char *send_data, char *recv_data, int data_indentifier)
{
	int status = -1;

	status = _modem.load_profile(SaraN2::COAP_PROFILE_0);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.select_coap_at_interface();
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.coap_put(send_data, recv_data, data_indentifier);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	return TP_NBIoT_Interface::NBIOT_OK;
}


int TP_NBIoT_Interface::coap_post(char *send_data, char *recv_data, int data_indentifier)
{
	int status = -1;

	status = _modem.load_profile(SaraN2::COAP_PROFILE_0);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.select_coap_at_interface();
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	status = _modem.coap_post(send_data, recv_data, data_indentifier);
	if(status != TP_NBIoT_Interface::NBIOT_OK)
	{
		return status;
	}

	return TP_NBIoT_Interface::NBIOT_OK;
}


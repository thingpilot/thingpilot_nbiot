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
TP_NBIoT_Interface::TP_NBIoT_Interface(PinName txu, PinName rxu, PinName cts, PinName rst, 
					       			   PinName vint, PinName gpio, int baud) :
                                       _modem(txu, rxu, cts, rst, vint, gpio, baud) 
{

}
#endif /* #if defined (BOARD) && (BOARD == ...) */


TP_NBIoT_Interface::~TP_NBIoT_Interface()
{
	#if defined (_COMMS_NBIOT_DRIVER) && (_COMMS_NBIOT_DRIVER == SARAN2)
    _modem.~SaraN2();
    #endif /* #if defined (_COMMS_NBIOT_DRIVER) && (_COMMS_NBIOT_DRIVER == SARAN2) */
}


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


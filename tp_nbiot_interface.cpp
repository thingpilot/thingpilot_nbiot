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

/** Power-cycle the NB-IoT modem
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::reboot_modem()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.reboot_module();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Query UE for radio connection and network registration status
 * 
 * @param &connected Address of integer in which to store radio 
 *                   connection status
 * @param &reg_status Address of integer in which to store
 *                    network registration status
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::get_connection_status(int &connected, int &reg_status)
{
    int status = -1;

    if(_driver == TP_NBIoT_Interface::SARAN2)
    {
        int urc;

        status = _modem.cscon(urc, connected);
        if(status != TP_NBIoT_Interface::NBIOT_OK)
        {
            return status;
        }

        status = _modem.cereg(urc, reg_status);
        if(status != TP_NBIoT_Interface::NBIOT_OK)
        {
            return status;
        }

        return TP_NBIoT_Interface::NBIOT_OK;
    }

    return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Return operation stats, of a given type, of the module
 * 
 * @param *data Point to .data parameter of Nuestats_t struct
 *              to copy data into
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::get_nuestats(char *data)
{
    int status = -1;

    if(_driver == TP_NBIoT_Interface::SARAN2)
    {
        status = _modem.nuestats(data);
        if(status != TP_NBIoT_Interface::NBIOT_OK)
        {
            return status;
        }

        return TP_NBIoT_Interface::NBIOT_OK;
    }

    return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Allow the platform to automatically attempt to connect to the 
 *  network after power-on or reboot. Will set AT+CFUN=1 and read
 *  the SIM PLMN. Will use APN provided by network.
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::enable_autoconnect()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::AUTOCONNECT, SaraN2::TRUE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Disable previously described autoconnect functionality
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::disable_autoconnect()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::AUTOCONNECT, SaraN2::FALSE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Enable CR_0354_0338 scrambling. This is an operator specific 
 *  setting so please confirm with your mobile network provider
 *  if you're unsure as to what to set this to
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::enable_scrambling()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::SCRAMBLING, SaraN2::TRUE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Disable previously described CR_0354_0338 scrambling functionality
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::disable_scrambling()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::SCRAMBLING, SaraN2::FALSE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Enable the scheduling of conflicted NSIB. This is an operator 
 *  specific setting so please confirm with your mobile network 
 *  provider if you're unsure as to what to set this to
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::enable_si_avoid()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::SI_AVOID, SaraN2::TRUE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Disable the previously described scheduling of conflicted NSIB
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::disable_si_avoid()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::SI_AVOID, SaraN2::FALSE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Enable combined EPS/IMSI network attach
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::enable_combine_attach()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::COMBINE_ATTACH, SaraN2::TRUE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Disable combined EPS/IMSI network attach
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::disable_combine_attach()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::COMBINE_ATTACH, SaraN2::FALSE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Enable RRC cell reselection
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::enable_cell_reselection()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::CELL_RESELECTION, SaraN2::TRUE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Disable RRC cell reselection
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::disable_cell_reselection()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::CELL_RESELECTION, SaraN2::FALSE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Enable Bearer Independent Protocol (BIP) where BIP is the interface
 *  between the SIM and the ME which provides access to the data bearers
 *  supported by the ME
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::enable_bip()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::ENABLE_BIP, SaraN2::TRUE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Disable previously described BIP functionality
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::disable_bip()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::ENABLE_BIP, SaraN2::FALSE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** When enabled, the SIM is only powered when it is accessed and will be 
 *  un-powered when not required, i.e. when in PSM
 *
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::enable_power_save_mode()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::NAS_SIM_PSM_ENABLE, SaraN2::TRUE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Disable previously described SIM PSM
 *
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::disable_power_save_mode()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.configure_ue(SaraN2::NAS_SIM_PSM_ENABLE, SaraN2::FALSE);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Configure CoAP profile 0 with a given IP address, port and URI
 *
 * @param *ipv4 Pointer to a byte array storing the IPv4 address of the 
 *              destination server as a string, for example:
 *              char ipv4[] = "168.134.102.18"; 
 * @param port Destination server port
 * @param *uri Pointer to a byte array storing the URI, for example:
 *             char uri[] = "http://coap.me:5683/sink";
 * @param uri_length Number of characters in URI, cannot be greater
 *                   than 200
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::configure_coap(char *ipv4, uint16_t port, char *uri, uint8_t uri_length)
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
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

		status = _modem.set_coap_uri(uri, uri_length);
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

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Perform a HTTP GET request over CoAP and capture the server
 *  response in recv_data
 *
 * @param *recv_data Pointer to a byte array that will be populated
 *              	 with the response from the server
 * @param &response_code Address of integer where CoAP operation response code
 *                       will be stored
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::coap_get(char *recv_data, int &response_code)
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
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

		status = _modem.coap_get(recv_data, response_code);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}
	
	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Perform a HTTP DELETE request over CoAP and capture the server
 *  response in recv_data
 *
 * @param *recv_data Pointer to a byte array that will be populated
 *              	 with the response from the server
 * @param &response_code Address of integer where CoAP operation response code
 *                       will be stored
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::coap_delete(char *recv_data, int &response_code)
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
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

		status = _modem.coap_delete(recv_data, response_code);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}
	
	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Perform a PUT request using CoAP and save the returned 
 *  data into recv_data
 * 
 * @param *send_data Pointer to a byte array containing the 
 *                   data to be sent to the server
 * @param *recv_data Pointer to a byte array where the data 
 *                   returned from the server will be stored
 * @param data_intenfier Integer value representing the data 
 *                       format type. Possible values are enumerated
 *                       in the driver header file, i.e. TEXT_PLAIN
 * @param &response_code Address of integer where CoAP operation response code
 *                       will be stored
 * @return Indicates success or failure reason
 */ 
int TP_NBIoT_Interface::coap_put(char *send_data, char *recv_data, int data_indentifier, int &response_code)
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
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

		status = _modem.coap_put(send_data, recv_data, data_indentifier, response_code);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Perform a POST request using CoAP and save the returned 
 *  data into recv_data
 * 
 * @param *send_data Pointer to a byte array containing the 
 *                   data to be sent to the server
 * @param *recv_data Pointer to a byte array where the data 
 *                   returned from the server will be stored
 * @param data_intenfier Integer value representing the data 
 *                       format type. Possible values are enumerated
 *                       in the driver header file, i.e. SaraN2::TEXT_PLAIN
 * @param &response_code Address of integer where CoAP operation response code
 *                       will be stored
 * @return Indicates success or failure reason
 */ 
int TP_NBIoT_Interface::coap_post(char *send_data, char *recv_data, int data_indentifier, int &response_code)
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
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

		status = _modem.coap_post(send_data, recv_data, data_indentifier, response_code);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}
	
	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}


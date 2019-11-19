/**
  * @file    tp_nbiot_interface.h
  * @version 0.3.1
  * @author  Adam Mitchell
  * @brief   Header file of the Thingpilot NB-IoT interface. This interface is hardware agnostic
  *          and depends on the underlying modem drivers exposing an identical interface
  */

/** Define to prevent recursive inclusion
 */
#pragma once

/** Includes 
 */
#include <mbed.h>

/** NB-IoT #defines 
 */
#define EARFCN_B8_LOW   3450
#define EARFCN_B8_HIGH  3799
#define EARFCN_B20_LOW  6150
#define EARFCN_B20_HIGH 6449


#if defined (BOARD) && (BOARD == WRIGHT_V1_0_0)

#include "SaraN2Driver.h"

#endif /* #if defined (BOARD) && (BOARD == WRIGHT_V1_0_0) */

/** Base class for the Thingpilot NB-IoT interface
 */
class TP_NBIoT_Interface
{

	public:

		/** Possible _driver values
		 */
		enum
		{
			UNDEFINED = 0,
			SARAN2    = 1
		};

		/** Function return codes
		 */
		enum
		{
			NBIOT_OK           = 0,
			DRIVER_UNKNOWN     = 60,
			EXCEEDS_MAX_VALUE  = 61,
			INVALID_UNIT_VALUE = 62,
			FAIL_TO_CONNECT    = 63
		};

		/** LTE Bands
		 */
		enum class TP_NBIoT_Band
		{
			BAND_8  	 = 0,
			BAND_20 	 = 1,
			BAND_UNKNOWN = 2
		};

		/** Connection status matrix values according to u-blox NB-IoT
		 *  application development guide:
		 *  https://www.u-blox.com/sites/default/files/SARA-N2-Application-Development_AppNote_%28UBX-16017368%29.pdf
		 */
		enum class TP_Connection_Status
		{
			ACTIVE_NO_NETWORK_ACTIVITY       = 0,
			ACTIVE_SCANNING_FOR_BASE_STATION = 1,
			ACTIVE_STARTING_REGISTRATION     = 2,
			ACTIVE_REGISTERED_RRC_CONNECTED  = 3,
			ACTIVE_REGISTERED_RRC_RELEASED   = 4,
			PSM_REGISTERED                   = 5,
			REGISTRATION_FAILED              = 6,
			STATE_UNDEFINED                  = 7
		};

		/** List of possible T3412 timer units
		 */
		enum class T3412_units
		{
			HR_320  = 0, // 1 1 0
			HR_10   = 1, // 0 1 0
			HR_1    = 2, // 0 0 1
			MIN_10  = 3, // 0 0 0 
			MIN_1   = 4, // 1 0 1
			SEC_30  = 5, // 1 0 0 
			SEC_2   = 6, // 0 1 1 
			DEACT   = 7, // 1 1 1 
            INVALID = 8
		};

		/** List of possible T3324 timer units
		 */
		enum class T3324_units
		{
			MIN_6   = 0, // 0 1 0
			MIN_1   = 1, // 0 0 1
			SEC_2   = 2, // 0 0 0 
			DEACT   = 3, // 1 1 1 
            INVALID = 4
		};

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
		TP_NBIoT_Interface(PinName txu, PinName rxu, PinName cts, PinName rst, 
                           PinName vint, PinName gpio, int baud = 57600);

		#endif /* #if defined (BOARD) && (BOARD == ...) */

        /** Destructor for the TP_NBIoT_Interface class
         */
		~TP_NBIoT_Interface();

        /** Determine when the modem is ready to recieve AT commands
         * or timeout if it's unresponsive for longer than timeout_s
         *
         * @param timeout_s Timeout period in seconds
         * @return Indicates success or failure reason  
         */
        int ready(uint8_t timeout_s = 10);

		/** Initialise the modem with default parameters:
		 *  AUTOCONNECT = TRUE
		 *  CELL_RESELECTION = TRUE
		 *  SIM_PSM = TRUE
		 *  MODULE_PSM = TRUE
		 * 
		 *  Then attempt to connect to a network for 5 minutes; if this is 
		 *  unsuccessful then turn off the modem and report that status
		 *  back to the application. If it is successful then the modem 
		 *  may not necessarily enter PSM instantly - this is determined by
		 *  T3324/T3412 timer settings
		 * 
         * @param timeout_s Timeout period in seconds
		 * @return Inidicates success or failure reason
		 */
		int start(uint16_t timeout_s = 300);

		/** Power-cycle the NB-IoT modem
		 * 
		 * @return Indicates success or failure reason
		 */
		int reboot_modem();

		/** Disable TX and RX RF circuits
		 * 
		 * @return Indicates success or failure reason
		 */
        int deactivate_radio();

		/** Enable TX and RX RF circuits
		 * 
		 * @return Indicates success or failure reason
		 */
        int activate_radio();

		/** Attempt to attach to network GPRS service
		 *
		 * @return Indicates success or failure reason
		 */
        int gprs_attach();

		/** Attempt to detach from network GPRS service
		 *
		 * @return Indicates success or failure reason
		 */
        int gprs_detach();

		/** Attempt to automatically register to network
		 *  using SIM card home PLMN
		 *
		 * @return Indicates success or failure reason
		 */
        int auto_register_to_network();

		/** Deregister from network
		 * 
		 * @return Indicates success or failure reason
		 */
        int deregister_from_network();

		/** Enable entire module Power Save Mode (PSM)
		 * 
		 * @return Indicates success or failure reason
		 */
		int enable_power_save_mode();

		/** Disable entire module Power Save Mode (PSM)
		 * 
		 * @return Indicates success or failure reason
		 */
		int disable_power_save_mode();

		/** Return u-blox defined connection status based on radio connection status, 
		 *  network registration status and PSM status. This document can be found in 8.4
		 *  at https://www.u-blox.com/sites/default/files/SARA-N2-Application-Development_AppNote_%28UBX-16017368%29.pdf
		 * 
		 * @param &status Address of TP_Connection_Status to return u-blox defined connection
		 *                status to
		 * @param &connected Address of integer value in which to store radio connection status
		 *                   where 1 = connected and 0 is not connected
		 * @param &registered Address of integer value in which to store network registration
		 *                    status. See AT+CEREG=0/AT+CEREG? for possible values
		 * @param &psm Address of integer value in which to store PSM status where 1 = in PSM
		 *             and 0 is in Active mode
		 * @return Indicates success or failure reason
		 */ 
		int get_module_network_status(TP_Connection_Status &status, int &connected, 
									  int &registered, int &psm);

		/** Query UE for radio connection and network registration status
		 * 
		 * @param &connected Address of integer in which to store radio 
		 *                   connection status
		 * @param &reg_status Address of integer in which to store
		 *                    network registration status
		 * @return Indicates success or failure reason
		 */
        int get_connection_status(int &connected, int &reg_status);

        /** Get last known RSRP and RSRQ
        * 
        * @param &power Address of integer in which to return
        *               last known RSRP
        * @param &quality Address of integer in which to return
        *                 last known RSRQ
        * @return Indicates success or failure reason
        */
        int get_csq(int &power, int &quality);

		/** Return LTE channel number, EARFCN
		 * 
		 * @param &band Address of TP_NBIoT_Band value in which to store
		 *              determined EARFCN
		 * @return Indicates success or failure reason
		 */
		int get_band(TP_NBIoT_Band &band);

		/** Return operation stats, of a given type, of the module
         * 
         * @param *data Point to .data parameter of Nuestats_t struct
         *              to copy data into
         * @return Indicates success or failure reason
         */
        int get_nuestats(char *data);

		/** Allow the platform to automatically attempt to connect to the 
		 *  network after power-on or reboot. Will set AT+CFUN=1 and read
		 *  the SIM PLMN. Will use APN provided by network.
		 * 
		 * @return Indicates success or failure reason
		 */
		int enable_autoconnect();

		/** Disable previously described autoconnect functionality
		 * 
		 * @return Indicates success or failure reason
		 */
		int disable_autoconnect();

		/** Enable CR_0354_0338 scrambling. This is an operator specific 
		 *  setting so please confirm with your mobile network provider
		 *  if you're unsure as to what to set this to
		 * 
		 * @return Indicates success or failure reason
		 */
		int enable_scrambling();

		/** Disable previously described CR_0354_0338 scrambling functionality
		 * 
		 * @return Indicates success or failure reason
		 */
		int disable_scrambling();

		/** Enable the scheduling of conflicted NSIB. This is an operator 
		 *  specific setting so please confirm with your mobile network 
		 *  provider if you're unsure as to what to set this to
		 * 
		 * @return Indicates success or failure reason
		 */
		int enable_si_avoid();

		/** Disable the previously described scheduling of conflicted NSIB
		 * 
		 * @return Indicates success or failure reason
		 */
		int disable_si_avoid();

		/** Enable combined EPS/IMSI network attach
		 * 
		 * @return Indicates success or failure reason
		 */
		int enable_combine_attach();

		/** Disable combined EPS/IMSI network attach
		 * 
		 * @return Indicates success or failure reason
		 */
		int disable_combine_attach();

		/** Enable RRC cell reselection
		 * 
		 * @return Indicates success or failure reason
		 */
		int enable_cell_reselection();

		/** Disable RRC cell reselection
		 * 
		 * @return Indicates success or failure reason
		 */
		int disable_cell_reselection();

		/** Enable Bearer Independent Protocol (BIP) where BIP is the interface
		 *  between the SIM and the ME which provides access to the data bearers
		 *  supported by the ME
		 * 
		 * @return Indicates success or failure reason
		 */
		int enable_bip();

		/** Disable previously described BIP functionality
		 * 
		 * @return Indicates success or failure reason
		 */
		int disable_bip();

		/** When enabled, the SIM is only powered when it is accessed and will be 
		 *  un-powered when not required, i.e. when in PSM
		 *
		 * @return Indicates success or failure reason
		 */
		int enable_sim_power_save_mode();

		/** Disable previously described SIM PSM
		 *
		 * @return Indicates success or failure reason
		 */
		int disable_sim_power_save_mode();

        /** Query whether or not Power Save Mode (PSM) is enabled
		 *  
		 * @param &power_save_mode Address of integer in which to store
		 *                         value of power save mode setting. 1 
		 *                         means that PSM is enabled, 0 means 
		 *                         that PSM is disable
		 * @return Indicates success or failure reason
		 */
        int query_power_save_mode(int &power_save_mode);

		/** Determine whether or not the modem is in power save mode or not
		 * 
		 * @param &psm Address of integer in which to store actual PSM value,
		 *             1 = in PSM, 0 = active
		 * @return Indicates success or failure reason
		 */ 
		int get_power_save_mode_status(int &psm);

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
		int configure_coap(char *ipv4, uint16_t port, char *uri, uint8_t uri_length);

		/** Perform a HTTP GET request over CoAP and capture the server
		 *  response in recv_data
		 *
		 * @param *recv_data Pointer to a byte array that will be populated
		 *              	 with the response from the server
         * @param &response_code Address of integer where CoAP operation response code
         *                       will be stored
		 * @return Indicates success or failure reason
		 */
		int coap_get(char *recv_data, int &response_code);

		/** Perform a HTTP DELETE request over CoAP and capture the server
		 *  response in recv_data
		 *
		 * @param *recv_data Pointer to a byte array that will be populated
		 *              	 with the response from the server
         * @param &response_code Address of integer where CoAP operation response code
         *                       will be stored
		 * @return Indicates success or failure reason
		 */
		int coap_delete(char *recv_data, int &response_code);

		/** Perform a PUT request using CoAP and save the returned 
		 *  data into recv_data
		 * 
		 * @param *send_data Pointer to a byte array containing the 
		 *                   data to be sent to the server
		 * @param *recv_data Pointer to a byte array where the data 
		 *                   returned from the server will be stored
		 * @param data_intenfier Integer value representing the data 
		 *                       format type. Possible values are enumerated
		 *                       in the header file, i.e. TEXT_PLAIN
         * @param &response_code Address of integer where CoAP operation response code
         *                       will be stored
		 * @return Indicates success or failure reason
		 */ 
		int coap_put(char *send_data, char *recv_data, int data_indentifier, int &response_code);

		/** Perform a POST request using CoAP and save the returned 
		 *  data into recv_data
		 * 
		 * @param *send_data Pointer to a byte array containing the 
		 *                   data to be sent to the server
		 * @param *recv_data Pointer to a byte array where the data 
		 *                   returned from the server will be stored
		 * @param data_intenfier Integer value representing the data 
		 *                       format type. Possible values are enumerated
		 *                       in the header file, i.e. TEXT_PLAIN
         * @param &response_code Address of integer where CoAP operation response code
         *                       will be stored
		 * @return Indicates success or failure reason
		 */ 
		int coap_post(char *send_data, char *recv_data, int data_indentifier, int &response_code);

		/** Set T3412 timer to multiples of given units
		 * 
		 * @param unit Enumerated value within T3412_units enum class
		 * @param multiples Value no greater than 31 that determines
		 *                  how many multiples of unit to set the
		 *                  timer to
		 * @return Indicates success or failure reason
		 */
		int set_tau_timer(T3412_units unit, uint8_t multiples);

		/** Retrieve T3412 timer value as binary string
		 * 
		 * @param *timer Pointer to char array in which to store
		 *               timer value as binary string
		 * @return Indicates success or failure reason
		 */
		int get_tau_timer(char *timer);

		/** Retrieve T3412 timer value as units and multiples
		 * 
		 * @param &unit Address of T3412_units value into which
		 *              the determined timer unit will be stored
		 * @param &multiples Address of uint8_t into which 
		 *                   the determined multiples value will be 
		 *                   stored
		 * @return Indicates success or failure reason
		 */
		int get_tau_timer(T3412_units &unit, uint8_t &multiples);

		/** Set T3324 timer to multiples of given units
		 * 
		 * @param unit Enumerated value within T3324_units enum class
		 * @param multiples Value no greater than 31 that determines
		 *                  how many multiples of unit to set the
		 *                  timer to
		 * @return Indicates success or failure reason
		 */
		int set_active_time(T3324_units unit, uint8_t multiples);

		/** Retrieve T3324 timer value as binary string
		 * 
		 * @param *timer Pointer to char array in which to store
		 *               timer value as binary string
		 * @return Indicates success or failure reason
		 */
		int get_active_time(char *timer);

		/** Retrieve T3324 timer value as units and multiples
		 * 
		 * @param &unit Address of T3324_units value into which
		 *              the determined timer unit will be stored
		 * @param &multiples Address of uint8_t into which 
		 *                   the determined multiples value will be 
		 *                   stored
		 * @return Indicates success or failure reason
		 */
		int get_active_time(T3324_units &unit, uint8_t &multiples);


	private:

		/** Convert decimal number (with max value of 5-bits) to a binary string,
		 *  i.e. 10 = "01010"
		 * 
		 * @param multiples Decimal value to convert to binary
		 * @param *binary Pointer to a char array to which to return 
		 *                the binary string
		 * @return None
		 */
		void dec_to_bin_5_bit(uint8_t multiples, char *binary);

		#if defined (_COMMS_NBIOT_DRIVER) && (_COMMS_NBIOT_DRIVER == SARAN2)
		SaraN2 _modem;
		#endif /* #if defined (_COMMS_NBIOT_DRIVER) && (_COMMS_NBIOT_DRIVER == SARAN2) */

		#if defined (_COMMS_NBIOT_DRIVER) && (_COMMS_NBIOT_DRIVER == SARAN2)
		int _driver = TP_NBIoT_Interface::SARAN2;
		#else 
		int _driver = TP_NBIoT_Interface::UNDEFINED;
		#endif /* #if defined (_COMMS_NBIOT_DRIVER) && (_COMMS_NBIOT_DRIVER == SARAN2) */
		
};



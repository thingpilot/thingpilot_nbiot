/**
  * @file    tp_nbiot_interface.h
  * @version 0.1.0
  * @author  
  * @brief   Header file of the Thingpilot NB-IoT interface. This interface is hardware agnostic
  *          and depends on the underlying modem drivers exposing an identical interface
  */

/** Define to prevent recursive inclusion
 */
#pragma once

/** Includes 
 */
#include <mbed.h>
#include "board.h"

#if defined (BOARD) && (BOARD == WRIGHT_V1_0_0)

#include "SaraN2Driver.h"

#endif /* #if defined (BOARD) && (BOARD == WRIGHT_V1_0_0) */


class TP_NBIoT_Interface
{

	public:

		enum
		{
			UNDEFINED = 0,
			SARAN2    = 1
		};

		enum
		{
			NBIOT_OK         = 0,
			DRIVER_UNKNOWN   = 40
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
		int configure_coap(char *ipv4, uint16_t port, char *uri);

		/** Perform a HTTP GET request over CoAP and capture the server
		 *  response in recv_data
		 *
		 * @param *recv_data Pointer to a byte array that will be populated
		 *              	 with the response from the server
		 * @return Indicates success or failure reason
		 */
		int coap_get(char *recv_data);

		/** Perform a HTTP DELETE request over CoAP and capture the server
		 *  response in recv_data
		 *
		 * @param *recv_data Pointer to a byte array that will be populated
		 *              	 with the response from the server
		 * @return Indicates success or failure reason
		 */
		int coap_delete(char *recv_data);

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
		 * @return Indicates success or failure reason
		 */ 
		int coap_put(char *send_data, char *recv_data, int data_indentifier);

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
		 * @return Indicates success or failure reason
		 */ 
		int coap_post(char *send_data, char *recv_data, int data_indentifier);


	private:

		#if defined (_COMMS_NBIOT_DRIVER) && (_COMMS_NBIOT_DRIVER == SARAN2)
		SaraN2 _modem;
		#endif /* #if defined (_COMMS_NBIOT_DRIVER) && (_COMMS_NBIOT_DRIVER == SARAN2) */

		#if defined (_COMMS_NBIOT_DRIVER) && (_COMMS_NBIOT_DRIVER == SARAN2)
		int _driver = TP_NBIoT_Interface::SARAN2;
		#else 
		int _driver = TP_NBIoT_Interface::UNDEFINED;
		#endif /* #if defined (_COMMS_NBIOT_DRIVER) && (_COMMS_NBIOT_DRIVER == SARAN2) */
		
};



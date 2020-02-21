/**
  * @file    tp_nbiot_interface.cpp
  * @version 0.4.0
  * @author  Adam Mitchell
  * @brief   C++ file of the Thingpilot NB-IoT interface. This interface is hardware agnostic
  *          and depends on the underlying modem drivers exposing an identical interface
  */

/* Don't build if target != below 
 */
#if BOARD == WRIGHT_V1_0_0 || BOARD == DEVELOPMENT_BOARD_V1_1_0 /* #endif at EoF */

/** Includes
 */
#include "tp_nbiot_interface.h"


#if BOARD == WRIGHT_V1_0_0 || BOARD == DEVELOPMENT_BOARD_V1_1_0
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
#endif /* #if BOARD == ... */

/** Destructor for the TP_NBIoT_Interface class
 */
TP_NBIoT_Interface::~TP_NBIoT_Interface()
{
	#if _COMMS_NBIOT_DRIVER == COMMS_DRIVER_SARAN2
    	_modem.~SaraN2();
    #endif /* #if _COMMS_NBIOT_DRIVER == COMMS_DRIVER_SARAN2 */
}

/** Determine when the modem is ready to recieve AT commands
  * or timeout if it's unresponsive for longer than timeout_s
  *
  * @param timeout_s Timeout period in seconds
  * @return Indicates success or failure reason  
  */
int TP_NBIoT_Interface::ready(uint8_t timeout_s)
{
    int status = -1;

    if(_driver == TP_NBIoT_Interface::SARAN2)
    {
        time_t start_time = time(NULL);

        while(true)
        {
            status = _modem.at();

            if(status == TP_NBIoT_Interface::NBIOT_OK)
            {
                return TP_NBIoT_Interface::NBIOT_OK;
            }

            time_t current_time = time(NULL);
            if(current_time >= start_time + timeout_s)
			{
				return TP_NBIoT_Interface::FAIL_TO_CONNECT;
			}

			ThisThread::sleep_for(500);
        }
    }

    return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

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
int TP_NBIoT_Interface::start(uint16_t timeout_s)
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = enable_autoconnect();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		status = enable_cell_reselection();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		status = enable_sim_power_save_mode();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		status = enable_power_save_mode();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		status = reboot_modem();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		TP_Connection_Status conn_status;
		int connected = 0;
		int registered = 0;
		int psm = 0;
		status = get_module_network_status(conn_status, connected, registered, psm);
		time_t start_time = time(NULL);

		/** Attempt to connect and register to the network for 5 minutes. If we fail
		 *  then turn off the radio to conserve power and let the application decide 
		 *  what to do
		 */
		while(true)
		{
            status = get_module_network_status(conn_status, connected, registered, psm);
            if (conn_status != TP_Connection_Status::ACTIVE_REGISTERED_RRC_CONNECTED ||
		        conn_status != TP_Connection_Status::ACTIVE_REGISTERED_RRC_RELEASED ||
			    conn_status != TP_Connection_Status::PSM_REGISTERED)
              {
                  break;
              }
			debug("\r\nconn_status %d, connected %d, registered %d, psm %d",conn_status, connected, registered, psm);
			time_t current_time = time(NULL);
			if(current_time >= start_time + timeout_s)
			{
				status = deactivate_radio();
				if(status != TP_NBIoT_Interface::NBIOT_OK)
				{
					return status;
				}

				return TP_NBIoT_Interface::FAIL_TO_CONNECT;
			}

			ThisThread::sleep_for(2500);
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
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

/** Is the modem TX/RX circuitry turned on or off? 1 is on, 0 is off
 * 
 * @param &status Address of integer value to which to return the status
 *                value of the radio
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::get_radio_status(int &radio_status)
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.get_radio_status(radio_status);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}	

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Disable TX and RX RF circuits
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::deactivate_radio()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.deactivate_radio();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Enable TX and RX RF circuits
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::activate_radio()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.activate_radio();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Attempt to attach to network GPRS service
 *
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::gprs_attach()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.gprs_attach();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Attempt to detach from network GPRS service
 *
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::gprs_detach()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.gprs_detach();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Attempt to automatically register to network
 *  using SIM card home PLMN
 *
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::auto_register_to_network()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.auto_register_to_network();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Deregister from network
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::deregister_from_network()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.deregister_from_network();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Enable entire module Power Save Mode (PSM)
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::enable_power_save_mode()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.enable_power_save_mode();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Disable entire module Power Save Mode (PSM)
 * 
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::disable_power_save_mode()
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.disable_power_save_mode();
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Query whether or not Power Save Mode (PSM) is enabled
 *  
 * @param &power_save_mode Address of integer in which to store
 *                         value of power save mode setting. 1 
 *                         means that PSM is enabled, 0 means 
 *                         that PSM is disabled
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::query_power_save_mode(int &power_save_mode)
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.query_power_save_mode(power_save_mode);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Determine whether or not the modem is in power save mode or not
 * 
 * @param &psm Address of integer in which to store actual PSM value,
 *             1 = in PSM, 0 = active
 * @return Indicates success or failure reason
 */ 
int TP_NBIoT_Interface::get_power_save_mode_status(int &psm)
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.npsmr(psm);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

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
int TP_NBIoT_Interface::get_module_network_status(TP_Connection_Status &status, int &connected, 
                                                  int &registered, int &psm)
{
	int func_status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		func_status = get_connection_status(connected, registered);
		if(func_status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return func_status;
		}

		func_status = get_power_save_mode_status(psm);
		if(func_status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return func_status;
		}

		if(registered == 0 && connected == 0 && psm == 0)
		{
			status = TP_NBIoT_Interface::TP_Connection_Status::ACTIVE_NO_NETWORK_ACTIVITY;
		}
		else if(registered == 2 && connected == 0 && psm == 0)
		{
			status = TP_NBIoT_Interface::TP_Connection_Status::ACTIVE_SCANNING_FOR_BASE_STATION;
		}
		else if(registered == 2 && connected == 1 && psm == 0)
		{
			status = TP_NBIoT_Interface::TP_Connection_Status::ACTIVE_STARTING_REGISTRATION;
		}
		else if((registered == 1 || registered == 5) && (connected == 1 && psm == 0))
		{
			status = TP_NBIoT_Interface::TP_Connection_Status::ACTIVE_REGISTERED_RRC_CONNECTED;
		}
		else if((registered == 1 || registered == 5) && (connected == 0 && psm == 0))
		{
			status = TP_NBIoT_Interface::TP_Connection_Status::ACTIVE_REGISTERED_RRC_RELEASED;
		}
		else if((registered == 1 || registered == 5) && (connected == 0 && psm == 1))
		{
			status = TP_NBIoT_Interface::TP_Connection_Status::PSM_REGISTERED;
		}
		else if(registered == 3)
		{
			status = TP_NBIoT_Interface::TP_Connection_Status::REGISTRATION_FAILED;
		}
		else
		{
			status = TP_NBIoT_Interface::TP_Connection_Status::STATE_UNDEFINED;
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


/** Get last known RSRP and RSRQ
 * 
 * @param &power Address of integer in which to return
 *               last known RSRP
 * @param &quality Address of integer in which to return
 *                 last known RSRQ
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::get_csq(int &power, int &quality)
{
    int status = -1;

    if(_driver == TP_NBIoT_Interface::SARAN2)
    {
        status = _modem.csq(power, quality);
        if(status != TP_NBIoT_Interface::NBIOT_OK)
        {
            return status;
        }

        return TP_NBIoT_Interface::NBIOT_OK;
    }

    return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Return LTE channel number, EARFCN
 * 
 * @param &band Address of TP_NBIoT_Band value in which to store
 *              determined EARFCN
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::get_band(TP_NBIoT_Band &band)
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		SaraN2::Nuestats_t stats;

		status = get_nuestats(stats.data);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		if(stats.parameters.earfcn >= EARFCN_B8_LOW && stats.parameters.earfcn <= EARFCN_B8_HIGH)
		{
			band = TP_NBIoT_Interface::TP_NBIoT_Band::BAND_8;
		}
		else if(stats.parameters.earfcn >= EARFCN_B20_LOW && stats.parameters.earfcn <= EARFCN_B20_HIGH)
		{
			band = TP_NBIoT_Interface::TP_NBIoT_Band::BAND_20;
		}
		else 
		{
			band = TP_NBIoT_Interface::TP_NBIoT_Band::BAND_UNKNOWN;
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
int TP_NBIoT_Interface::enable_sim_power_save_mode()
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
int TP_NBIoT_Interface::disable_sim_power_save_mode()
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
int TP_NBIoT_Interface::coap_post(uint8_t *send_data, size_t buffer_len, char *recv_data, int data_indentifier, int &response_code)
{
    int status = -1;
    if(_driver == TP_NBIoT_Interface::SARAN2)
    {
        //todo: load_profile and select_coap_at_interface outside of loop?
        status = _modem.load_profile(SaraN2::COAP_PROFILE_0);
        if(status != TP_NBIoT_Interface::NBIOT_OK)
        {
            debug("\r\nError load_profile(SaraN2::COAP_PROFILE_0); %d",status);
            return status;
        }

        status = _modem.select_coap_at_interface();
        if(status != TP_NBIoT_Interface::NBIOT_OK)
        {
            debug("\r\nError select_coap_at_interface(); %d",status);
            return status;
        }

        uint8_t send_block_number=0;
        uint8_t send_more_block=0;
        uint8_t *buff512= new uint8_t[512];
        long done = 0;
        while (done < buffer_len)
        {
            long available = buffer_len - done;
            if (available>512)
            {
                available = 512;
                send_more_block=1;
            }
            else 
            {
                send_more_block=0;
            }
            //memcpy(send_data + done, buff512, available);
            memcpy(buff512, send_data+done, available); 
            done += available;
            
            debug("\r\nSending");
            status = _modem.coap_post(buff512, recv_data, data_indentifier, send_block_number, 
                                    send_more_block, response_code);
            send_block_number++;
        
            if(status != TP_NBIoT_Interface::NBIOT_OK)
            {
                //debug("\r\nError sending. Response_code %d",response_code);
                return status;
            }
        }
        return TP_NBIoT_Interface::NBIOT_OK;
    }

	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Set T3412 timer to multiples of given units
 * 
 * @param unit Enumerated value within T3412_units enum class
 * @param multiples Value no greater than 31 that determines
 *                  how many multiples of unit to set the
 *                  timer to
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::set_tau_timer(T3412_units unit, uint8_t multiples)
{
    if(multiples > 31)
    {
        return TP_NBIoT_Interface::EXCEEDS_MAX_VALUE;
    }

    char binary[8];
    dec_to_bin_5_bit(multiples, binary);

    char data[9];
	memcpy(&data[8], &"\0", 1);

    char unit_char[3];

    switch(unit)
    {
        case T3412_units::HR_320: 
        {
            memcpy(&unit_char[0], &"110", 3);
            break;
        }
        case T3412_units::HR_10: 
        {
            memcpy(&unit_char[0], &"010", 3);
            break;
        }
        case T3412_units::HR_1: 
        {
            memcpy(&unit_char[0], &"001", 3);
            break;
        }
        case T3412_units::MIN_10: 
        {
            memcpy(&unit_char[0], &"000", 3);
            break;
        }
        case T3412_units::MIN_1: 
        {
            memcpy(&unit_char[0], &"101", 3);
            break;
        }
        case T3412_units::SEC_30: 
        {
            memcpy(&unit_char[0], &"100", 3);
            break;
        }
        case T3412_units::SEC_2: 
        {
            memcpy(&unit_char[0], &"011", 3);
            break;
        }
        case T3412_units::DEACT: 
        {
            memcpy(&unit_char[0], &"111", 3);
            break;
        }
		default:
		{
			return TP_NBIoT_Interface::INVALID_UNIT_VALUE;
		}
    }
    
    memcpy(&data[0], unit_char, 3);
    memcpy(&data[3], &binary[0], 5);

	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.set_t3412_timer(data);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

    return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}   

/** Retrieve T3412 timer value as binary string
 * 
 * @param *timer Pointer to char array in which to store
 *               timer value as binary string
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::get_tau_timer(char *timer)
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.get_t3412_timer(timer);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}
	
	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Retrieve T3412 timer value as units and multiples
 * 
 * @param &unit Address of T3412_units value into which
 *              the determined timer unit will be stored
 * @param &multiples Address of uint8_t into which 
 *                   the determined multiples value will be 
 *                   stored
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::get_tau_timer(T3412_units &unit, uint8_t &multiples)
{
	int status = -1;
    char timer[10];

    status = get_tau_timer(timer);
    if(status != TP_NBIoT_Interface::NBIOT_OK)
    {
        return status;
    }

    if(strncmp(timer, "110", 3) == 0)
    {
        unit = TP_NBIoT_Interface::T3412_units::HR_320;
    }
    else if(strncmp(timer, "010", 3) == 0)
    {
        unit = TP_NBIoT_Interface::T3412_units::HR_10;
    }
    else if(strncmp(timer, "001", 3) == 0)
    {
        unit = TP_NBIoT_Interface::T3412_units::HR_1;
    }
    else if(strncmp(timer, "000", 3) == 0)
    {
        unit = TP_NBIoT_Interface::T3412_units::MIN_10;
    }
    else if(strncmp(timer, "101", 3) == 0)
    {
        unit = TP_NBIoT_Interface::T3412_units::MIN_1;
    }
    else if(strncmp(timer, "100", 3) == 0)
    {
        unit = TP_NBIoT_Interface::T3412_units::SEC_30;
    }
    else if(strncmp(timer, "011", 3) == 0)
    {
        unit = TP_NBIoT_Interface::T3412_units::SEC_2;
    }
    else if(strncmp(timer, "111", 3) == 0)
    {
        unit = TP_NBIoT_Interface::T3412_units::DEACT;
    }
    else
    {
        unit = TP_NBIoT_Interface::T3412_units::INVALID;
    }

    uint8_t binary_value = 16;
    for(int i = 3; i < 8; i++)
    {
        if((int)timer[i] == 49) // 1
        {
            multiples = multiples + binary_value;
        }
        
        if(binary_value == 1)
        {
            break;
        }

        binary_value = binary_value / 2;
    }
    
    return TP_NBIoT_Interface::NBIOT_OK;
}

/** Set T3324 timer to multiples of given units
 * 
 * @param unit Enumerated value within T3324_units enum class
 * @param multiples Value no greater than 31 that determines
 *                  how many multiples of unit to set the
 *                  timer to
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::set_active_time(T3324_units unit, uint8_t multiples)
{
	if(multiples > 31)
	{
		return TP_NBIoT_Interface::EXCEEDS_MAX_VALUE;
	}

	char binary[8];
    dec_to_bin_5_bit(multiples, binary);

    char data[9];
	memcpy(&data[8], &"\0", 1);

    char unit_char[3];

	switch(unit)
	{
		case T3324_units::MIN_6:
		{
			memcpy(&unit_char[0], &"010", 3);
			break;
		}
		case T3324_units::MIN_1:
		{
			memcpy(&unit_char[0], &"001", 3);
			break;
		}
		case T3324_units::SEC_2:
		{
			memcpy(&unit_char[0], &"000", 3);
			break;
		}
		case T3324_units::DEACT:
		{
			memcpy(&unit_char[0], &"111", 3);
			break;
		}
		default:
		{
			return TP_NBIoT_Interface::INVALID_UNIT_VALUE;
		}
	}

	memcpy(&data[0], unit_char, 3);
    memcpy(&data[3], &binary[0], 5);

    int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.set_t3324_timer(data);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}

    return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Retrieve T3324 timer value as binary string
 * 
 * @param *timer Pointer to char array in which to store
 *               timer value as binary string
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::get_active_time(char *timer)
{
	int status = -1;

	if(_driver == TP_NBIoT_Interface::SARAN2)
	{
		status = _modem.get_t3324_timer(timer);
		if(status != TP_NBIoT_Interface::NBIOT_OK)
		{
			return status;
		}

		return TP_NBIoT_Interface::NBIOT_OK;
	}
	
	return TP_NBIoT_Interface::DRIVER_UNKNOWN;
}

/** Retrieve T3324 timer value as units and multiples
 * 
 * @param &unit Address of T3324_units value into which
 *              the determined timer unit will be stored
 * @param &multiples Address of uint8_t into which 
 *                   the determined multiples value will be 
 *                   stored
 * @return Indicates success or failure reason
 */
int TP_NBIoT_Interface::get_active_time(T3324_units &unit, uint8_t &multiples)
{
	int status = -1;
    char timer[10];

    status = get_active_time(timer);
    if(status != TP_NBIoT_Interface::NBIOT_OK)
    {
        return status;
    }

    if(strncmp(timer, "010", 3) == 0)
    {
        unit = TP_NBIoT_Interface::T3324_units::MIN_6;
    }
    else if(strncmp(timer, "001", 3) == 0)
    {
        unit = TP_NBIoT_Interface::T3324_units::MIN_1;
    }
    else if(strncmp(timer, "000", 3) == 0)
    {
        unit = TP_NBIoT_Interface::T3324_units::SEC_2;
    }
    else if(strncmp(timer, "111", 3) == 0)
    {
        unit = TP_NBIoT_Interface::T3324_units::DEACT;
    }
    else
    {
        unit = TP_NBIoT_Interface::T3324_units::INVALID;
    }

    uint8_t binary_value = 16;
    for(int i = 3; i < 8; i++)
    {
        if((int)timer[i] == 49) // 1
        {
            multiples = multiples + binary_value;
        }
        
        if(binary_value == 1)
        {
            break;
        }

        binary_value = binary_value / 2;
    }
    
    return TP_NBIoT_Interface::NBIOT_OK;
}

/** Convert decimal number (with max value of 5-bits) to a binary string,
 *  i.e. 10 = "01010"
 * 
 * @param multiples Decimal value to convert to binary
 * @param *binary Pointer to a char array to which to return 
 *                the binary string
 * @return None
 */
void TP_NBIoT_Interface::dec_to_bin_5_bit(uint8_t multiples, char *binary)
{
    int buffer[8];
    int i = 0;

    for(i; multiples > 0; i++)
    {
        buffer[i] = multiples % 2;
        multiples = multiples / 2;
    }

    int padding = 5 - i;

    for(int j = 0; j < padding; j++)
    {
        sprintf(&binary[j], "%d", 0);
    }

    int index = 4 - padding;
    for(padding; padding < 5; padding++)
    {
        sprintf(&binary[padding], "%d", buffer[index]);
        index--;
    }
}

#endif /* #if BOARD == WRIGHT_V1_0_0 || BOARD == DEVELOPMENT_BOARD_V1_1_0 */


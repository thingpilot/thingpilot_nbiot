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

#if defined (BOARD) && (BOARD == BOARD == WRIGHT_V1_0_0)

#include "SaraN2Driver.h"

#endif /* #if defined (BOARD) && (BOARD == BOARD == WRIGHT_V1_0_0) */


class TP_NBIoT_Interface
{

	public:

		TP_NBIoT_Interface();

		~TP_NBIoT_Interface();

	private:

};



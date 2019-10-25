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


#if defined (BOARD) && (BOARD == WRIGHT_V1_0_0)
TP_NBIoT_Interface::TP_NBIoT_Interface() 
{

}
#endif /* #if defined (BOARD) && (BOARD == WRIGHT_V1_0_0) */



TP_NBIoT_Interface::~TP_NBIoT_Interface()
{
	#if defined (_COMMS_DRIVER) && (_COMMS_DRIVER == SARAN2)
    
    #endif /* #if defined (_COMMS_DRIVER) && (_COMMS_DRIVER == SARAN2) */
}


## Thingpilot NB-IoT Interface Release Notes
**v0.4.0** *25/11/2019*

- Add functionality to check readiness-state of module
- Configure .start() timeout period
- Update preprocessor format to use BOARD macro
- #ifdef guard around .h include in .cpp so that we don't attempt to build if unsupported target is selected

**v0.3.1** *15/11/2019*

- Add .start() function. This will initialise the modem into a state whereby it will automatically attempt network connection and registration and enter PSM where possible
- Ability to determine LTE EARFCN
- No longer require board.h from board_definitions due to implementation of custom Mbed targets, found in [mbed targets](https://github.com/thingpilot/mbed_targets)

**v0.3.0** *14/11/2019*

- Perform AT+CSQ for RSRP/RSRQ parameters
- Query current PSM status of the module
- Return u-blox defined connection status according to section 8.4 of the [u-blox NB-IoT Application Development guide](https://www.u-blox.com/sites/default/files/SARA-N2-Application-Development_AppNote_%28UBX-16017368%29.pdf)

**v0.2.0** *12/11/2019*

- Return CoAP request response code to application layer
- Enable/Disable the Autoconnect, Scrambing, SI_Avoid, BIP, SIM PSM, Cell Reselection and Combined Attach
- Power-cycle modem and return status code representing success or failure
- Enable/Disable modem PSM and query this setting
- Set T3412 timer and read this either as a binary string or as T3412 units (as specified in 3GPP Rel. 13) and multiples of said unit
- Set T3324 timer and read this either as a binary string or as T3324 units (as specified in 3GPP Rel. 13) and multiples of said unit
- Manually trigger network registration/deregistration, radio functionality and GPRS attach/detach

**v0.1.0** *08/11/2019*

- Configure CoAP profiles
- Optionally include host, port and path in PDU header
- Implement HTTP GET, DELETE, PUT and POST requests and return data to the application
- Get perceived radio connection status
- Get network registration status
- Macros for EARFCN band 8/20 ranges
- UE statistics from AT+NUESTATS

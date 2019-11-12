## Thingpilot NB-IoT Interface Release Notes
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

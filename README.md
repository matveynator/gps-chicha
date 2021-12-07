# gps-chicha
gps powered chronograph / tracker

esp8266 / a9g


The A9G module has GSM capabilities with the help of which it can be used to start a call, receive a call, and send an SMS as well when we dial the number whose SIM card is inserted within the A9G module a "RING" message will continuously appear on the monitor. The AT Commands which can be used for calling and sending SMS using the A9G are:-

Call Commands:-

ATA: Used to Answer an Incoming Call. On sending this command "+CIEV: "CALL",1 CONNECT"; the message is received.
ATD: This Command is used to dial a number this command is sent as "AT+number to be dialed" and on sending this command we receive a message saying "ATD+number dialed OK +CIEV: "CALL",1 +CIEV: "SOUNDER",1 ";
ATH: This command is used to disconnect a call. This command is sent as "ATH" and on sending this we receive a message "+CIEV: "CALL",0 OK";
AT+SNFS=0: This command used to enable any earphones/headphones connected to the module. This command enables them.
AT+SNFS=1: This command is used to enable Loudspeaker selection.
AT+CHUP: This command causes the mobile terminal to hang up the current call

SMS Commands:-

AT+CMGF=1: This command is used to select the SMS message format. On sending his command we receive an OK. This is to read and write SMS messages as strings instead of hexadecimal characters.
AT+CMGS: This command is used to send SMS to a given mobile number. The format for sending this command is "AT+CMGS=” mobile number”.On sending this command the monitor will show > You can now type the message text and send the message using the - key combination: TEST After some seconds the modem will respond with the message ID of the message, indicating that the message was sent correctly: "+CMGS: 62". The message will arrive on the mobile phone shortly.
AT+CMGL: This command is used to read SMS messages from preferred storage.

# ESP32 Interfacing

This section briefly describes which topics ESP32 is subscribed to, which ones it is publishing to and which type of data to send/expect back.

## ESP32 DATA RECEIVE

Microcontroller is subscribed to topic:

	302CEM/lion/esp32/led_control

And awaiting input of JSON format. There are so far 2 possible JSON data inputs:

1) Brightness Control. JSON fields are as follows:

	{"name":"kitchen","type":"lights","value":"X"}
	
X is a value of brightness to set. Value of X can be any value on a scale from 0 to 100 in increments of 1. Values higher than 100 are going to be assigned 100 while values lower than 0 are going to be assigned 0. Any non-numerical or non-integer values are dropped and execution continues as per usual.

2) Delay Control. JSON fields are as follows:

	{"name":"kitchen","type":"delay","value":"X"}

Where X is a value of delay (in minutes) to set. Value of X can be any value on a scale from 5 to 60 in increments of 1. Values higher than 60 are going to be assigned 60 while values lower than 0 are going to be assigned 0. Any non-numerical or non-integer values are dropped and execution continues as per usual.

**If JSON data going to have different fields and/or values(apart of described JSON fields before), ESP32 skips such command and continues execution as per usual.**


## ESP32 DATA TRANSMIT

Microcontroller is publishing current LED data to:

	302CEM/lion/esp32/singleLED

The format of publishing is JSON with fields and values of:
	
	{"name":"kitchen","type":"lights","value":"0"}

*NB value of "0" will depend on data received previously. Initially, value 0 is assigned.*

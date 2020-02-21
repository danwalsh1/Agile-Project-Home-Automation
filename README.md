# ESP32 Interfacing

This section briefly describes which topics ESP32 is subscribed to, which ones it is publishing to and which type of data to send/expect.

## ESP32 DATA RECEIVE

Microcontroller is subscribed to topic:

	302CEM/lion/esp32/led_control

And awaiting input of JSON format:

	{"name":"kitchen","type":"lights","value":"0"}

If JSON data going to have different fields and/or values, ESP32 skips such command and continues execution as per usual.

NB change value of "0" to any on a scale from 0 to 100 in increments of 1.




## ESP32 DATA TRANSMIT

Microcontroller is publishing current LED data to:

	302CEM/lion/esp32/singleLED

The format of publishing is JSON with fields and values of:
	
	{"name":"kitchen","type":"lights","value":"0"}

NB value of "0" will depend on data received previously. Initially, value 0 is assigned

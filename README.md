# ESP32 Interfacing

## DATA RECEIVE

Microcontroller is subscribed to topic:

	*302CEM/lion/esp32/led_control*

And awaiting input of JSON format for LED:

	*{"name":"kitchen","type":"lights","value":"0"}*

NB change value of "0" to any on a scale from 0 to 100 in increments of 1.




## DATA TRANSMIT

Microcontroller is publishing current LED data to:

	*302CEM/lion/esp32/singleLED*

The format of publishing is JSON:
	
	*{"name":"kitchen","type":"lights","value":"0"}*

NB value of "0" will depend on data received previously.
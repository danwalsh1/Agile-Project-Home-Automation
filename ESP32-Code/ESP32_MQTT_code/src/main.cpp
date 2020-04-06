// Secure encrypted MQTT with TLS/SSL using the ESP32 esp32doit-devkit-v1
// main.cpp

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <stdio.h>
#include <math.h>
//#include "cJSON.c"
#include "ArduinoJson.h"
#include "mqttcert.h"
#include <Wire.h>
#include <SparkFunTSL2561.h>

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

long lastMsgTimer = 0;
long lastSuccessfulLedMessage = 0;
const int onboardLED = 2; // GPIO2 (D2) on the DOIT-ESP32-DevKitV1
const int switch1 = 19;   // GPIO19 (D19) on the DOIT-ESP32-DevKitV1
const int singleLED = 4;  // GPIO4 (D4) on the DOIT-ESP32-DevKitV1
const int pirSensor = 18; // GPIO18 (D18) on the DOIT-ESP32-DevKitV1
const int buzzer = 13;    // GPIO13 (D13) on the DOIT-ESP32-DevKitV1

int delayForLed;

// Setting PWM properties for LED brightness control
const int ledChannel = 0; // Using PWM channel 0
const int ledFreq = 5000;
const int ledResolution = 7;
int brightness = 0;

// Create an SFE_TSL2561 object, here called "light":

SFE_TSL2561 light;

boolean gain;    // Gain setting, 0 = X1, 1 = X16;
unsigned int ms; // Integration ("shutter") time in milliseconds
double lastLightSendorReading;

bool movementSensed = false;

// Function declaration to be used later
void turnLightOn();
void turnLightOff();
void turnBuzzerOn();
void turnBuzzerOff();
double lightSensorRead();
void printError(byte);

/**
 * A function to blink on-board LED
 * @param int times - amount of times to blink
 */
void blinkLED(int times)
{
    for (int i = 0; i < times; i++)
    {
        digitalWrite(onboardLED, HIGH);
        delay(200);
        digitalWrite(onboardLED, LOW);
        delay(200);
    }
}

/** The receivedCallback() function will be invoked when this client receives data about the subscribed topic
 * @param topic - topic to wait message from
 * @param payload - a message received on a subscribed topic
 * @param length - message length
 */
void receivedCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message received on topic:  ");
    Serial.print(topic);
    Serial.print("  Message reads:  ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }

    // Creating JSON buffer
    StaticJsonBuffer<200> jsonBuffer;

    // Parse payload to be accessed through JsonObject
    JsonObject &root = jsonBuffer.parseObject((char *)payload);

    // Test if parsing succeeds.
    if (!root.success())
    {
        // Append this function if non-JSON data has to be processed
        Serial.println();
        Serial.println("parseObject() failed. Non JSON format data received");
        return;
    }
    if (strcmp(topic, "302CEM/lion/esp32/led_control") == 0)
    {
        // Extract values to appropriate fields of arrays
        const char *name = root["name"];
        const char *type = root["type"];
        const char *value = root["value"];

        // Received value for lights brightness adjustment
        if (strcmp(type, "lights") == 0)
        {
            // Value conversion to 0-127 to fit 8 bit led PWM resolution
            int conversionTo128 = 0; // This to be corrected to some kind of non-volatile memory access to reset on reboot

            // Handling errornous input from JSON data of type "lights"
            if (atoi(value) > 100 && strcmp(name, "kitchen") == 0)
            {
                conversionTo128 = 100;
                Serial.println();
                Serial.print("Entered value exceeding maximum value! Assigning value of: ");
                Serial.println(conversionTo128);
                brightness = conversionTo128;
            }
            else if (atoi(value) < 0 && strcmp(name, "kitchen") == 0)
            {
                conversionTo128 = 0;
                Serial.println();
                Serial.print("Entered value exceeding minimum value! Assigning value of: ");
                Serial.println(conversionTo128);
                brightness = conversionTo128;
            }
            if (atoi(value) >= 0 && atoi(value) <= 100 && strcmp(name, "kitchen") == 0)
            {
                conversionTo128 = int(atoi(value) * 1.27);
                Serial.println();
                Serial.print("Value after conversion is: ");
                Serial.println(conversionTo128);
                brightness = atoi(value);
                lastSuccessfulLedMessage = millis();
                ledcWrite(ledChannel, conversionTo128);
            }
        }

        if (strcmp(type, "delay") == 0)
        {
            if (atoi(value) > 60 && strcmp(name, "kitchen") == 0)
            {
                Serial.println();
                Serial.print("Entered value exceeding minimum value! Assigning value of: ");
                value = "60";
                Serial.println(value);
            }
            else if (atoi(value) < 5 && strcmp(name, "kitchen") == 0)
            {
                Serial.println();
                Serial.print("Entered value exceeding minimum value! Assigning value of: ");
                value = "5";
                Serial.println(value);
            }
            if (atoi(value) <= 60 && atoi(value) >= 5 && strcmp(name, "kitchen") == 0)
            {
                // Converting to milliseconds
                delayForLed = atoi(value) * 1000 * 60;
                Serial.println();
                Serial.print("Delay value is: ");
                Serial.println(delayForLed);
                lastSuccessfulLedMessage = millis();
            }
        }
    }

    Serial.println();
    blinkLED(2);
}

// The mqttConnect() function will attempt to connect to MQTT and subscribe to a topic feed:
void mqttConnect()
{
    while (!mqttClient.connected())
    {

        Serial.print("In mqttConnect(), connecting...  ");

        if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME.c_str(), MQTT_PASSWORD))
        {
            Serial.println("...connected to mqtt server!");
            blinkLED(3);
            Serial.print("Subscribing to topic:  ");
            Serial.println((MQTT_TOPIC_NAME + "/#").c_str());

            // Subscribe topic with default QoS 0
            // Let's just subscribe to the same feed we are publishing to, to see if our message gets recorded.
            mqttClient.subscribe((MQTT_TOPIC_NAME + "/#").c_str());
        }
        else
        {
            Serial.println("...mqttConnect() failed, status code =");
            Serial.println(mqttClient.state());
            Serial.println("try again in 5 seconds...");
            delay(5000); // Wait 5 seconds before retrying
        }
    }
}

/** 
 * Turns the LED on
 */
void turnLightOn()
{
    lastSuccessfulLedMessage = millis();
    //lightSensorRead();
    if (lastLightSendorReading > 600)
    {
        Serial.println("Too bright, no lights are going to be turn on.");
        brightness = 0;
        ledcWrite(ledChannel, 0);
    }
    else if (lastLightSendorReading > 0 && lastLightSendorReading <= 100)
    {
        Serial.println("100% brightness");
        brightness = 100;
        ledcWrite(ledChannel, (int)(brightness * 1.27));
    }
    else if (lastLightSendorReading > 100 && lastLightSendorReading <= 200)
    {
        Serial.println("90% brightness");
        brightness = 90;
        ledcWrite(ledChannel, (int)(brightness * 1.27));
    }
    else if (lastLightSendorReading > 200 && lastLightSendorReading <= 300)
    {
        Serial.println("80% brightness");
        brightness = 80;
        ledcWrite(ledChannel, (int)(brightness * 1.27));
    }
    else if (lastLightSendorReading > 300 && lastLightSendorReading <= 400)
    {
        Serial.println("70% brightness");
        brightness = 70;
        ledcWrite(ledChannel, (int)(brightness * 1.27));
    }
    else if (lastLightSendorReading > 400 && lastLightSendorReading <= 500)
    {
        Serial.println("60% brightness");
        brightness = 60;
        ledcWrite(ledChannel, (int)(brightness * 1.27));
    }
    else if (lastLightSendorReading > 500 && lastLightSendorReading <= 600)
    {
        Serial.println("50% brightness");
        brightness = 50;
        ledcWrite(ledChannel, (int)(brightness * 1.27));
    }
}

/** 
 * Turns the LED off
 */
void turnLightOff()
{
    Serial.println("Turning light off.");
    brightness = 0;
    ledcWrite(ledChannel, 0);
}

/**
 * ISR for movement sensor detecting
 */
void IRAM_ATTR MovementDetected() // Function is called when movement is detected by PIR sensor
{
    movementSensed = true;
    Serial.println("Presence detected... ");
    turnLightOn();
}

/**
 * ISR for buttton press
 */
void IRAM_ATTR ButtonPressed() // Function is called when button has been pressed
{
    Serial.println("Button pressed... ");
    turnLightOff();
}

/**
 * Reads i2c light sensor
 * @returns value in lux, -1 if error occured
 */
double lightSensorRead()
{
    // ms = 1000;
    // light.manualStart();
    delay(ms);
    // light.manualStop();

    // Once integration is complete, we'll retrieve the data.

    // There are two light sensors on the device, one for visible light
    // and one for infrared. Both sensors are needed for lux calculations.

    // Retrieve the data from the device:

    unsigned int data0, data1;

    if (light.getData(data0, data1))
    {
        // getData() returned true, communication was successful

        //Serial.print("data0: ");
        //Serial.print(data0);
        //Serial.print(" data1: ");
        //Serial.print(data1);

        // To calculate lux, pass all your settings and readings
        // to the getLux() function.

        double lux;   // Resulting lux value
        boolean good; // True if neither sensor is saturated

        // Perform lux calculation:

        good = light.getLux(gain, ms, data0, data1, lux);

        // Print out the results:

        //Serial.print(" lux: ");
        //Serial.print(lux);
        if (good)
        {
            //Serial.println(" (good)");
            return lux;
        }
        else
        {
            //Serial.println(" (BAD)");
            return -1;
        }
    }
    else
    {
        // getData() returned false because of an I2C error, inform the user.

        byte error = light.getError();
        printError(error);
        return -1;
    }
}

void printError(byte error)
// If there's an I2C error, this function will
// print out an explanation.
{
    Serial.print("I2C error: ");
    Serial.print(error, DEC);
    Serial.print(", ");

    switch (error)
    {
    case 0:
        Serial.println("success");
        break;
    case 1:
        Serial.println("data too long for transmit buffer");
        break;
    case 2:
        Serial.println("received NACK on address (disconnected?)");
        break;
    case 3:
        Serial.println("received NACK on data");
        break;
    case 4:
        Serial.println("other error");
        break;
    default:
        Serial.println("unknown error");
    }
}

void turnBuzzerOn()
{
    digitalWrite(buzzer, LOW);
};
void turnBuzzerOff()
{
    digitalWrite(buzzer, HIGH);
};

void setup()
{
    // Setting onboard LED
    pinMode(onboardLED, OUTPUT);

    // Configure LED PWM functionalitites
    ledcSetup(ledChannel, ledFreq, ledResolution);
    // Attach ledChannel to the GPIO to be controlled
    ledcAttachPin(singleLED, ledChannel);

    // Setting up buzzer
    pinMode(buzzer, OUTPUT);
    turnBuzzerOff();

    Serial.begin(9600);
    Serial.println();
    Serial.println();
    Serial.println("Hello MQTT program");
    Serial.print("Attempting to connect to WiFi SSID:  ");
    Serial.println(WIFI_SSID);
    blinkLED(1);
    Serial.print("Connecting");

    // We start by connecting to a WiFi network
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    // Start I2C communication
    light.begin();
    // Get factory ID from sensor:
    // (Just for fun, you don't need to do this to operate the sensor)

    unsigned char ID;

    if (light.getID(ID))
    {
        Serial.print("Got factory ID: 0X");
        Serial.print(ID, HEX);
        Serial.println(", should be 0X5X");
    }
    // Most library commands will return true if communications was successful,
    // and false if there was a problem. You can ignore this returned value,
    // or check whether a command worked correctly and retrieve an error code:
    else
    {
        byte error = light.getError();
        printError(error);
    }

    // The light sensor has a default integration time of 402ms,
    // and a default gain of low (1X).

    // If you would like to change either of these, you can
    // do so using the setTiming() command.

    // If gain = false (0), device is set to low gain (1X)
    // If gain = high (1), device is set to high gain (16X)

    gain = 0;

    // If time = 0, integration will be 13.7ms
    // If time = 1, integration will be 101ms
    // If time = 2, integration will be 402ms
    // If time = 3, use manual start / stop to perform your own integration

    unsigned char time = 2;

    // setTiming() will set the third parameter (ms) to the
    // requested integration time in ms (this will be useful later):

    Serial.println("Set timing...");
    light.setTiming(gain, time, ms);

    // To start taking measurements, power up the sensor:

    Serial.println("Powerup...");
    light.setPowerUp();

    // The sensor will now gather light during the integration time.
    // After the specified time, you can retrieve the result from the sensor.
    // Once a measurement occurs, another integration period will start.

    // Initial value reading from a sensor
    lastLightSendorReading = lightSensorRead();

    // Setting up interrupts
    pinMode(switch1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(switch1), ButtonPressed, FALLING);
    pinMode(pirSensor, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pirSensor), MovementDetected, RISING);

    Serial.println("");
    Serial.println("WiFi connected");
    blinkLED(2);
    Serial.print("IP address:  ");
    Serial.println(WiFi.localIP());

    Serial.println("Setting up MQTT...");

    // We need a certificate in order to do a __secure__ TLS/SSL connection to our server
    espClient.setCACert(CA_CERT);

    // Port 1883 is reserved with IANA for use with MQTT.
    // TCP/IP port 8883 is also registered, for using MQTT over SSL.
    // help url:  http://www.iotsharing.com/2017/08/how-to-use-esp32-mqtts-with-mqtts-mosquitto-broker-tls-ssl.html
    mqttClient.setServer(MQTT_SERVER, 8883); // Port 8883 for MQTT over SSL.

    // The receivedCallback() function will be invoked when this client receives the subscribed topic:
    mqttClient.setCallback(receivedCallback);

    Serial.println();
    Serial.println("Setting default value for a delay to 5 min");
    delayForLed = 5 * 1000 * 60;
}

void loop()
{
    mqttConnect();

    // this function will listen for incoming subscribed topic processes and invoke receivedCallback()
    mqttClient.loop();

    // Reading light sensor to off-load the ISR
    lastLightSendorReading = lightSensorRead();
    // we send a reading every 5 sec
    // we count until 5 mins(default, based on lastSuccessfulLedMessage value) reached to avoid blocking program (instead of using delay())
    long now = millis();

    if (now - lastSuccessfulLedMessage > delayForLed)
    {
        ledcWrite(ledChannel, 0);
        brightness = 0;
    }

    if (movementSensed)
    {
        turnBuzzerOn();
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        root["name"] = "kitchen";
        root["type"] = "movement";
        root["value"] = "1";
        char JSONmessageBuffer[100];
        root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

        Serial.println();
        Serial.print("Publishing data:  ");
        Serial.println(JSONmessageBuffer);
        // This prints:
        // {"name":"kitchen","type":"movement","value":"%d"}, %d - valueof(singleLedDataToSend.c_str());
        mqttClient.publish((MQTT_TOPIC_NAME + "/singleLED").c_str(), JSONmessageBuffer);
        movementSensed = false;

        Serial.println();

        StaticJsonBuffer<200> jsonBuffer1;
        JsonObject &root1 = jsonBuffer1.createObject();
        root1["name"] = "kitchen";
        root1["type"] = "value";
        root1["value"] = (String)brightness;
        char JSONmessageBuffer1[100];
        root.printTo(JSONmessageBuffer1, sizeof(JSONmessageBuffer1));

        Serial.println();
        Serial.print("Publishing data:  ");
        Serial.println(JSONmessageBuffer1);
        // This prints:
        // {"name":"kitchen","type":"movement","value":"%d"}, %d - valueof(singleLedDataToSend.c_str());
        mqttClient.publish((MQTT_TOPIC_NAME + "/singleLED").c_str(), JSONmessageBuffer1);
    }

    if (now - lastMsgTimer > 5000)
    {

        lastMsgTimer = now;
        /*
        // Getting button1 reading
        // just convert time stamp to a c-string and send as data:
        String button1DataToSend = (String)digitalRead(switch1); // dataToSend could be a sensor reading instead
        Serial.println();
        Serial.print("Publishing data:  ");
        Serial.println(button1DataToSend);
        blinkLED(1);
        mqttClient.publish((MQTT_TOPIC_NAME + "/button1").c_str(), button1DataToSend.c_str());


        // Getting button2 reading
        // just convert time stamp to a c-string and send as data:
        String button2DataToSend = (String)digitalRead(switch2); // dataToSend could be a sensor reading instead
        Serial.println();
        Serial.print("Publishing data:  ");
        Serial.println(button2DataToSend);
        blinkLED(1);
        mqttClient.publish((MQTT_TOPIC_NAME + "/button2").c_str(), button2DataToSend.c_str());
        */

        //mqttClient.publish((MQTT_TOPIC_NAME + "/singleLED").c_str(), singleLedDataToSend.c_str());
        //mqttClient.publish((MQTT_TOPIC_NAME + "/singleLED").c_str(), "{\"name\":\"kitchen\", \"type\": \"lights\", \"value\":\"50\"}");

        //  Getting singleLED reading
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        root["name"] = "kitchen";
        root["type"] = "lights";
        root["value"] = (String)brightness;
        char JSONmessageBuffer[100];
        root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

        Serial.println();
        Serial.print("Publishing data:  ");
        Serial.println(JSONmessageBuffer);
        // This prints:
        // {"name":"kitchen","type":"lights","value":"%d"}, %d - valueof(singleLedDataToSend.c_str());
        mqttClient.publish((MQTT_TOPIC_NAME + "/singleLED").c_str(), JSONmessageBuffer);
    }
}
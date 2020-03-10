// Chris Bass - Coventry University
// 13/04/2019
// Secure encrypted MQTT with TLS/SSL using the ESP32 esp32doit-devkit-v1
// main.cpp
// This program provides an example of sending and receiving data via the MQTT publish-subscribe protocol

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <stdio.h>
#include <math.h>
//#include "cJSON.c"
#include "ArduinoJson.h"

#include "mqttcert.h"

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

long lastMsgTimer = 0;
long lastSuccessfulLedMessage = 0;
const int onboardLED = 2;       // GPIO2 (D2) on the DOIT-ESP32-DevKitV1
const int switch1 = 22;         // GPIO22 (D22) on the DOIT-ESP32-DevKitV1
//const int switch2 = 18;          // GPIO5 (D5) on the DOIT-ESP32-DevKitV1
const int singleLED = 4;        // GPIO4 (D4) on the DOIT-ESP32-DevKitV1
const int pirSensor = 18;

bool lightsOn;

int delayForLed;


// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 7;
int brightness = 0;


void blinkLED(int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(onboardLED, HIGH);
        delay(200);
        digitalWrite (onboardLED, LOW);
        delay(200);
    }
}

// The receivedCallback() function will be invoked when this client receives data about the subscribed topic:
void receivedCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received on topic:  ");
    Serial.print(topic);
    Serial.print("  Message reads:  ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }

    // Creating JSON buffer
    StaticJsonBuffer<200> jsonBuffer;

    // Parse payload to be accessed through JsonObject
    JsonObject& root = jsonBuffer.parseObject((char*)payload);

    // Test if parsing succeeds.
    if (!root.success()) 
    {
        // Append this function if non-JSON data has to be processed
        Serial.println();
        Serial.println("parseObject() failed. Non JSON format data received");
        return;
    }
    if(strcmp(topic, "302CEM/lion/esp32/led_control") == 0)
    {
        // Extract values to appropriate fields of arrays
        const char* name = root["name"];
        const char* type = root["type"];
        const char* value = root["value"];

        // Received value for lights brightness adjustment
        if(strcmp(type, "lights") == 0)
        {
            // Value conversion to 0-127 to fit 8 bit led PWM resolution 
            int conversionTo128=0;                      // This to be corrected to some kind of non-volatile memory access to reset on reboot

            // Handling errornous input from JSON data of type "lights"
            if(atoi(value) > 100 && strcmp(name, "kitchen") == 0)
            {
                conversionTo128 = 100;
                Serial.println();
                Serial.print("Entered value exceeding maximum value! Assigning value of: ");
                Serial.println(conversionTo128);
                brightness = conversionTo128;
            }
                
            else if(atoi(value) < 0 && strcmp(name, "kitchen") == 0)
            {
                conversionTo128 = 0;
                Serial.println();
                Serial.print("Entered value exceeding minimum value! Assigning value of: ");
                Serial.println(conversionTo128);
                brightness = conversionTo128;
            }
            if(atoi(value) >= 0 && atoi(value) <= 100 && strcmp(name, "kitchen") == 0)
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

        if(strcmp(type, "delay") == 0)
        {   
            if(atoi(value) > 60 && strcmp(name, "kitchen") == 0)
            {
                Serial.println();
                Serial.print("Entered value exceeding minimum value! Assigning value of: ");
                value = "60";
                Serial.println(value);
            }
            else if(atoi(value) < 5 && strcmp(name, "kitchen") == 0)
            {
                Serial.println();
                Serial.print("Entered value exceeding minimum value! Assigning value of: ");
                value = "5";
                Serial.println(value);
            }
            if(atoi(value) <= 60 && atoi(value) >= 5 && strcmp(name, "kitchen") == 0)
            {
                // Converting to milliseconds
                delayForLed = atoi(value)*1000*60;
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
void mqttConnect() {
    while (!mqttClient.connected()) {

        Serial.print("In mqttConnect(), connecting...  ");

        if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME.c_str(), MQTT_PASSWORD)) {
            Serial.println("...connected to mqtt server!");
            blinkLED(3);
            Serial.print("Subscribing to topic:  ");
            Serial.println((MQTT_TOPIC_NAME + "/#").c_str());

            // Subscribe topic with default QoS 0
            // Let's just subscribe to the same feed we are publishing to, to see if our message gets recorded.
            mqttClient.subscribe((MQTT_TOPIC_NAME + "/#").c_str());

        } else {
            Serial.println("...mqttConnect() failed, status code =");
            Serial.println(mqttClient.state());
            Serial.println("try again in 5 seconds...");
            delay(5000); // Wait 5 seconds before retrying
        }
    }
}

/*
void IRAM_ATTR MovementDetected()
{
    Serial.println("Presence detected");
    turnLightOn();
}

void noMotionDetected()
{
    turnLightOff()
}

void turnLightOn()
{
    brightness = 100;
    ledcWrite(ledChannel, (int)(brightness*1.27));
}

void turnLightOff()
{
    brightness = 0;
    ledcWrite(ledChannel, (int)(brightness*1.27));
}
*/
void setup() {
    pinMode(onboardLED, OUTPUT);
    pinMode(switch1, INPUT);
    //pinMode(switch2, INPUT);
    //pinMode(pirSensor, INPUT_PULLUP);
    //attachInterrupt(digitalPinToInterrupt(pirSensor), MovementDetected, RISING);

    // configure LED PWM functionalitites
    ledcSetup(ledChannel, freq, resolution);
    // attach the channel to the GPIO to be controlled
    ledcAttachPin(singleLED, ledChannel);

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

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

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
    delayForLed = 5*1000*60;
}




void loop() {

    mqttConnect();

    // this function will listen for incoming subscribed topic processes and invoke receivedCallback()
    mqttClient.loop();

    // we send a reading every 5 secs
    // we count until 5 secs reached to avoid blocking program (instead of using delay())
    long now = millis();
    if(now - lastSuccessfulLedMessage > delayForLed)
    {
        ledcWrite(ledChannel, 0);
        brightness = 0;
        
    }
    if (now - lastMsgTimer > 5000) {
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


        //Getting button2 reading
        // just convert time stamp to a c-string and send as data:
        String button2DataToSend = (String)digitalRead(switch2); // dataToSend could be a sensor reading instead
        Serial.println();
        Serial.print("Publishing data:  ");
        Serial.println(button2DataToSend);
        blinkLED(1);
        mqttClient.publish((MQTT_TOPIC_NAME + "/button2").c_str(), button2DataToSend.c_str());
        */

        //  Getting singleLED reading


        //mqttClient.publish((MQTT_TOPIC_NAME + "/singleLED").c_str(), singleLedDataToSend.c_str());
        //mqttClient.publish((MQTT_TOPIC_NAME + "/singleLED").c_str(), "{\"name\":\"kitchen\", \"type\": \"lights\", \"value\":\"50\"}");

        //  Getting singleLED reading
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["name"] = "kitchen";
        root["type"] = "lights";
        root["value"] = (String)brightness;
        char JSONmessageBuffer[100];
        root.printTo(JSONmessageBuffer,sizeof(JSONmessageBuffer));
        
        Serial.println();
        Serial.print("Publishing data:  ");
        Serial.println(JSONmessageBuffer);
        // This prints:
        // {"name":"kitchen","type":"lights","value":"%d"}, %d - valueof(singleLedDataToSend.c_str());
        mqttClient.publish((MQTT_TOPIC_NAME + "/singleLED").c_str(), JSONmessageBuffer);
        
    }
}

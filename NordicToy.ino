#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "WebInterface.h"
#include "Fire.h"
#include "Emoncms.h"
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <BlynkSimpleEsp8266.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//char auth[] = "ccfb9e756f0d48c9a430bb08615208f0"; // Go to the Project Settings (nut icon).
// char auth[] = "658343a33bfc49d1aeb1465ffa2d85fe"; // Go to the Project Settings (nut icon).

char* auth[3]; // = "ebbd76a09c4c46e8af5437fd6025a223"; // Nordic Toy 1
// char auth2[] = "b7d9e24a10024b958cad5abf84bcb9f7"; // Nordic Toy 2
// char auth3[] = "87ee90f313eb4cd09f5385dde6779af4"; // Nordic Toy 3

WidgetBridge bridge1(V6);
WidgetBridge bridge2(V6);

//const char* ssid = "Zuhause";
//const char* password = "dresden123456";
const char* ssid = "Nordic Game Jam 2017";
const char* password = "notthere";
byte brightness = 31;

int currentLEDs = 15;

int myId = 2;
int toyCount = 3;

#define NUM_LEDS 360
CRGB leds[NUM_LEDS];

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("Fire2012_NeoPixelBus");

    pinMode(D2, OUTPUT);

    digitalWrite(D2, LOW);

    pinMode(D3, INPUT_PULLUP);

    setupFire();

    for (int i = 0; i < 15; ++i)
    {
        leds[i] = CRGB::Black;
        if (i <= myId) leds[i] = CRGB::Red;
    }
    //strip.Show();
    keepFireAlive();

    setupWiFi();

    auth[0] = "ebbd76a09c4c46e8af5437fd6025a223";
    auth[1] = "b7d9e24a10024b958cad5abf84bcb9f7"; 
    auth[2] = "87ee90f313eb4cd09f5385dde6779af4"; 

    Blynk.begin(auth[myId], ssid, password);


    setupWebServer();


    // ArduinoOTA.setPassword("admin");
ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    for (int i = 0; i < 15; ++i)
    {
        leds[i] = CRGB::Black;
        if (i <= myId) leds[i] = CRGB::Green;
    }

}

int red = 0;
int green = 0;
int blue = 0;

void updateStrip()
{
    for (int i = 0; i < 15; i++)
    {
        if (currentLEDs > i)
        {
            leds[i].r = red;
            leds[i].g = green;
            leds[i].b = blue;      
        }
        else
        {
            leds[i].r = 0;
            leds[i].g = 0;
            leds[i].b = 0; 
        }
    }
}

BLYNK_CONNECTED()
{
    for (int i = 0; i < toyCount; ++i)
    {
        bridge1.setAuthToken(auth[(myId + 1) % toyCount]);
        bridge2.setAuthToken(auth[(myId + 2) % toyCount]);
    }
}

BLYNK_WRITE(V1) 
{
    brightness = param.asInt(); 
}

BLYNK_WRITE(V2) 
{
    red = param.asInt();
    updateStrip();
}

BLYNK_WRITE(V3) 
{
    green = param.asInt();
    updateStrip();
}
BLYNK_WRITE(V4) 
{
    blue = param.asInt();
    updateStrip();
}
BLYNK_WRITE(V5) 
{
    currentLEDs = param.asInt();
    updateStrip();
}

BLYNK_WRITE(V6) 
{
    int whichButton = param.asInt();
    //leds[13 - whichButton] = CRGB::Blue;
    Serial.print("Received button: ");
    Serial.println(whichButton);
    //updateStrip();
}

long lastButton = 0;
void loopGame()
{
    if (digitalRead(D3) == LOW && millis() - lastButton > 50)
    {
        leds[14] = CRGB::Yellow;
        lastButton = millis();
        bridge1.virtualWrite(V6, myId);
        bridge2.virtualWrite(V6, myId);
    }

    if (millis() - lastButton > 50)
    {
        leds[14] = CRGB::Black;
    }
}


void loop()
{
    ArduinoOTA.handle();
    server.handleClient();
    //logToEmoncms();
    keepFireAlive();
    loopGame();

    Blynk.run();

    delay(10);
}

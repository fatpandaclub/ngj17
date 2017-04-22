#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
// #include "WebInterface.h"
#include "Fire.h"
// #include "Emoncms.h"
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <BlynkSimpleEsp8266.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//char auth[] = "ccfb9e756f0d48c9a430bb08615208f0"; // Go to the Project Settings (nut icon).
// char auth[] = "658343a33bfc49d1aeb1465ffa2d85fe"; // Go to the Project Settings (nut icon).

char* auth; // = "ebbd76a09c4c46e8af5437fd6025a223"; // Nordic Toy 1
// char auth2[] = "b7d9e24a10024b958cad5abf84bcb9f7"; // Nordic Toy 2
// char auth3[] = "87ee90f313eb4cd09f5385dde6779af4"; // Nordic Toy 3

WidgetBridge bridge1(V6);
// WidgetBridge bridge2(V6);

//const char* ssid = "Zuhause";
//const char* password = "dresden123456";
const char* ssid = "Nordic Game Jam 2017";
const char* password = "notthere";
byte brightness = 31;

int currentLEDs = 15;

int myId = 1;
int toyCount = 3;

#define NUM_LEDS 360
CRGB leds[NUM_LEDS];

void setup()
{
    myId = 3;

    Serial.begin(115200);
    Serial.println();
    Serial.print("My hardware id: ");
    Serial.println(system_get_chip_id());

    if (system_get_chip_id() == 1662332) myId = 0;
    if (system_get_chip_id() == 9638783) myId = 1;
    if (system_get_chip_id() == 1943989) myId = 2;

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

    auth = "ebbd76a09c4c46e8af5437fd6025a223";
    //auth[1] = auth[0]; //"b7d9e24a10024b958cad5abf84bcb9f7"; 
    //auth[2] = auth[0]; //"87ee90f313eb4cd09f5385dde6779af4"; 

    Blynk.begin(auth, ssid, password);


    // setupWebServer();


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
    bridge1.setAuthToken(auth);
    //bridge2.setAuthToken(auth[(myId + 2) % toyCount]);
    Serial.println("Set auth tokens for bridge");
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
    bool isButtonPressed = whichButton < 1000;

    if (isButtonPressed)
        leds[13 - whichButton] = CRGB::Blue;
    else
        leds[13 - (whichButton - 1000)] = CRGB::Black;

    int buttonSum = (leds[13].b > 0 ? 1 : 0) +  (leds[12].b > 0 ? 1 : 0) +  (leds[11].b > 0 ? 1 : 0);
    digitalWrite(D2, buttonSum >= 2 ? HIGH : LOW);

    Serial.print("Received button: ");
    Serial.println(whichButton);
    //updateStrip();
}

long lastButton = 0;
bool buttonPressed = false;
void loopGame()
{
    if (digitalRead(D3) == LOW && !buttonPressed)
    {
        Serial.println("My button pressed");
        leds[14] = CRGB::Yellow;
        lastButton = millis();
        buttonPressed = true;
        bridge1.virtualWrite(V6, myId);
    }

    if (digitalRead(D3) == HIGH && millis() - lastButton > 50 && buttonPressed)
    {
        buttonPressed = false;
        leds[14] = CRGB::Black;
        bridge1.virtualWrite(V6, myId + 1000);
    }
}


void loop()
{
    ArduinoOTA.handle();
    // server.handleClient();
    //logToEmoncms();
    keepFireAlive();
    loopGame();

    Blynk.run();

    delay(10);
}

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

#define LED_PIN  3
#define NUM_LEDS 360
CRGB leds[NUM_LEDS];
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(NUM_LEDS, LED_PIN);

void setup()
{
    myId = 3;

    Serial.begin(115200);
    Serial.println();
    Serial.print("My hardware id: ");
    Serial.println(system_get_chip_id());

    // HEY ROBIN, WE'VE CHANGED THIS, SORRY
    if (system_get_chip_id() == 1943989) myId = 0;
    if (system_get_chip_id() == 9638783) myId = 1;
    if (system_get_chip_id() == 14098955) myId = 2;
    if (system_get_chip_id() == 1662332) myId = 3;

    

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

int myHue = 100;
int mySize = 3;
int myPulseSpeed = 100;

int INACTIVE = 0;
int ALIVE = 1;
int SHOOTER = 2;
int SHIELD = 3;
int DEAD = 4;

int animationState = INACTIVE;

int patternCount = 0;
struct Pattern
{
    bool isShortPattern;
    byte hue;
};
Pattern patterns[5];

void createRandomPattern()
{
    int pCount = random(1, 4);
    for (int i = 0; i < pCount; ++i)
    {
        patterns[i].isShortPattern = random(0, 100) < 50;
        patterns[i].hue = 255;
    }
}

void setPixel(int p, CRGB col)
{
    if (p >= 0 && p < 15)
        leds[p] = col;
}

void drawPattern(int pos, bool isShortPattern, byte hue)
{
    setPixel(pos, CHSV(hue, 255, 255));
    setPixel(pos+1, CHSV(hue, 255, 255));

    if (!isShortPattern)
    {
        setPixel(pos-1, CHSV(hue, 255, 255));
        setPixel(pos+2, CHSV(hue, 255, 255));
    }

}

void drawPlayer()
{
    if (animationState == INACTIVE) return;

    nscale8(leds, 15, 230);

    if (animationState == ALIVE || animationState == SHIELD)
    {
         int offset = beatsin16(37, mySize, 15 - mySize);
        if (mySize == 0) return;
        // draw center

        if (animationState == SHIELD)
        {
            for (int i = 0; i < 15; ++i)
            {
                setPixel(i, CRGB(0,0, beatsin16(140, 100, 255) - (8 - abs(i - 7))*5));
            }
        }

        for (int i = 0; i < beatsin16(myPulseSpeed, 1, mySize); i++)
        {
            setPixel(offset + i, CHSV(myHue, random(220, 255), 255 - i * 10));
            setPixel(offset - i, CHSV(myHue, random(220, 255), 255 - i * 10));
        }
    }
    else if (animationState == SHOOTER)
    {

        for (int i = 0; i < 15; ++i)
        {
            //setPixel(i, CHSV(0, 0, beatsin16(250, 0, 255)));
            setPixel(i, CHSV(0, 0, 255));
        }

        if (patternCount == 0)
        {

        }
        else
        {
            if (patternCount == 1)
            {
                drawPattern(7, patterns[0].isShortPattern, patterns[0].hue);
            }
            else if (patternCount == 2)
            {
                drawPattern(5, patterns[0].isShortPattern, patterns[0].hue);
                drawPattern(9, patterns[1].isShortPattern, patterns[1].hue);
            }
            else if (patternCount == 3)
            {
                drawPattern(3, patterns[0].isShortPattern, patterns[0].hue);
                drawPattern(7, patterns[1].isShortPattern, patterns[1].hue);
                drawPattern(11, patterns[2].isShortPattern, patterns[2].hue);
            }
        }
    }
    else if (animationState == DEAD)
    {
        for (int i = 0; i < 15; ++i)
        {
            setPixel(i, CRGB(beatsin16(40, 0, 255), 0, 0));
        }
    }
   
    strip.Show();
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

int numberOfPlayers = 4;
int readyPlayerCount = 0;
long lastButton = 0;
bool buttonPressed = false;
bool isServer = false;
bool isGameStarted = false;
bool isUserReady = false;
int shooterId = -1;
int heroId = -1;
bool hasShooterShot = false;
long shotTimestamp = 0;
bool hasDefendedSelf = false;
int gameLength = 3000;

BLYNK_WRITE(V6) 
{
    int whichButton = param.asInt();
    bool isButtonPressed = whichButton < 1000;

    if (isButtonPressed)
        leds[13 - whichButton] = CRGB::Blue;
    else
        leds[13 - (whichButton - 1000)] = CRGB::Black;

    int buttonSum = (leds[13].b > 0 ? 1 : 0) + (leds[12].b > 0 ? 1 : 0) + (leds[11].b > 0 ? 1 : 0);
    digitalWrite(D2, buttonSum >= 2 ? HIGH : LOW);

    Serial.print("Received button: ");
    Serial.println(whichButton);
    //updateStrip();
}

// Other players use this to tell the server they are ready
BLYNK_WRITE(V7) 
{
    readyPlayerCount++;
}

// Server uses this to tell others that the game has started
BLYNK_WRITE(V8)
{
    isGameStarted = true;
}

// Server uses this to tell others who is the shooter
BLYNK_WRITE(V9)
{
    shooterId = param.asInt();
}

// Shooter uses this to tell everyone, they have shot
BLYNK_WRITE(V10)
{
    hasShooterShot = true;
    shotTimestamp = millis();
}

// Someone can use this to restart the game at everyone
BLYNK_WRITE(V11)
{
    restartGame();
}

// Server uses this to tell others who is the hero
BLYNK_WRITE(V12)
{
    heroId = param.asInt();
}


void loopGame()
{
    if(myId == 0 && !isServer)
    {
        isServer = true;
    }

    if(!isGameStarted)
    {
        if(readyPlayerCount == numberOfPlayers) {
            isGameStarted = true;
            bridge1.virtualWrite(V8, 1);
        }

        if(!isUserReady) 
        {
            red = green = blue = 0;
            updateStrip();
            leds[7] = CRGB::White;

            if(isButtonPressed() == 1)
            {
                isUserReady = true;
                if(!isServer)
                {
                    bridge1.virtualWrite(V7, myId);
                }
                else 
                {   
                    readyPlayerCount++;
                }
            }
        }
        else 
        {
            red = green = blue = 255;
            updateStrip();
        }
    }
    else // Game has started now!
    {
        // Choose a shooter!
        if(isServer && shooterId == -1)
        {
            shooterId = random(numberOfPlayers);
            heroId = random(numberOfPlayers);
            animationState = ALIVE;
            if(shooterId == heroId) 
            {
                heroId = shooterId == numberOfPlayers - 1 ? shooterId - 1 : shooterId + 1; 
            }

            bridge1.virtualWrite(V9, shooterId);
            bridge1.virtualWrite(V12, heroId);
        }

        if(shooterId == myId)
        {
            digitalWrite(D2, HIGH);
            if(isButtonPressed() == 1 && !hasShooterShot)
            {
                bridge1.virtualWrite(V10, 1);
                hasShooterShot = true;
                shotTimestamp = millis();
                red = green = 0;
                blue = 255;
                animationState = SHOOTER;
                updateStrip();
            }
        }
        if(hasShooterShot)
        {
            if(myId != heroId && myId != shooterId && !hasDefendedSelf)
            {
                red = 0;
                green = blue = 255;
                updateStrip();
            }
            else if (myId == heroId)
            {
                hasDefendedSelf = true; // hero is already protected, but if they press the button later, they lose this
            }

            if(millis() - shotTimestamp < gameLength)
            {
                if(isButtonPressed() == 1 && shooterId != myId)
                {
                    if(heroId == myId)
                    {
                        hasDefendedSelf = false;
                        green = blue = 0;
                        red = 255;
                        animationState = DEAD;
                        updateStrip();
                    }
                    else
                    {
                        hasDefendedSelf = true;
                        green = 0;
                        animationState = SHIELD;
                        red = blue = 255;
                        updateStrip();

                    }                    
                }   
            }
            else // game over, bruh
            {
                if(!hasDefendedSelf && shooterId != myId)
                {
                    green = blue = 0;
                    red = 255;
                    animationState = DEAD;
                    updateStrip();
                }
                else if (hasDefendedSelf && shooterId != myId)
                {
                    green = 0;
                    red = blue = 255;
                    animationState = ALIVE;
                    updateStrip();
                }
                else if (shooterId == myId)
                {
                    green = 255;
                    red = blue = 0;
                    animationState = SHOOTER;
                    updateStrip();
                }

                // Let's restart the game, everyone!
                if(isButtonPressed() == 1 && millis() - shotTimestamp > gameLength + 1000)
                {
                    restartGame();
                    bridge1.virtualWrite(V11, 1);
                }
            }

        }
    }
}

void restartGame()
{
    Serial.println("Restart a game. Preferably this game.");
    digitalWrite(D2, LOW);
    readyPlayerCount = 0;
    isGameStarted = false;
    isUserReady = false;
    shooterId = -1;
    hasShooterShot = false;
    hasDefendedSelf = false;
    animationState = INACTIVE;
}


int isButtonPressed() {
    if (digitalRead(D3) == LOW && !buttonPressed)
    {
        Serial.println("My button pressed");
        lastButton = millis();
        buttonPressed = true;
        return 1;
    }

    if (digitalRead(D3) == HIGH && millis() - lastButton > 50 && buttonPressed)
    {
        buttonPressed = false;
        return 0;
    }

    return -1;
}

void loop()
{
    ArduinoOTA.handle();
    // server.handleClient();
    //logToEmoncms();
    keepFireAlive();
    loopGame();

    Blynk.run();
    drawPlayer();
    delay(10);
}

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);

#define RELAY1     4
#define RELAY2     5
#define LEDSTRIP   3
#define LEDBUILTIN 14

char apssid[] = "wifilamp";
char appassword[] = "hellohihi";

int ledval = 0;
byte relay1val = 0;
byte relay2val = 0;

void handleRoot() {
    server.send(200, "text/plain", "Hello world!");
}

void handleLed() {
    int tmpval = 0;
    if (server.hasArg("val")) {
        if (server.arg("val") != NULL && server.arg("val") != "") {
            tmpval = server.arg("val").toInt();
        }
    }
    delay(0);
    char buf[10];
    if ((tmpval >= 0) && (tmpval <= 1023)) {
        ledval = tmpval;
        sprintf(buf, "%d", (int)ledval);
    } else {
        sprintf(buf, "-1");
    }
    delay(0);
    server.send(200, "text/plain", buf); 
}

void handleRelay() {
    byte val = 0;
    if (server.hasArg("val")) {
        if (server.arg("val") != NULL && server.arg("val") != "") {
            val = server.arg("val").toInt();
        }
    }
    byte index = 0;
    if (server.hasArg("index")) {
        if (server.arg("index") != NULL && server.arg("index") != "") {
            index = server.arg("index").toInt();
        }
    }
    const char* buf;
    if (index == 0) {
        if (val == 2) {
            relay1val = 1 - relay1val;
        } else {
            relay1val = val;  
        }
        buf = (relay1val == 1) ? "On" : "Off";
    } else if (index == 1) {
        if (val == 2) {
            relay2val = 1 - relay2val;
        } else {
            relay2val = val;  
        }
        buf = (relay2val == 1) ? "On" : "Off";
    } else {
        server.send(200, "text/plain", "???"); 
        return;
    }
    delay(0);
    server.send(200, "text/plain", buf); 
}

void setup() {
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(LEDSTRIP, OUTPUT);

    Serial.begin(115200);
    
    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    
    wifiMulti.addAP("NETWORK_NAME", "NETWORK_PASSWORD");
    
    Serial.println("Connecting ...");
    int i = 0;
    long lastSerial = millis();
    int statusLedPWM = 200; 
    int statusLedDelta = 10;
    while (wifiMulti.run() != WL_CONNECTED) {
        delay(10);
        if (millis() - lastSerial > 250) {
            Serial.println(".");
            lastSerial = millis();
            statusLedDelta *= -1;
            i++;
            if (i == 40) break;
        }
        statusLedPWM = statusLedPWM + statusLedDelta;
        analogWrite(LEDBUILTIN, statusLedPWM);
    }

    if (i == 40) {
        Serial.println("Cannot connect to specified networks.\nCreating one...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP(apssid, appassword);
        Serial.print("Access Point \"");
        Serial.print(apssid);
        Serial.print("\" started. Password: ");
        Serial.println(appassword);
        
        Serial.print("IP address:\t");
        Serial.println(WiFi.softAPIP()); 
    } else {
    
        Serial.println('\n');
        Serial.print("Connected to ");
        Serial.println(WiFi.SSID());
        Serial.print("IP address:\t");
        Serial.println(WiFi.localIP());
    }

    if (MDNS.begin("lamp")) {
        Serial.println("mDNS responder started");
    } else {
        Serial.println("Error setting up MDNS responder!");
    }

    server.on("/", handleRoot);
    server.on("/relay", handleRelay);
    server.on("/led", handleLed);

    server.begin();
    Serial.println("HTTP server started");
    digitalWrite(LEDBUILTIN, 0);
}

void loop() {
    server.handleClient();
    analogWrite(LEDSTRIP, 1023 - ledval);
    digitalWrite(RELAY1, relay1val);
    digitalWrite(RELAY2, relay2val);
}

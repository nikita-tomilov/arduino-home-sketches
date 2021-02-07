#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <Adafruit_BME280.h>
#include "util.h"

OneWire ds(2);
#include "ds18b20custom.h"

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);
Adafruit_BME280 bme;

char apssid[] = "wifilamp";
char appassword[] = "hellohihi";

byte ds_outside_addr[] = {0x28, 0xFF, 0x86, 0x29, 0xC3, 0x16, 0x03, 0xEF};
byte ds_inside_addr[]  = {0x28, 0xFF, 0xEE, 0xAD, 0x53, 0x17, 0x04, 0x78};

float ds_temp_inside = 0;
float ds_temp_outside = 0;
float bme_temp = 0;
float bme_hum = 0;
float bme_pres = 0;
float heat_idx = 0;

bool setup_ds_done = false;
long last_temp_scan = -3000;

void handleRoot() {
  server.send(200, "text/plain", "Hello world!\n Go to /json for json info.");
}

void sendJsonResponse() {
  char buf[1024];
  sprintf(buf, "{\n \"DS_INS\": %f,\n \"DS_OUT\": %f,\n \"BME_T\": %f,\n \"BME_HUM\": %f,\n \"BME_PRES\": %f,\n \"HEAT_IDX\": %f\n}", ds_temp_inside, ds_temp_outside, bme_temp, bme_hum, bme_pres, heat_idx);
  server.send(200, "application/json", buf);
}

void setup() {
    Serial.begin(115200);

    if (!bme.begin(0x76)) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
    }
    
    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    
    wifiMulti.addAP("networkname", "networkpass");
    
    Serial.println("Connecting ...");
    int i = 0;
    long lastSerial = millis();
    while (wifiMulti.run() != WL_CONNECTED) {
        delay(10);
        if (millis() - lastSerial > 250) {
            Serial.println(".");
            lastSerial = millis();
            i++;
            if (i == 40) break;
        }
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

    if (MDNS.begin("wifiweather")) {
        Serial.println("mDNS responder started");
    } else {
        Serial.println("Error setting up MDNS responder!");
    }

    server.on("/", handleRoot);
    server.on("/json", sendJsonResponse);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
    if ((!setup_ds_done) && ((millis() - last_temp_scan) > 3000)) {
        setup_ds_temp(ds_inside_addr);
        setup_ds_temp(ds_outside_addr);
        setup_ds_done = true;
        last_temp_scan = millis();
    } else if ((setup_ds_done) && ((millis() - last_temp_scan) > 1000)) {
        ds_temp_inside = get_ds_temp(ds_inside_addr);
        ds_temp_outside = get_ds_temp(ds_outside_addr);
        bme_temp = bme.readTemperature();
        bme_hum = bme.readHumidity();
        bme_pres = bme.readPressure();
        heat_idx = computeHeatIndex(bme_temp, bme_hum, false);
        setup_ds_done = false;
        last_temp_scan = millis();
        Serial.print("DS_T_INS ");
        Serial.println(ds_temp_inside);
        Serial.print("DS_T_OUT ");
        Serial.println(ds_temp_outside);
        Serial.print("BME_T ");
        Serial.println(bme_temp);
        Serial.print("BME_H ");
        Serial.println(bme_hum);
        Serial.print("BME_P ");
        Serial.println(bme_pres);
    }
}

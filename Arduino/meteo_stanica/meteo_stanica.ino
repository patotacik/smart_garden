#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <FirebaseArduino.h>
  #include <WiFiClient.h>
#endif
#include <Wire.h>
#include "DHT.h"       
#define DHTTYPE DHT11 
#define dht_dpin D6
DHT dht(dht_dpin, DHTTYPE); 

#include <BH1750FVI.h>
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);

#define FIREBASE_HOST "*****.firebaseio.com"
#define FIREBASE_AUTH "*****"

const char* ssid     = "*****";
const char* password = "*****";
const char* serverName = "*****";
String apiKeyValue = "*****";
String sensorName = "meteo";
String sensorLocation = "balkon";

void setup()
{
  Serial.begin(115200);
  LightSensor.begin();  
  dht.begin();
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
}


void loop()
{
uint16_t lux = LightSensor.GetLightIntensity();
float h = dht.readHumidity();
float t = dht.readTemperature();   

if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    Serial.print(lux);

    String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + h + "&value2=" + t +"&value3=" + lux +"";
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
        int httpResponseCode = http.POST(httpRequestData);
 if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
        Firebase.setFloat ("vlhkost_vz",h);
        Firebase.setFloat ("teplota_vz",t);
        Firebase.setFloat ("light",lux);

  }
  else {
    Serial.println("WiFi Disconnected");
  }
  ESP.deepSleep(480e6); 
}

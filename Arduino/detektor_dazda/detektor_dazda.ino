#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <FirebaseArduino.h>
#include <WiFiClient.h>
#include <Wire.h>

#define rainAnalog A0
#define rainDigital D2

#define FIREBASE_HOST "****.firebaseio.com"
#define FIREBASE_AUTH "****"

const char* ssid     = "****";
const char* password = "****";
const char* serverName = ****";
String apiKeyValue = "****";
String sensorName = "dazd";
String sensorLocation = "balkon";

void setup() {
Serial.begin(115200);
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
  pinMode(rainDigital,INPUT);
}

void loop() {
 String pravdepodobnost = Firebase.getString("pravdepodobnost");
 float pravd_zrazok= pravdepodobnost.toFloat();
 int rainAnalogVal = analogRead(rainAnalog);
 int rainDigitalVal = digitalRead(rainDigital);
   
   if (pravd > 51 || rainDigitalVal == 1 ) {
   Firebase.setFloat ("prsi",1);
if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + pravd_zrazok + "&value2=" + rainDigitalVal +"";
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
  ESP.deepSleep(220e6); 

  }
  else {
    Serial.println("WiFi Disconnected");
  }
    }
   Firebase.setFloat ("prsi",0);
  ESP.deepSleep(220e6); 

}

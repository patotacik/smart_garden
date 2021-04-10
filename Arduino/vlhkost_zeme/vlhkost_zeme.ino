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

const int AirValue = 790;
const int WaterValue = 390;
#define FIREBASE_HOST "****.firebaseio.com"
#define FIREBASE_AUTH "****"
const int SensorPin = A0;

int soilMoistureValue = 0;
int soilmoisturepercent=0;

const char* ssid     = "****";
const char* password = "****";
const char* serverName = "****";
String apiKeyValue = "****";
String sensorName = "soil";
String sensorLocation = "rastlina";

BH1750 lightMeter;
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

}

void loop() {
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
soilMoistureValue = analogRead(SensorPin);  //put Sensor insert into soil
Serial.println(soilMoistureValue);
soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);

   
  if(soilmoisturepercent > 100){
  Serial.println("100 %");
    String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + "100" + "";
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
    Firebase.setFloat ("Poda",100);

  }

    if(soilmoisturepercent <0){
    Serial.println("0 %");
    String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + "0" + "";
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
    Firebase.setFloat ("Poda",0);
  }

  else if(soilmoisturepercent >=0 && soilmoisturepercent <= 100){
     Serial.println(soilmoisturepercent + " %");
     String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + String(soilmoisturepercent) + "";
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
        Firebase.setFloat ("Poda",soilmoisturepercent);

  }

  }
  else {
    Serial.println("WiFi Disconnected");
  }
  delay(600000);  

}

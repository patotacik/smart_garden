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
#include <math.h> 
float polomer_na_druhu = 17.64;
float objem = 0;
float celkom = 500;

#define FIREBASE_HOST "*****"
#define FIREBASE_AUTH "*****"
const char* ssid     = "*****";
const char* password = "*****";
const char* serverName = "*****";
String apiKeyValue = "*****";
String sensorName = "nadrz";
String sensorLocation = "Nádrž 1";

const int trigPin = D7;  
const int echoPin = D6;  

long duration;
int distance;

void setup() {
pinMode(trigPin, OUTPUT);
pinMode(echoPin, INPUT);
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
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
duration = pulseIn(echoPin, HIGH);
distance= duration*0.034/2;
objem = 3.14 * polomer_na_druhu * distance;
if( objem >500){
 objem = 500 ;
  }
Serial.print("Objem nádrže : ");
Serial.println(objem);
Serial.println(distance);
Firebase.setFloat ("stav_nadrze",objem);
int zavlazovanie = Firebase.getInt("zapnute_zavlazovanie");
float stav = celkom - objem;
if(zavlazovanie == 1){
if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + stav +"";
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
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}
delay(10000);
}

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
#include <FuzzyRule.h>
#include <FuzzyComposition.h>
#include <Fuzzy.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzyOutput.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzySet.h>
#include <FuzzyRuleAntecedent.h>
#include <SFE_BMP180.h>

SFE_BMP180 bmp180;
float alt = 300.0;

#define FIREBASE_HOST "****.firebaseio.com"
#define FIREBASE_AUTH "****"
const char* ssid     = "****";
const char* password = "****";

const char* serverName = "****";
String apiKeyValue = "****";
String sensorName = "zrazky";
String sensorLocation = "Lucenec";

int vlhkost;
int tlak;
float output;
float pravd;


Fuzzy* fuzzy = new Fuzzy();

// tlak vzduchu  
FuzzySet* nizky = new FuzzySet(0, 0, 800, 810);
FuzzySet* stredny = new FuzzySet(790, 800, 950, 960); 
FuzzySet* vyssoky = new FuzzySet(940, 950, 1200, 1200); 
// vlhkost vzduchu
FuzzySet* sucho = new FuzzySet(0, 0, 30, 35);
FuzzySet* normalne = new FuzzySet(25, 30, 60, 65);
FuzzySet* vlhko = new FuzzySet(55, 60, 100, 100);
// Fuzzy výstup pravdepodobnosti
FuzzySet* ziadna = new FuzzySet(0, 0, 0, 5);
FuzzySet* mala = new FuzzySet(2.5, 5, 25, 30);
FuzzySet* stredna = new FuzzySet(20, 25, 50, 55);
FuzzySet* vyssia = new FuzzySet(50, 50, 75, 80);
FuzzySet* vysoka = new FuzzySet(70, 75, 100, 100);


void setup() 
{  
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
  bool success = bmp180.begin();
  if (success) {
    Serial.println("BMP180 init success");
  }

  FuzzyInput* tlak = new FuzzyInput(1);

    tlak->addFuzzySet(nizky);
    tlak->addFuzzySet(stredny); 
    tlak->addFuzzySet(vyssoky);

    fuzzy->addFuzzyInput(tlak); 

    FuzzyInput* vlhkost = new FuzzyInput(2);

    vlhkost->addFuzzySet(sucho);
    vlhkost->addFuzzySet(normalne);
    vlhkost->addFuzzySet(vlhko);

    fuzzy->addFuzzyInput(vlhkost);

    FuzzyOutput* pravdepodobnost = new FuzzyOutput(1);

    pravdepodobnost->addFuzzySet(ziadna); 
    pravdepodobnost->addFuzzySet(mala); 
    pravdepodobnost->addFuzzySet(stredna); 
    pravdepodobnost->addFuzzySet(vyssia);
    pravdepodobnost->addFuzzySet(vysoka);

    fuzzy->addFuzzyOutput(pravdepodobnost);


    //fuzzy rule 
    FuzzyRuleConsequent* ziadna_p = new FuzzyRuleConsequent();
    ziadna_p->addOutput(ziadna);
    FuzzyRuleConsequent* mala_p = new FuzzyRuleConsequent();
    mala_p->addOutput(mala);
    FuzzyRuleConsequent* stredna_p = new FuzzyRuleConsequent(); 
    stredna_p->addOutput(stredna);  
    FuzzyRuleConsequent* vyssia_p = new FuzzyRuleConsequent(); 
    vyssia_p->addOutput(vyssia);
    FuzzyRuleConsequent* max_p = new FuzzyRuleConsequent(); 
    max_p->addOutput(vysoka);


//1 Tlak je nízky + vlhkosť veľká = vysoká pravdepodobnosť zrážok
    FuzzyRuleAntecedent* ifNizkyVlhko = new FuzzyRuleAntecedent();
    ifNizkyVlhko->joinWithAND(nizky, vlhko); 
    FuzzyRule* fuzzyRule01 = new FuzzyRule(1, ifNizkyVlhko, max_p);
    fuzzy->addFuzzyRule(fuzzyRule01); 

//2 Tlak je nízky + vlhkosť priemerná = vyššia pravdepodobnosť zrážok
    FuzzyRuleAntecedent* ifNizkyNormalne = new FuzzyRuleAntecedent(); 
    ifNizkyNormalne->joinWithAND(nizky, normalne); 
    FuzzyRule* fuzzyRule02 = new FuzzyRule(2, ifNizkyNormalne, vyssia_p);
    fuzzy->addFuzzyRule(fuzzyRule02);
//3
    FuzzyRuleAntecedent* ifNizkySucho = new FuzzyRuleAntecedent(); 
    ifNizkySucho->joinWithAND(nizky, sucho);
    FuzzyRule* fuzzyRule03 = new FuzzyRule(3, ifNizkySucho, stredna_p);
    fuzzy->addFuzzyRule(fuzzyRule03); 
//4
    FuzzyRuleAntecedent* ifStrednyVlhko = new FuzzyRuleAntecedent(); 
    ifStrednyVlhko->joinWithAND(stredny, vlhko);
    FuzzyRule* fuzzyRule04 = new FuzzyRule(4, ifStrednyVlhko, vyssia_p);
    fuzzy->addFuzzyRule(fuzzyRule04); 
//5
    FuzzyRuleAntecedent* ifStrednyNormalne = new FuzzyRuleAntecedent(); 
    ifStrednyNormalne->joinWithAND(stredny, normalne); 
    FuzzyRule* fuzzyRule05 = new FuzzyRule(5, ifStrednyNormalne, stredna_p); 
    fuzzy->addFuzzyRule(fuzzyRule05); 
//6
    FuzzyRuleAntecedent* ifStrednySucho = new FuzzyRuleAntecedent(); 
    ifStrednySucho->joinWithAND(stredny, sucho); 
    FuzzyRule* fuzzyRule06 = new FuzzyRule(6, ifStrednySucho, mala_p); 
    fuzzy->addFuzzyRule(fuzzyRule06); 
//7
    FuzzyRuleAntecedent* ifVyssokySucho = new FuzzyRuleAntecedent(); 
    ifVyssokySucho->joinWithAND(vyssoky, vlhko); 
    FuzzyRule* fuzzyRule07 = new FuzzyRule(7, ifVyssokySucho, stredna_p); 
    fuzzy->addFuzzyRule(fuzzyRule07); 
//8
    FuzzyRuleAntecedent* ifVyssokyNormalne = new FuzzyRuleAntecedent(); 
    ifVyssokyNormalne->joinWithAND(vyssoky, normalne); 
    FuzzyRule* fuzzyRule08 = new FuzzyRule(8, ifVyssokyNormalne, ziadna_p); 
    fuzzy->addFuzzyRule(fuzzyRule08);
//9
    FuzzyRuleAntecedent* ifVSucho = new FuzzyRuleAntecedent(); 
    ifVSucho->joinWithAND(vyssoky, sucho); 
    FuzzyRule* fuzzyRule09 = new FuzzyRule(9, ifVSucho, ziadna_p); 
    fuzzy->addFuzzyRule(fuzzyRule09);

  
}

void loop() 
{
char status;
  double T, P, seaLevelPressure;
  bool success = false;
  vlhkost = Firebase.getInt("figovnik_vlhkost_vz");
  String pravde = Firebase.getString("pravdepodobnost");
  pravd= pravde.toFloat();
  int dazd = Firebase.getInt("prsi");
  
  status = bmp180.startTemperature();

  if (status != 0) {
    delay(1000);
    status = bmp180.getTemperature(T);

    if (status != 0) {
      status = bmp180.startPressure(3);

      if (status != 0) {
        delay(status);
        status = bmp180.getPressure(P, T);

        if (status != 0) {
          seaLevelPressure = bmp180.sealevel(P, alt);

          Serial.print("Pressure at sea level: ");
          Serial.print(seaLevelPressure);
          Serial.println(" hPa");
          Firebase.setFloat ("tlak",seaLevelPressure);
        }
      }
    }
  }
  
  delay(2000);
  fuzzy->setInput(1, seaLevelPressure);
  fuzzy->setInput(2, vlhkost);
  fuzzy->fuzzify();
  output = fuzzy->defuzzify(1);
  Serial.print(output);
  Serial.print(" %");
  Serial.println("");
  
  Firebase.setFloat ("fuzzy_odhad",output);

if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    

 String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + output + "&value2=" + pravd + "&value3=" + dazd +"";
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
    // Free resources
    http.end();
}
else {
    Serial.println("WiFi Disconnected");
  }
  delay(100000);
  }

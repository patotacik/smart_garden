#include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <FirebaseArduino.h>
  #include <WiFiClient.h>
#include <SPI.h>
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


int relay1State = HIGH;
const int relay1 = D1;
int relayON = HIGH;
int relayOFF = LOW;

int Humidity;
int temperature;
float output;


#define FIREBASE_HOST "*****"
#define FIREBASE_AUTH "*****"
const char* ssid     = "*****";
const char* password = "*****";
const char* serverName = "*****";
String apiKeyValue = "*****";
String sensorName = "relay";
String sensorLocation = "Lucenec"; //   rubanisko3/34lucenec



Fuzzy* fuzzy = new Fuzzy();

FuzzySet* chladno = new FuzzySet(0, 0, 15, 25);
FuzzySet* normal = new FuzzySet(20, 25, 30, 35); 
FuzzySet* horuco = new FuzzySet(25, 30, 40, 40); 

FuzzySet* sucho = new FuzzySet(0, 0, 30, 35);
FuzzySet* normalne = new FuzzySet(25, 30, 60, 65);
FuzzySet* vlhko = new FuzzySet(55, 60, 90, 100);

FuzzySet* kratke_zavlazovanie = new FuzzySet(0, 2, 4, 5);
FuzzySet* priemerne_zavlazovanie = new FuzzySet(4, 5, 10, 11);
FuzzySet* dlhe_zavlazovanie = new FuzzySet(9, 10, 15, 16);


void setup() 
{  
  pinMode(relay1, OUTPUT);
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



 
  FuzzyInput* teplota = new FuzzyInput(1);

    teplota->addFuzzySet(chladno);
    teplota->addFuzzySet(normal); 
    teplota->addFuzzySet(horuco);

    fuzzy->addFuzzyInput(teplota); 


    FuzzyInput* vlhkost = new FuzzyInput(2);

    vlhkost->addFuzzySet(sucho);
    vlhkost->addFuzzySet(normalne);
    vlhkost->addFuzzySet(vlhko);

    fuzzy->addFuzzyInput(vlhkost);

    FuzzyOutput* dlzka_zavlazovania = new FuzzyOutput(1);

    dlzka_zavlazovania->addFuzzySet(kratke_zavlazovanie); 
    dlzka_zavlazovania->addFuzzySet(priemerne_zavlazovanie); 
    dlzka_zavlazovania->addFuzzySet(dlhe_zavlazovanie);
  
    fuzzy->addFuzzyOutput(dlzka_zavlazovania);


    //fuzzy rule 
    FuzzyRuleConsequent* thenfast = new FuzzyRuleConsequent();
    thenfast->addOutput(kratke_zavlazovanie);
    FuzzyRuleConsequent* thenAverage = new FuzzyRuleConsequent(); 
    thenAverage->addOutput(priemerne_zavlazovanie);  
    FuzzyRuleConsequent* thenLong = new FuzzyRuleConsequent(); 
    thenLong->addOutput(dlhe_zavlazovanie);



    //Je chladno a pôda je vlhka = krátka doba zavlažovania
    FuzzyRuleAntecedent* ifChladnoVlhko = new FuzzyRuleAntecedent();
    ifChladnoVlhko->joinWithAND(chladno, vlhko); 
    FuzzyRule* fuzzyRule01 = new FuzzyRule(1, ifChladnoVlhko, thenfast);
    fuzzy->addFuzzyRule(fuzzyRule01); 


    FuzzyRuleAntecedent* ifChladnoNormalne = new FuzzyRuleAntecedent(); 
    ifChladnoNormalne->joinWithAND(chladno, normalne);
    FuzzyRule* fuzzyRule02 = new FuzzyRule(2, ifChladnoNormalne, thenfast);
    fuzzy->addFuzzyRule(fuzzyRule02);

    FuzzyRuleAntecedent* ifChladnoSucho = new FuzzyRuleAntecedent(); 
    ifChladnoSucho->joinWithAND(chladno, sucho);

    FuzzyRule* fuzzyRule03 = new FuzzyRule(3, ifChladnoSucho, thenAverage);
    fuzzy->addFuzzyRule(fuzzyRule03); 

    FuzzyRuleAntecedent* ifNormalVlhko = new FuzzyRuleAntecedent(); 
    ifNormalVlhko->joinWithAND(normal, vlhko);

    FuzzyRule* fuzzyRule04 = new FuzzyRule(4, ifNormalVlhko, thenfast);
    fuzzy->addFuzzyRule(fuzzyRule04); 

    FuzzyRuleAntecedent* ifNormalNormalne = new FuzzyRuleAntecedent(); 
    ifNormalNormalne->joinWithAND(normal, normalne); 

    FuzzyRule* fuzzyRule05 = new FuzzyRule(5, ifNormalNormalne, thenAverage); 
    fuzzy->addFuzzyRule(fuzzyRule05); 

    FuzzyRuleAntecedent* ifNormalSucho = new FuzzyRuleAntecedent(); 
    ifNormalSucho->joinWithAND(normal, sucho); 
 
    FuzzyRule* fuzzyRule06 = new FuzzyRule(6, ifNormalSucho, thenLong); 
    fuzzy->addFuzzyRule(fuzzyRule06); 

    FuzzyRuleAntecedent* ifHorucoSucho = new FuzzyRuleAntecedent(); 
    ifHorucoSucho->joinWithAND(horuco, vlhko); 

    FuzzyRule* fuzzyRule07 = new FuzzyRule(7, ifHorucoSucho, thenAverage); 
    fuzzy->addFuzzyRule(fuzzyRule07); 

    FuzzyRuleAntecedent* ifPanasLembab = new FuzzyRuleAntecedent(); 
    ifPanasLembab->joinWithAND(horuco, normalne); 
      
    FuzzyRule* fuzzyRule08 = new FuzzyRule(8, ifPanasLembab, thenAverage); 
    fuzzy->addFuzzyRule(fuzzyRule08);
  
    FuzzyRuleAntecedent* ifPanasKering = new FuzzyRuleAntecedent(); 
    ifPanasKering->joinWithAND(horuco, sucho); 

    FuzzyRule* fuzzyRule09 = new FuzzyRule(9, ifPanasKering, thenLong); 
    fuzzy->addFuzzyRule(fuzzyRule09);

}

void loop() 
{
  int temperature = Firebase.getInt("figovnik_teplota_vz");
  int soilMoister = Firebase.getInt("vlhkost_zeme");
  int max_vlh = Firebase.getInt("Kolko");
  int max_prav = Firebase.getInt("Max_prav"); 
  String pravde = Firebase.getString("pravdepodobnost");
  float pravd= pravde.toFloat();
  int prsi = Firebase.getInt("prsi");
  int max_svet = Firebase.getInt("Max_sviet");
  int svetlo = Firebase.getInt("light");
  int zapnut = Firebase.getInt("aktivne_zavlazovanie");


Serial.print("max_vlh: ");
    Serial.println(max_vlh);

 Serial.print("Max_prav: ");
    Serial.println(max_prav);
    

 Serial.print("pravd: ");
    Serial.println(pravd);

     Serial.print("prsi: ");
    Serial.println(prsi);

Serial.print("max_svet: ");
    Serial.println(max_svet);
   
Serial.print("svetlo: ");
    Serial.println(svetlo);



  int dur = ((output*1000)/1000);
  delay(2000);

  
  fuzzy->setInput(1, temperature);
  fuzzy->setInput(2, soilMoister);

  fuzzy->fuzzify();
  output = fuzzy->defuzzify(1);




  if( pravd < max_prav && prsi == 0 && svetlo <= max_svet && zapnut == 1 && soilMoister <= max_vlh){
      
     Firebase.setFloat ("zapnute_zavlazovanie",1);
     Firebase.setFloat ("dlzka_zavlazovania",output);


if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    
     Firebase.setFloat ("figovnik_zavlazovanie",1);
  String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + "1" +"&value2=" + pravd + "&value3=" + soilMoister +"&value4=" + output +"" ;
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
  digitalWrite(relay1, relayON);
  delay(output*10000);
  delay(2000);
  
  digitalWrite(relay1, relayOFF);

}

  else{
  
  digitalWrite(relay1, relayOFF);
  Firebase.setFloat ("zapnute_zavlazovanie",0);
  Serial.println("Zavlažovanie je vypnuté");
  delay(2000);
  }
}

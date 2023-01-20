#include <Ticker.h>

// https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/

#include "Arduino.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define LED_BUILTIN 2 
void ledStatOn() {
  digitalWrite(LED_BUILTIN, LOW);
}
void ledStatOff() {
  digitalWrite(LED_BUILTIN, HIGH);
}

#define LED_ERR 4 // GPIO4 i.e. D2 on NodeMCU mini
void blinkErrLed(int howLong) {
  Serial.println("Error for ms " + String(howLong)); 
  digitalWrite(LED_ERR, HIGH);
  delay(howLong);
  digitalWrite(LED_ERR, LOW);  
}

//const char* ssid = "Orange-Ma-Dwa";
//const char* password = "HRL34HRAJSK3";
const char* ssid = "Trzy-Ma-Nasz";
const char* password = "2zaef6fz57aw";

 #define SETTING_PIN 13 // D7

 unsigned long errPeriod = 10000; // 30000 30 secs
 unsigned long vReadPeriod = 60000;

 int timerScaler = 2; // Ticker can't handle timer values bigger then 4,2G microsecs, 
                      // so we use the trick to split the timer value and call timer function several times
                      // but only once we do the actual work 
 unsigned long maxReadPeriod = timerScaler*4200000; // secs

 Ticker timer;

void setup() {
  Serial.begin(115200);

  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_ERR, OUTPUT);
  pinMode(SETTING_PIN, INPUT_PULLUP);

  ledStatOn();
 
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  ledStatOff();
  
  int switchVal = digitalRead(SETTING_PIN);  
  Serial.println("switchVal is "+String(switchVal));
  if (switchVal == 0) { // occurs on closed pin, what is illogical, anyway, if closed it's testing mode
    errPeriod = 3*1000; 
    vReadPeriod = 30*1000;
  } else {
    errPeriod = 30*1000; 
    vReadPeriod = 2*3600*1000;
  }

  // schedule reading triggering
  // internally timer uses microsecs, and since we use secs and multiple by 1000, we need to ensure
  // we don't exceed ulong range 4,2G
  if (vReadPeriod <= maxReadPeriod) {
    timer.attach(vReadPeriod/timerScaler/1000, triggerReading); // ticker is in secs 
    Serial.println("ErrPeriod "+String(errPeriod/1000.0,0)+" vReadPeriod "+String(vReadPeriod/1000.0,0)+" sec");
  }
  else {
    Serial.println("vReadPeriod too big, can't be bigger then"+String(maxReadPeriod/1000.0,0)+" sec");
  }
   
  // blink the error led to check whether the led can function
  blinkErrLed(1000);
}

const char* dmtUrl = "http://pi3kodi:8080/json.htm?type=command&param=udevice&idx=16&nvalue=0&svalue=";

bool sendToServer(float volts) {
  if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      String serverPath = dmtUrl + String(volts,3);
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverPath.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode<=0) {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();

      return (httpResponseCode>0);
    }
    else {
      Serial.println("WiFi Disconnected");
      return false;
    }
}

int PIN_V = A0; //Analog channel A0 as used to measure battery voltage
float readVolts(float vMultiplier) {
  float vSum = 0;
  float v, v33, resV, v1;

  for (unsigned int i=0; i<10; i++) {
    v1 = analogRead(PIN_V);
    vSum = vSum + v1; //Read analog Voltage
    Serial.print("Single r:");
    Serial.print(v1);
    Serial.print(";");
    delay(5);                        //ADC stable
  }
  Serial.println("");
  v=(float)vSum/10.0; //Find average of 10 values
  Serial.print("Avg. reading from A0 is ");
  Serial.println(v,3); 

  v33=round(1000.0*(v/1078.936)*3.3)/1000.0; //Convert Voltage from 3.3v factor
  Serial.print("Volt. from A0 is ");
  Serial.println(v33,3);

  resV = v33 * vMultiplier; 
  resV = round(resV * 100.0)/100.0; // skip obsolete precision, we want just few digits after decimal
  Serial.print("Recalc.  volt. is ");
  Serial.println(resV,3);
  return resV;
}

// vDivRatio is from experiments, the theoretical value was lower (cca 4.3)
const float vDivRatio = 4.513494318; 

void readsendVolts()
{
  float volts = readVolts(vDivRatio);
  if (!sendToServer(volts))
  {
    blinkErrLed(errPeriod);
  }
}


int timerCounter = timerScaler-1; 
bool doReading = true; // so that we run callback on startup
// callback function called from Ticker can not use network, file, serial etc.
// that's why we just set the flag and check it in the loop

// timer callback will be called more often (due to the scaler) then we want
// so we count the calls and do the reading on 0 count (then reset the counter)
void triggerReading() {
  if (timerCounter == 0) {
    doReading = true;
    timerCounter = timerScaler-1; 
  } else {
    timerCounter--;
  }
}


void loop() {
  if (doReading) {
    readsendVolts();
    doReading = false;
  } 
}

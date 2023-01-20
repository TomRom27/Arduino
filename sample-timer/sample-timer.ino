#include "Arduino.h"
#include <Ticker.h>

Ticker timer;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  timer.attach(10, doSomething); 
}

int counter = 0;
void doSomething() {
    Serial.print("Scheduled proc called ");
    Serial.println(++counter);

}

void loop() {
  // put your main code here, to run repeatedly:

}

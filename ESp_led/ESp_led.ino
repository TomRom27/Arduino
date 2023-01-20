/* example from  http://www.arduinesp.com/examples  */
/* to be flashed directly to ESP8266 */
int ledPin = 2;
void setup() 
{
pinMode(ledPin, OUTPUT);
}
void loop()
{
digitalWrite(ledPin, HIGH); // turn the LED on (HIGH is the voltage level)
delay(1000); // wait for a second
digitalWrite(ledPin, LOW); // turn the LED off by making the voltage LOW
delay(1000); // wait for a second
}

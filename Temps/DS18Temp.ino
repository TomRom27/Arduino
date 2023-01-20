/********************************************************************/
// First we include the libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

/********************************************************************/
// Used for de- and serialization to json
const char func_field[] = "func";
const char value_field[] = "value";
const char active_field[] = "active";
const char temp_func[] = "temp";

//{"func":"temp","active":"true"}
//{"func":"temp","active":"false"}
/********************************************************************/
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
bool readTemp = true;
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
/********************************************************************/
// json buffer for serialization
// see more https://arduinojson.org/v5/example/generator/
StaticJsonBuffer<50> jsonBuffer;
// It's a reference to the JsonObject, the actual bytes are inside the
// JsonBuffer with all the other nodes of the object tree.
// Memory is freed when jsonBuffer goes out of scope.
JsonObject& tempJson = jsonBuffer.createObject();
/********************************************************************/


void readTempAndOutput(void)
{
  // call sensors . requestTemperatures() to issue a global temperature request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperature readings
  tempJson[func_field] = temp_func;
  // You can have more than one DS18B20 on the same bus.
  // 0 refers to the first IC on the wire
  tempJson[value_field] = sensors.getTempCByIndex(0);
  tempJson[active_field] = readTemp;
  tempJson.printTo(Serial);
  Serial.println("");
}

void processInput(String inputLine)
{
  StaticJsonBuffer<100> jb;
    // we expect something like that
    // {"func":"temp","active":"true"}
    // {"func":"temp","active":"false"}
    
    JsonObject& obj = jb.parseObject(inputLine);
    if (obj.success()) {
      const char* func = obj.get<char*>(func_field);
      bool active = obj.get<bool>(active_field);

      if (strcmp(func,temp_func)==0) {
        if (active) {
          readTemp = true;
        }
        else {
          readTemp = false;
        }
      }
      Serial.print("f is=");
      Serial.print(func);
      Serial.print(" and a is=");
      Serial.print(active);
      Serial.println();
    }
    else {
      Serial.print("parsing failed");//log
      Serial.print(inputLine);
      Serial.println();
    }
}

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  // Start up the library
  sensors.begin();
}


void loop(void)
{
    // each input must end with end of line
    if(Serial.available()){
      processInput(Serial.readStringUntil("\n"));
    }

  if (readTemp) {
    readTempAndOutput();
  }

  delay(500);
}

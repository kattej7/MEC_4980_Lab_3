#include <Arduino.h>
#define MICRO
#include <stdint.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SparkFun_Qwiic_OLED.h>

Adafruit_BME280 bme;

#if defined(TRANSPARENT)
QwiicTransparentOLED myOLED;
const char *deviceName = "Transparent OLED";

#elif defined(NARROW)
QwiicNarrowOLED myOLED;
const char *deviceName = "Narrow OLED";

#else
QwiicMicroOLED myOLED;
const char *deviceName = "Micro OLED";

#endif

int yoffset;
float targetTemperature = 20.0;
char degreeSys[] = "C";

int pinButton = A1;
int pinButtonUp = A2;
int pinButtonDown = A3;
int heaterLED = A0;

bool prevPressed = false;
bool prevUp = false;
bool prevDown = false;

enum MachineStates
{
  DisplayTemps, // 0
  SetTemp,      // 1
  ChooseSystem  // 2
};

MachineStates currentState;

void setup()
{
  currentState = DisplayTemps;
  pinMode(pinButton, INPUT_PULLDOWN);
  pinMode(pinButtonUp, INPUT_PULLDOWN);
  pinMode(pinButtonDown, INPUT_PULLDOWN);
  pinMode(heaterLED, OUTPUT);

  Serial.begin(115200);
  delay(1000);
  Serial.println("Test");

  if (!bme.begin(0x77, &Wire))
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }

  delay(500);

  Serial.println("\n\r-----------------------------------");

  Serial.print("Running Test #5 on: ");
  Serial.println(String(deviceName));

  if (!myOLED.begin())
  {
    Serial.println("- Device Begin Failed");
    while (1)
      ;
  }

  yoffset = (myOLED.getHeight() - myOLED.getFont()->height) / 2;

  delay(1000);
}

float cToF(float degC)
{
  return (degC * 9.0 / 5.0) + 32.0;
}

void loop()
{
  bool currPressed = !digitalRead(pinButton);
  bool currUp = !digitalRead(pinButtonUp);
  bool currDown = !digitalRead(pinButtonDown);
  float currentTemp = bme.readTemperature();

  if (currPressed && !prevPressed)
  {
    currentState = MachineStates(((int)currentState + 1) % 3);
  }

  char myNewText[50];

  if (currentState == DisplayTemps)
  {
    if (degreeSys[0] == 'C')
    {
      sprintf(myNewText, "Tc: %.1f C", currentTemp);
    }
    else
    {
      sprintf(myNewText, "Tf: %.1f F", cToF(currentTemp));
    }
    myOLED.erase();
    myOLED.text(3, yoffset, myNewText);
    myOLED.display();
  }
  else if (currentState == ChooseSystem)
  {
    if (currUp && !prevUp)
    {
      degreeSys[0] = 'C';
    }
    else if (currDown && !prevDown)
    {
      degreeSys[0] = 'F';
    }
    sprintf(myNewText, "System: %s", degreeSys);
    myOLED.erase();
    myOLED.text(3, yoffset, myNewText);
    myOLED.display();
  }
  else if (currentState == SetTemp)
  {
    if (currUp && !prevUp)
    {
      targetTemperature++;
    }
    else if (currDown && !prevDown)
    {
      targetTemperature--;
    }
    if (degreeSys[0] == 'C')
    {
      sprintf(myNewText, "Ttar: %.1f C", targetTemperature);
    }
    else
    {
      sprintf(myNewText, "Ttar: %.1f F", cToF(targetTemperature));
    }

    myOLED.erase();
    myOLED.text(3, yoffset, myNewText);
    myOLED.display();
  }

  if (currentTemp > targetTemperature)
  {
    digitalWrite(heaterLED, LOW);
  }
  else
  {
    digitalWrite(heaterLED, HIGH);
  }

  prevPressed = currPressed;
  prevUp = currUp;
  prevDown = currDown;

  delay(50);
}
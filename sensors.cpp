#include "sensors.h"

Adafruit_VL53L0X waterLevelSensor = Adafruit_VL53L0X();
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temperatureSensor(&oneWire);

float waterLevelCm = -1;
float tempC = -127;
float lux = 0;

void setupSensors() {
  if (!waterLevelSensor.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }
  temperatureSensor.begin();
  pinMode(LIGHTSENSORPIN, INPUT);
}

void readAllSensors() {

  readWaterLevel();

  readTemp();

  readLux();
  
}

void readWaterLevel() {
  VL53L0X_RangingMeasurementData_t measure;
  waterLevelSensor.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    float distanceToWater = measure.RangeMilliMeter / 10.0;
    waterLevelCm = 45.0 - distanceToWater;
  } else {
    waterLevelCm = -1;
  }
}

void readTemp() {
  temperatureSensor.requestTemperatures();
  tempC = temperatureSensor.getTempCByIndex(0);
}

void readLux() {
  int lightReading = analogRead(LIGHTSENSORPIN);
  float volts = lightReading * 5.0 / 1024.0;
  float amps = volts / 10000.0;
  float microamps = amps * 1000000;
  lux = microamps * 2.0;
}

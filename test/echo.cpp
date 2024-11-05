#include <Arduino.h>

#define TE_POWER 14

void setup()
{
    // USB Serial
    Serial.begin();

    // Serial Pins 0, 1
    Serial1.begin(115200);

    pinMode(TE_POWER, OUTPUT);
    digitalWrite(TE_POWER, LOW);
    delay(2000);
    digitalWrite(TE_POWER, HIGH);
    Serial.println("SIM7080G should be powered on now!");
}

void loop()
{
    if(Serial.available())
        Serial1.write(Serial.read());

    if(Serial1.available())
        Serial.write(Serial1.read());
}
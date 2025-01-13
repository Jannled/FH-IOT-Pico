/**
 * @file main.cpp
 * @author Jannik Schmöle (@Jannled)
 * @brief Serial passthrough for the Waveshare Pico-SIM7080G-Cat-M module
 * @version 0.1
 * @date 2024-10-11
 * 
 * @copyright MIT License
 */

#include <Arduino.h>
#include <Wire.h>

// URLs, Passwords etc.
#include "secrets.h"

#include "SimComModem.hpp"

SimComModem modem;

const std::vector<char*> carriers = {"Telekom.de", "vodafone.de", "o2 - de"};

void switchCarrier(const char* operatorName)
{
	SERIAL_MODEM.printf("AT+COPS=4,0,\"%s\"\r\n", operatorName); // Mode: 4 = Semi Automatic, Format: 0 = Long Operator

	volatile int i=0;
	while(SERIAL_MODEM.readString().length() < 1)
	{
		i++;
	}
}

void setup() 
{
	modem.init(115200);
	modem.powerOnSequence();

	Serial.begin(115200); // USB UART
	delay(2000);

	Serial.println("Waiting for Modem to come Online");
	modem.echoAT("AT+GMI");
	for(int i=0; i<10; i++)
	{
		//Serial.print("Modem is booting...");
		modem.sendAT("AT");
		String foo = modem.readStringUntil('\r');
		if(foo.length() < 1)
			Serial.print(".");
		else
			Serial.println(foo);
	}
	Serial.println();

	SERIAL_MODEM.readString();

	modem.echoAT("ATI");
	modem.echoAT("AT+GMI");
	modem.echoAT("AT+GSN");
	modem.echoAT("AT+CPIN?");
	modem.echoAT("AT+CFUN?");
	modem.echoAT("AT+CSCLK?");

	// Preferred Selection between CAT-M and NB-IoT
	modem.echoAT("AT+CMNB=2");

	// Select the first operator
	const char* opr = carriers[0];
	Serial.printf("[%8lu] %s%s\r\n", millis(), "AT+COPS=", opr);
	switchCarrier(opr);
}

void loop() 
{
	static unsigned long lastMeasurement = millis();
	static unsigned long lastOperatorChange = millis();

	static uint8_t opr_idx = 0;

	// Make sure all data from Modem is printed
	while(modem.available())
		Serial.write((uint8_t) modem.read());

	const unsigned long now = millis();

	// Switch Operators
	if (now - lastOperatorChange > (30 * 1000))
	{
		const char* opr = carriers[opr_idx];
		opr_idx = (opr_idx + 1) % carriers.size();

		Serial.println();
		Serial.println(" --------------------------------------------------------------");
		Serial.println();

		Serial.printf("[%8lu] %s%s\r\n", millis(), "AT+COPS=", opr);
		switchCarrier(opr);
		lastOperatorChange = now;
	}

	// Ask about RSSI and SNR
	if (now - lastMeasurement > 2500)
	{
		Serial.printf("[%8lu] %s\r\n", millis(), "AT+CPSI?");
		modem.sendAT("AT+CENG?");
		lastMeasurement = now;
	}
}

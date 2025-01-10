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

#define NBIOT

#define SERIAL_MODEM Serial1

// Pull down to wake Modem during sleep
#define PIN_MODEM_WAKE 17

// Pull down to cut modem power
#define PIN_MODEM_SLEEP 14

#define CRLF "\r\n"

void sendATCommand(const char* command)
{
	SERIAL_MODEM.println(command);
	String foo = SERIAL_MODEM.readStringUntil('\n');
	Serial.print(command);
	Serial.print(": ");
	foo.trim();
	Serial.println(foo);
}

int send_at(const char* command, const char* expectedResult)
{
	SERIAL_MODEM.println(command);
	String foo;
	for(int i=0; i<10; i++)
	{
		foo = SERIAL_MODEM.readString();
		foo.trim();

		if(foo.length() > 0)
			break;
	}

	Serial.println("---");
	Serial.print("FOO: ");
	Serial.println(foo);
	Serial.println("---");
	return foo.compareTo(expectedResult);
}

void setup() 
{
	pinMode(PIN_MODEM_WAKE, OUTPUT);
	pinMode(PIN_MODEM_SLEEP, OUTPUT);

	Serial.begin(115200); // USB UART
	SERIAL_MODEM.begin(115200); // SIM7080G-Cat-M module

	delay(2000);
	Serial.println("Hello World");

	// Wake up Modem
	delay(6000);
	digitalWrite(PIN_MODEM_SLEEP, HIGH);
	digitalWrite(PIN_MODEM_WAKE, LOW);

	Serial.println("AT");
	Serial.setTimeout(5000);

	for(int i=0; i<5; i++)
	{
		SERIAL_MODEM.println("AT");
		String foo = SERIAL_MODEM.readStringUntil('\n');
		Serial.println(foo);
	}

	SERIAL_MODEM.readString();

	Serial.printf("Response: %d" CRLF, send_at("AT", "AT\r\r\nOK"));
	delay(1000);

	sendATCommand("ATI");
	delay(1000);	
	sendATCommand("AT+GMI");
	delay(1000);
	sendATCommand("AT+GSN");
	delay(1000);
	sendATCommand("AT+CPIN?");
	delay(1000);
}

unsigned long lastPublish = millis();

void loop() 
{
	while(SERIAL_MODEM.available())
		Serial.write((uint8_t) SERIAL_MODEM.read());

	const unsigned long now = millis();
	if (now - lastPublish > 1000)
	{
		Serial.printf("[%8lu] %s\r\n", millis(), "AT+CSQ");
		SERIAL_MODEM.println("AT+CSQ");
		lastPublish = now;
	}
}

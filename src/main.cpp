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

// Pull down to wake Modem during sleep
#define PIN_MODEM_WAKE 17

// Pull down to cut modem power
#define PIN_MODEM_SLEEP 14

void sendATCommand(const char* command)
{
	Serial1.println(command);
	String foo = Serial1.readString();
	Serial.print(command);
	Serial.print(": ");
	Serial.println(foo);
}

void setup() 
{
	pinMode(PIN_MODEM_WAKE, OUTPUT);
	pinMode(PIN_MODEM_SLEEP, OUTPUT);

	Serial.begin(115200); // USB UART
	Serial1.begin(115200); // SIM7080G-Cat-M module

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
		Serial1.println("AT");
		String foo = Serial1.readStringUntil('\n');
		Serial.println(foo);
	}

	Serial1.readString();

	sendATCommand("ATI");
	
	#ifdef NBIOT
	sendATCommand("AT+GMI");
	sendATCommand("AT+GSN");
	sendATCommand("AT+CPIN?");
	sendATCommand("AT+CIFSR");

	for(int i=0; i<10; i++)
	{
		sendATCommand("AT+CSQ");
		delay(1000);

		sendATCommand("AT");
		delay(1000);
	}
	#else
	sendATCommand("AT+CGNSPWR=1");

	delay(500);

	sendATCommand("AT+CGNSPWR=1");

	for(int i=0; i<100; i++)
	{
		sendATCommand("AT");
		delay(1000);

		sendATCommand("AT+CGNSINF");
		delay(1000);
	}
	#endif


	/*for(int i=0; i<2; i++)
	{
		Serial1.println("ATI");
		for(int j=0; j<3; j++)
		{
			String foo = Serial1.readStringUntil('\n');
			Serial.println(foo);
		}
		delay(100);
	}

	for(int i=0; i<5; i++)
	{
		Serial1.println("AT+GMI");
		for(int j=0; j<3; j++)
		{
			String foo = Serial1.readStringUntil('\n');
			Serial.println(foo);
		}
		delay(2000);
	}

	for(int i=0; i<5; i++)
	{
		Serial1.println("AT+GSN");
		for(int j=0; j<3; j++)
		{
			String foo = Serial1.readStringUntil('\n');
			Serial.println(foo);
		}
		delay(2000);
	}*/
}

void loop() 
{
	
}

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
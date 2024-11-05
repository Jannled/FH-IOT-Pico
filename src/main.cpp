/**
 * @file main.cpp
 * @author Jannik Schmöle (@Jannled)
 * @brief Serial passthrough for the Waveshare Pico-SIM7080G-Cat-M module
 * @version 0.1
 * @date 2024-10-11
 * 
 * 
 * 
 * @copyright MIT License
 */

#include <Arduino.h>

// Pull down to wake Modem during sleep
#define PIN_MODEM_WAKE 17

// Pull down to cut modem power
#define PIN_MODEM_SLEEP 14

#define NBIOT

void sendATCommand(const char* command)
{
	Serial1.println(command);
	String foo = Serial1.readString();
	Serial.print(command);
	Serial.print(": ");
	Serial.println(foo);
}

uint8_t send_at(const char* command, const char* expectedResult)
{
	Serial1.println(command);
	String foo = Serial1.readString();
	foo.trim();

	return foo.compareTo(expectedResult) == 0;
}

void waveshare_example()
{
	// Request Manufacturer Identification
	sendATCommand("AT+GMI");

	// Request TA Serial Number Identification (IMEI)
	sendATCommand("AT+GSN");

	sendATCommand("AT+CPIN?");

	if(!send_at("AT+CPIN?", "READY"))
		Serial.println("No SIM inserted");

	Serial.print("Connecting ");
	for(int i=0; i<10; i++)
	{
		if(!send_at("AT+CGATT?", "1"))
		{
			Serial.print(".");
			delay(500);
		}
		else
		{
			Serial.println("Connected");
			break;
		}
	}

	sendATCommand("AT+CSQ");
	sendATCommand("AT+CPSI?");
    sendATCommand("AT+COPS?");
}

void my_stuff()
{
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

	// Request Manufacturer Identification
	sendATCommand("AT+GMI");

	// Request TA Serial Number Identification (IMEI)
	sendATCommand("AT+GSN");

	// Request Complete TA Capabilities List
	sendATCommand("AT+GCAP");

	// Enter PIN
	sendATCommand("AT+CPIN?");

	// Operator Selection
	sendATCommand("AT+COPS?");

	// Network Registration
	sendATCommand("AT+CREG?");
	// +CREG: <n>,<stat>[,<lac>,<ci>,<netact>]
	// stat: 
	//   1 Registered, home network
	//   2 Not registered, but MT is currently searching a new operator to register to
	//   3 Registration denied
	//   4 Unknown
	//   5 Registered, roaming

	// Attach or detach from GPRS service
	sendATCommand("AT+CGATT?");

	// GNSS Power Control
	sendATCommand("AT+CGNSPWR?");

	// 
	sendATCommand("AT+CGCONTRDP");
	
	//CSOC

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

void setup() 
{
	pinMode(PIN_MODEM_WAKE, OUTPUT);
	pinMode(PIN_MODEM_SLEEP, OUTPUT);

	Serial.begin(115200); // USB UART
	Serial1.begin(115200); // SIM7080G-Cat-M module
	Serial.setTimeout(5000);

	// Wake up Modem
	delay(500);
	digitalWrite(PIN_MODEM_SLEEP, HIGH);
	digitalWrite(PIN_MODEM_WAKE, LOW);

	delay(6000);
	Serial.println("Hello World");

	// Prime the connection
	Serial1.println("AT");
	Serial1.readString();

	waveshare_example();
}

void loop() 
{
	
}

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

// Documentation: https://github.com/finitespace/BME280
#include <BME280.h>
#include <BME280I2C.h>

// Documentation: https://github.com/hideakitai/MsgPack
#include <MsgPack.h>

#define NBIOT

#define SERIAL_MODEM Serial1

// Pull down to wake Modem during sleep
#define PIN_MODEM_WAKE 17

// Pull down to cut modem power
#define PIN_MODEM_SLEEP 14

#define CRLF "\r\n"

// TODO implement IMEI parsing
const uint32_t deviceID = 42;

// BME Sensor
BME280I2C sensor;

struct SensorMessage {
	// The actual data
	float temperature;
	float humidity;
	float pressure;
	uint8_t rssi;
	uint32_t senderID;

	// Namen in Anlehnung an RFC 8428
	MsgPack::str_t key_temp = "Cel";
	MsgPack::str_t key_hum = "%RH";
	MsgPack::str_t key_press = "Pa";
	MsgPack::str_t key_id = "bn";
	MsgPack::str_t key_rssi = "dB";

	// Build the message
	MSGPACK_DEFINE_MAP(
		key_temp, temperature,
		key_hum, humidity,
		key_press, pressure,
		key_rssi, rssi,
		key_id, senderID
	);
};


// MessagePack transport format
MsgPack::Packer packer;
SensorMessage sensorMessage;

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

	// Init Temperature/Humidity Sensor
	Wire.begin();
	sensor.begin();

	delay(2000);
	Serial.println("Starting Modem...");

	// Wake up Modem
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
unsigned long lastI2C = millis();

void loop() 
{
	while(SERIAL_MODEM.available())
		Serial.write((uint8_t) SERIAL_MODEM.read());

	const unsigned long now = millis();
	if (now - lastPublish > 1000)
	{
		Serial.printf("[%8lu] %s\r\n", millis(), "AT+CPSI?");
		SERIAL_MODEM.println("AT+CPSI?");
		lastPublish = now;
	}

	if(now - lastI2C > 5000)
	{
		float temperature(NAN), humidity(NAN), pressure(NAN);
		sensor.read(pressure, temperature, humidity);

		Serial.printf("{\"temp\": %f, \"hum\": %f, \"press\": %f}\r\n", temperature, humidity, pressure);
		//Serial.printf("%.2f°C \r\n", temperature);
		
		// Print MessagePack debug output
		// Online Decoder: https://msgpack.solder.party/
		sensorMessage.temperature = temperature;
		sensorMessage.humidity = humidity;
		sensorMessage.pressure = pressure;
		sensorMessage.senderID = deviceID;
		packer.serialize(sensorMessage);
		for(size_t i=0; i<packer.size(); i++)
			Serial.printf("%02X ", packer.data()[i]);
		Serial.println();
		packer.clear();

		lastI2C = now;
	}
}

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

#ifdef ENABLE_COAP
// Documentation: https://github.com/finitespace/BME280
#include <BME280.h>
#include <BME280I2C.h>

// Documentation: https://github.com/hideakitai/MsgPack
#include <MsgPack.h>

// TODO implement IMEI parsing
const uint32_t deviceID = 1337;

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
#endif // ENABLE_COAP


SimComModem modem;

void setup() 
{
	modem.init(115200);
	modem.powerOnSequence();

	Serial.begin(115200); // USB UART
	
	// Init Temperature/Humidity Sensor
	Wire.begin();
	#ifdef ENABLE_COAP
	sensor.begin();
	#endif

	modem.wakeup();

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

	modem.echoAT("AT+CEREG=1");
	modem.echoAT("AT+CGNAPN");

	Serial.println("Setting APN:");
	modem.echoAT("AT+CNCFG=0,1,\"" APN "\"");

	Serial.println("Checking if it worked:");
	modem.echoAT("AT+CNCFG?");

	// Init CoAP
	modem.flush();
	modem.initCoAP();
	modem.flush();
}

unsigned long lastPublish = millis();
unsigned long lastI2C = millis();

void loop() 
{
	// Make sure all data from Modem is printed
	while(modem.available())
		Serial.write((uint8_t) modem.read());

	// Ask about RSSI and SNR
	const unsigned long now = millis();
	if (now - lastPublish > 2500)
	{
		Serial.printf("[%8lu] %s\r\n", millis(), "AT+CPSI?");
		modem.sendAT("AT+CENG?");
		lastPublish = now;
	}

	// Trigger measurement and send packet
	#ifdef ENABLE_COAP
	if(now - lastI2C > 30000)
	{
		// Read BME280 Sensor
		float temperature(NAN), humidity(NAN), pressure(NAN);
		sensor.read(pressure, temperature, humidity);

		// Build a JSON object
		char buff[128] = {};
		snprintf(
			buff, sizeof(buff) - 1, 
			"{\"temp\": %.2f, \"hum\": %.2f, \"press\": %.4f, \"bn\": %d}", 
			temperature, humidity, pressure, deviceID
		);
		Serial.println(buff);

		// Send JSON in plain text
		modem.sendPacket(COAP_URL, "sensor/data", buff);
		
		// Build MessagePack
		sensorMessage.temperature = temperature;
		sensorMessage.humidity = humidity;
		sensorMessage.pressure = pressure;
		sensorMessage.senderID = deviceID;
		packer.serialize(sensorMessage);

		// Print MessagePack debug output
		// Online Decoder: https://msgpack.solder.party/
		for(size_t i=0; i<packer.size(); i++)
			Serial.printf("%02X ", packer.data()[i]);
		Serial.println();

		//modem.sendPacket(COAP_URL, "sensor/data", packer.data(), packer.size());

		// Clear the store, otherwise the next packet will be concatenated
		packer.clear();

		lastI2C = now;
	}
	#endif
}

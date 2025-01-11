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

#include "SimComModem.hpp"

#ifdef ENABLE_COAP
// Documentation: https://github.com/finitespace/BME280
#include <BME280.h>
#include <BME280I2C.h>

// Documentation: https://github.com/hideakitai/MsgPack
#include <MsgPack.h>

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
#endif // ENABLE_COAP


SimComModem modem;

void sendATCommand(const char* command)
{
	SERIAL_MODEM.println(command);
	String foo = SERIAL_MODEM.readStringUntil('\n');
	Serial.print(command);
	Serial.print(": ");
	foo.trim();
	Serial.println(foo);
}

void setup() 
{
	modem.init(115200);
	Serial.begin(115200); // USB UART
	
	// Init Temperature/Humidity Sensor
	Wire.begin();
	#ifdef ENABLE_COAP
	sensor.begin();
	#endif

	modem.wakeup();

	delay(2000);
	modem.wakeup();

	for(int i=0; i<5; i++)
	{
		SERIAL_MODEM.println("AT");
		String foo = SERIAL_MODEM.readStringUntil('\n');
		Serial.println(foo);
	}

	SERIAL_MODEM.readString();
	//modem.sleep();

	sendATCommand("ATI");
	delay(500);	
	sendATCommand("AT+GMI");
	delay(500);
	sendATCommand("AT+GSN");
	delay(500);
	sendATCommand("AT+CPIN?");
	delay(500);
	sendATCommand("AT+CFUN?");
	delay(500);
	sendATCommand("AT+CSCLK?");

	delay(500);
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
	if (now - lastPublish > 2000)
	{
		Serial.printf("[%8lu] %s\r\n", millis(), "AT+CPSI?");
		modem.sendAT("AT+CPSI?");
		lastPublish = now;
	}

	// Trigger measurement and send packet
	#ifdef ENABLE_COAP
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
	#endif
}

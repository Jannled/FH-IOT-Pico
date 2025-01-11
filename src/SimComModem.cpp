#include "SimComModem.hpp"

SimComModem::SimComModem()
{

}

void SimComModem::init(unsigned long baudRate)
{
    pinMode(PIN_MODEM_WAKE, OUTPUT);
	pinMode(PIN_MODEM_SLEEP, OUTPUT);

    digitalWrite(PIN_MODEM_SLEEP, LOW);
	digitalWrite(PIN_MODEM_WAKE, LOW);

    SERIAL_MODEM.begin(baudRate); // SIM7080G-Cat-M module
    SERIAL_MODEM.setTimeout(5000);
}

void SimComModem::powerOnSequence()
{
    const unsigned long oldTimeout = SERIAL_MODEM.getTimeout();
    SERIAL_MODEM.setTimeout(500);

    // Power on State is only possible with DTR and PWR on LOW
    digitalWrite(PIN_MODEM_SLEEP, LOW);
	digitalWrite(PIN_MODEM_WAKE, LOW);

    // Write AT Command to check if modem is already on.
    // If we receive a response we don't have to do anything
    SERIAL_MODEM.println("AT");
    String response = SERIAL_MODEM.readStringUntil('\r');

    // Restore Timeout
    SERIAL_MODEM.setTimeout(oldTimeout);
    if(response.length() > 0)
        return;

    // Wake up Modem
	digitalWrite(PIN_MODEM_SLEEP, HIGH);
    delay(1200);
    digitalWrite(PIN_MODEM_SLEEP, LOW);
}

void SimComModem::sendAT(const char* command)
{
    // Pull down DTR to wake up modem if in Sleep
    wakeup();

    SERIAL_MODEM.println(command);

    delay(10);
    sleep();
}

void SimComModem::wakeup()
{
    digitalWrite(PIN_MODEM_WAKE, LOW);
    delay(10);
}

void SimComModem::sleep()
{
    //digitalWrite(PIN_MODEM_WAKE, HIGH);
}

int SimComModem::available()
{
    return SERIAL_MODEM.available();
}

size_t SimComModem::write(uint8_t data)
{
    return SERIAL_MODEM.write(data);
}

int SimComModem::read()
{
    return SERIAL_MODEM.read();
}

arduino::String SimComModem::readStringUntil(char terminator)
{
    return SERIAL_MODEM.readStringUntil(terminator);
}

arduino::String SimComModem::readLine()
{
    String foo = SERIAL_MODEM.readStringUntil('\n');
	foo.trim();
    return foo;
}
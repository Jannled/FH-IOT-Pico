#include "SimComModem.hpp"

SimComModem::SimComModem()
{

}

void SimComModem::init(unsigned long baudRate)
{
    pinMode(PIN_MODEM_DTR, OUTPUT);
	pinMode(PIN_MODEM_PWRKEY, OUTPUT);

    digitalWrite(PIN_MODEM_PWRKEY, LOW);
	digitalWrite(PIN_MODEM_DTR, LOW);

    SERIAL_MODEM.begin(baudRate); // SIM7080G-Cat-M module
    SERIAL_MODEM.setTimeout(5000);
}

void SimComModem::powerOnSequence()
{
    const unsigned long oldTimeout = SERIAL_MODEM.getTimeout();
    SERIAL_MODEM.setTimeout(500);

    // Power on State is only possible with DTR and PWR on LOW
    digitalWrite(PIN_MODEM_PWRKEY, LOW);
	digitalWrite(PIN_MODEM_DTR, LOW);

    // Write AT Command to check if modem is already on.
    // If we receive a response we don't have to do anything
    SERIAL_MODEM.println("AT");
    String response = SERIAL_MODEM.readStringUntil('\r');

    // Restore Timeout
    SERIAL_MODEM.setTimeout(oldTimeout);
    if(response.length() > 0)
    {
        //Serial.println("Reboot sequence");
        //reboot();
        return;
    }

    // Wake up Modem
    Serial.println("Wakeup sequence");
	digitalWrite(PIN_MODEM_PWRKEY, HIGH);
    delay(1200);
    digitalWrite(PIN_MODEM_PWRKEY, LOW);
}

void SimComModem::reboot()
{
    // Pulling PWRKEY to low for >12s will reboot the device
    digitalWrite(PIN_MODEM_PWRKEY, HIGH);
    delay(1400); 
    digitalWrite(PIN_MODEM_PWRKEY, LOW);
    digitalWrite(PIN_MODEM_DTR, LOW);
    //sendAT("AT+CREBOOT");
}

void SimComModem::sendAT(const char* command)
{
    wakeup();
    SERIAL_MODEM.println(command);
}

void SimComModem::echoAT(const char* command)
{
	SERIAL_MODEM.println(command);
    unsigned long startTime = millis();
    while((millis() - startTime) < timeout || available())
    {
        if(available())
            Serial.write((uint8_t) read());
    }
    delay(100);
}

void SimComModem::wakeup()
{
    digitalWrite(PIN_MODEM_DTR, LOW);
}

void SimComModem::sleep()
{
    //digitalWrite(PIN_MODEM_DTR, HIGH);
}

void SimComModem::initCoAP()
{
    echoAT("AT+CNACT=0,1");
    delay(500);
    echoAT("AT+CNACT?");
    delay(100);
    echoAT("AT+CCOAPINIT");
}

int SimComModem::sendPacket(const char* url, const char* path, const char* payload)
{
    SERIAL_MODEM.printf("AT+CCOAPURL=\"coap://%s\"\r\n", url);
    delay(500);
    flush();
    echoAT("AT+CCOAPPARA=\"CODE\",1,uri-path,0,\"sensor/data\",uri-query,0,\"address=1\",payload,0,\"hello world\"");
    //SERIAL_MODEM.printf(
    //    "AT+CCOAPPARA=\"CODE\",1,uri-path,0,\"%s\",uri-query,0,\"address=1\",payload,0,\"%s\"\r\n", 
    //    path, payload
    //);
    delay(100);
    sendAT("AT+CCOAPACTION");

    return 1;
}

void SimComModem::ping(const char* url)
{
    echoAT("AT+SNPDPID?");
    SERIAL_MODEM.printf("AT+SNPING4=\"%s\",1,8,10000\r\n", url);
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

unsigned int SimComModem::flush()
{
    unsigned int bytes = 0;
    unsigned long startTime = millis();
    while((millis() - startTime) < timeout || available())
    {
        if(available())
            Serial.write((uint8_t) read());
    }

    return bytes;
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

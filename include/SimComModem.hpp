#ifndef SIMCOM_7080G_H
#define SIMCOM_7080G_H

#include <stdint.h>
#include <Arduino.h>

#define SERIAL_MODEM Serial1

// Pull down to wake Modem during sleep
// Connected to DTR Pin on Modem
#define PIN_MODEM_DTR 17

// Pull down to cut modem power
#define PIN_MODEM_PWRKEY 14

#define CRLF "\r\n"

class SimComModem
{
    public:
    SimComModem();

    void init(unsigned long baudRate);
    void sendAT(const char* string);
    void echoAT(const char* command);

    void powerOnSequence();
    void wakeup();
    void sleep();
    void reboot();

    void initCoAP();
    int sendPacket(const char* url, const char* path, const char* payload);
    void ping(const char* url);

    int available();
    size_t write(uint8_t data);
    int read();
    arduino::String readStringUntil(char terminator);
    arduino::String readLine();
    unsigned int flush();

    unsigned long timeout = 1500;

    private:
};

#endif // SIMCOM_7080G_H

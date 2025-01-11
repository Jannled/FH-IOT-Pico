#ifndef SIMCOM_7080G_H
#define SIMCOM_7080G_H

#include <stdint.h>
#include <Arduino.h>

#define SERIAL_MODEM Serial1

// Pull down to wake Modem during sleep
// Connected to DTR Pin on Modem
#define PIN_MODEM_WAKE 17

// Pull down to cut modem power
#define PIN_MODEM_SLEEP 14

#define CRLF "\r\n"

class SimComModem
{
    public:
    SimComModem();

    void init(unsigned long baudRate);
    void sendAT(const char* string);

    void powerOnSequence();
    void wakeup();
    void sleep();

    int available();
    size_t write(uint8_t data);
    int read();
    arduino::String readStringUntil(char terminator);
    arduino::String readLine();

    private:
};

#endif // SIMCOM_7080G_H

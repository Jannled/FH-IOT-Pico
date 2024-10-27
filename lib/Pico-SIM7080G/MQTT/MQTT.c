#include "MQTT.h"

// MQTT Server info
char mqtt_host[] = "47.89.22.46,";           
char mqtt_port[] = "1883";
char mqtt_msg[] = "on";

void mqttTest()
{
    char SMCONF[100] = "AT+SMCONF=\"URL\",";
    strcat(SMCONF,mqtt_host);
    strcat(SMCONF,mqtt_port);
    sendCMD_waitResp(SMCONF, "OK", 2000);
    sendCMD_waitResp("AT+SMCONF=\"KEEPTIME\",600", "OK", 2000);
    sendCMD_waitResp("AT+SMCONF=\"CLIENTID\",\"Pico_SIM7080G\"", "OK", 2000);
    sendCMD_waitResp("AT+SMCONN", "OK", 2000);
    sendCMD_waitResp("AT+SMSUB=\"mqtt\",1", "OK", 2000);
    sendCMD_waitResp("AT+SMPUB=\"mqtt\",2,1,0", "OK", 2000);
    uart_puts(UART_ID0, mqtt_msg);
    DEV_Delay_ms(2000);
    sendCMD_waitResp("AT+SMUNSUB=\"mqtt\"", "OK", 2000);
    printf("send message successfully!\r\n");
    sendCMD_waitResp("AT+SMDISC", "OK", 2000);
}
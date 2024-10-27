#include "Pico_SIM7080G_NB_loT.h"


int Pico_SIM7080G_NB_loT_AT()
{
    DEV_Module_Init();
    led_blink();
    DEV_Delay_ms(5000);
    check_start();
    set_network();
    check_network();
    return true;
}
int Pico_SIM7080G_NB_loT_HTTP()
{
    DEV_Module_Init();
    led_blink();
    DEV_Delay_ms(5000);
    check_start();
    set_network();
    check_network();
    while (1)
    {
        http_get();
        http_post();
    }

    return true;
}

int Pico_SIM7080G_NB_loT_MQTT()
{
    DEV_Module_Init();
    led_blink();
    DEV_Delay_ms(5000);
    check_start();
    set_network();
    check_network();
    while (1)
    {
        mqttTest();
    }
    
   
    return true;
}

int Pico_SIM7080G_NB_loT_GPS()
{
    DEV_Module_Init();
    led_blink();
    DEV_Delay_ms(5000);
    check_start();
    while (1)
    {
        GPSTest();
    }
       
   
    return true;
}
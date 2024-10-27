#include "GPS.h"

void GPSTest()
{
    int count = 0;
    printf("Start GPS session...\r\n");
    sendCMD_waitResp("AT+CGNSPWR=1", "OK", 2000);
    DEV_Delay_ms(2000);
    for (int i = 1; i < 10; i++)
    {
        if (sendCMD_waitResp("AT+CGNSINF", ",,,,", 2000) == 1)
        {
            printf("GPS is not ready\r\n");
            if (i >= 9)
            {
                printf("GPS positioning failed, please check the GPS antenna!\r\n");
                sendCMD_waitResp("AT+CGNSPWR=0", "OK", 2000);
            }
            else
            {
                printf("wait...\r\n");
                DEV_Delay_ms(2000);
                continue;
            }
        }
        else
        {
            if (count <= 3)
            {
                count++;
                printf("GPS info:\r\n");
            }
            else
            {
                sendCMD_waitResp("AT+CGNSPWR=0", "OK", 2000);
                break;
            }
        }
    }
}

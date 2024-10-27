#include "HTTP.h"

void set_http_length()
{
    sendCMD_waitResp("AT+SHCONF=\"BODYLEN\",1024", "OK", 2000);
    sendCMD_waitResp("AT+SHCONF=\"HEADERLEN\",350", "OK", 2000);
}

void set_http_content()
{
    sendCMD_waitResp("AT+SHCHEAD", "OK", 2000);
    sendCMD_waitResp("AT+SHAHEAD=\"Content-Type\",\"application/x-www-form-urlencoded\"", "OK", 2000);
    sendCMD_waitResp("AT+SHAHEAD=\"User-Agent\",\"curl/7.47.0\"", "OK", 2000);
    sendCMD_waitResp("AT+SHAHEAD=\"Cache-control\",\"no-cache\"", "OK", 2000);
    sendCMD_waitResp("AT+SHAHEAD=\"Connection\",\"keep-alive\"", "OK", 2000);
    sendCMD_waitResp("AT+SHAHEAD=\"Accept\",\"*/*\"", "OK", 2000);
    sendCMD_waitResp("AT+SHCHEAD", "OK", 2000);
}
void http_get()
{
    char http_get_server0[] = "http://api.seniverse.com";
    char http_get_server1[] = "/v3/weather/now.json?key=SwwwfskBjB6fHVRon&location=shenzhen&language=en&unit=c";
    char SHCONF[200] = "AT+SHCONF=\"URL\",\"";
    char SHREQ[200] = "AT+SHREQ=\"";
    char SHREAD[100] = "AT+SHREAD=0,";
    char a[] = "\"", b[] = "\",1";
    char *resp, *p, *get_pack_len;
    sendCMD_waitResp("AT+SHDISC", "OK", 2000);
    strcat(SHCONF, http_get_server0);
    strcat(SHCONF, a);
    sendCMD_waitResp(SHCONF, "OK", 2000);
    set_http_length();
    sendCMD_waitResp("AT+SHCONN", "OK", 3000);
    if (sendCMD_waitResp("AT+SHSTATE?", "1", 2000))
    {
        set_http_content();
        strcat(SHREQ, http_get_server1);
        strcat(SHREQ, b);
        resp = waitResp(SHREQ, "OK", 8000);
        p = strrchr(resp, ',');
        get_pack_len = p + 1;
        if (*get_pack_len > 0)
        {
            strcat(SHREAD, get_pack_len);
            waitResp(SHREAD, "OK", 5000);
            sendCMD_waitResp("AT+SHDISC", "OK", 2000);
        }
        else
        {
            printf("HTTP Get failed!\n");
        }
    }
    else
    {
        printf("HTTP connection disconnected, please check and try again\n");
    }
}

void http_post()
{
    char http_post_server0[] = "http://pico.wiki";
    char http_post_server1[] = "post-data.php";
    char http_post_tmp[] = "api_key=tPmAT5Ab3j888&value1=26.44&value2=57.16&value3=1002.95";
    char SHCONF[200] = "AT+SHCONF=\"URL\",\"";
    char SHREQ[200] = "AT+SHREQ=\"/";
    char SHREAD[100] = "AT+SHREAD=0,";
    char a[] = "\"", b[] = "\",3";
    char *resp, *p, *get_pack;
    sendCMD_waitResp("AT+SHDISC", "OK", 2000);
    strcat(SHCONF, http_post_server0);
    strcat(SHCONF, a);
    sendCMD_waitResp(SHCONF, "OK", 2000);
    set_http_length();
    sendCMD_waitResp("AT+SHCONN", "OK", 3000);
    if (sendCMD_waitResp("AT+SHSTATE?", "1", 2000))
    {
        set_http_content();
        sendCMD_waitResp("AT+SHCPARA", "OK", 3000);
        if (sendCMD_waitResp("AT+SHBOD=62,10000", ">", 1000))
        {
            sendCMD_waitResp(http_post_tmp, "OK", 2000);
            strcat(SHREQ, http_post_server1);
            strcat(SHREQ, b);
            resp = waitResp(SHREQ, "OK", 8000);
            p = strrchr(resp, ',');
            get_pack = p + 1;
            if (*get_pack > 0)
            {
                strcat(SHREAD, get_pack);
                waitResp(SHREAD, "OK", 3000);
                sendCMD_waitResp("AT+SHDISC", "OK", 2000);
            }
            else
            {
                printf("HTTP Get failed!\n");
            }
        }
        else
            printf("Send failed\n");
    }
    else
        printf("HTTP connection disconnected, please check and try again\n");
}
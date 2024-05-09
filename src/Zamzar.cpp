
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "myWiFi.h"
#include <utils.h>
#include "Zamzar_htps_certificate.h"  // rootCACertificate_Zamzar

// https://www.os-domzale.si/ --> https://www.os-domzale.si/files/2024/04/jedilnik-2024-5-1.pdf

/*
 https://developers.zamzar.com/docs#section-Start_a_conversion_job
 https://developers.zamzar.com/docs#section-Download_the_converted_file

1. start the job; get job ID
    curl https://sandbox.zamzar.com/v1/jobs -u eed6d139dc0b59597bce6d5c598b735da678d496: -X POST --data-urlencode "source_file=https://www.os-domzale.si/files/2024/04/jedilnik-2024-5-1.pdf" -d "target_format=txt"
2. poll the job ID status; get file ID
    curl https://sandbox.zamzar.com/v1/jobs/43533821 -u eed6d139dc0b59597bce6d5c598b735da678d496:
3. download the file ID
    curl https://sandbox.zamzar.com/v1/files/169760676/content -u eed6d139dc0b59597bce6d5c598b735da678d496: -L -i  
*/


    const String apiKey = "eed6d139dc0b59597bce6d5c598b735da678d496";
    const String endpointTest = "sandbox.zamzar.com/v1/jobs";
    const String endpoint = "api.zamzar.com/v1/jobs";
    const String sourceFile = "https://www.os-domzale.si/files/2024/04/jedilnik-2024-5-1.pdf";
    const String targetFormat = "txt";



/*
https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/esp_http_client.html#http-authentication

Examples of Authentication Configuration
Authentication with URI

esp_http_client_config_t config = {
    .url = "http://user:passwd@httpbin.org/basic-auth/user/passwd",
    .auth_type = HTTP_AUTH_TYPE_BASIC,
};
Authentication with username and password entry

esp_http_client_config_t config = {
    .url = "http://httpbin.org/basic-auth/user/passwd",
    .username = "user",
    .password = "passwd",
    .auth_type = HTTP_AUTH_TYPE_BASIC,
};
*/


